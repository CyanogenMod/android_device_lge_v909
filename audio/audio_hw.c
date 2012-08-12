/*
 * Copyright (C) 2012 Wolfson Microelectronics plc
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Liberal inspiration drawn from the AOSP code for Toro.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "audio_hw_primary"
#define LOG_NDEBUG 0

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <cutils/str_parms.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include <expat.h>

#include <tinyalsa/asoundlib.h>
#include <audio_utils/resampler.h>
#include <hardware/audio_effect.h>


#define PCM_CARD 0
#define PCM_DEVICE 0
#define PCM_DEVICE_AUX 1
#define PCM_DEVICE_SCO 2

#define OUT_PERIOD_SIZE 1024
#define OUT_SHORT_PERIOD_COUNT 2
#define OUT_LONG_PERIOD_COUNT 8
#define OUT_SAMPLING_RATE 44100

#define IN_PERIOD_SIZE 1024
#define IN_PERIOD_COUNT 4
#define IN_SAMPLING_RATE 44100

#define SCO_PERIOD_SIZE 256
#define SCO_PERIOD_COUNT 4
#define SCO_SAMPLING_RATE 8000

/* minimum sleep time in out_write() when write threshold is not reached */
#define MIN_WRITE_SLEEP_US 2000
#define MAX_WRITE_SLEEP_US ((OUT_PERIOD_SIZE * OUT_SHORT_PERIOD_COUNT * 1000000) \
                                / OUT_SAMPLING_RATE)

enum {
    OUT_BUFFER_TYPE_UNKNOWN,
    OUT_BUFFER_TYPE_SHORT,
    OUT_BUFFER_TYPE_LONG,
};

struct route_setting
{
    char *ctl_name;
    int intval;
    char *strval;
};

struct config_parse_state {
    struct tiny_audio_device *adev;
    struct tiny_dev_cfg *dev;
    bool on;

    struct route_setting *path;
    unsigned int path_len;
};

static const struct {
    int mask;
    const char *name;
} dev_names[] = {
    { AUDIO_DEVICE_OUT_SPEAKER, "speaker" },
    { AUDIO_DEVICE_OUT_WIRED_HEADSET | AUDIO_DEVICE_OUT_WIRED_HEADPHONE,
      "headphone" },
    { AUDIO_DEVICE_OUT_AUX_DIGITAL, "aux-digital" },
    { AUDIO_DEVICE_OUT_ALL_SCO, "sco-out" },

    { AUDIO_DEVICE_IN_BUILTIN_MIC, "builtin-mic" },
    { AUDIO_DEVICE_IN_WIRED_HEADSET, "headset-in" },
    { AUDIO_DEVICE_IN_ALL_SCO, "sco-in" },
};

struct pcm_config pcm_config_out = {
    .channels = 2,
    .rate = OUT_SAMPLING_RATE,
    .period_size = OUT_PERIOD_SIZE,
    .period_count = OUT_LONG_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = OUT_PERIOD_SIZE * OUT_SHORT_PERIOD_COUNT,
};

struct pcm_config pcm_config_in = {
    .channels = 2,
    .rate = IN_SAMPLING_RATE,
    .period_size = IN_PERIOD_SIZE,
    .period_count = IN_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = 1,
    .stop_threshold = (IN_PERIOD_SIZE * IN_PERIOD_COUNT),
};

struct pcm_config pcm_config_sco = {
    .channels = 1,
    .rate = SCO_SAMPLING_RATE,
    .period_size = SCO_PERIOD_SIZE,
    .period_count = SCO_PERIOD_COUNT,
    .format = PCM_FORMAT_S16_LE,
};

struct tiny_dev_cfg {
    int mask;

    struct route_setting *on;
    unsigned int on_len;

    struct route_setting *off;
    unsigned int off_len;
};

struct tiny_audio_device {
    struct audio_hw_device hw_device;
    pthread_mutex_t lock; /* see note below on mutex acquisition order */
    struct mixer *mixer;
    unsigned int active_devices;
    unsigned int devices;
    bool standby;
    bool mic_mute;
    struct tiny_dev_cfg *dev_cfgs;
    int num_dev_cfgs;
    int orientation;
    bool screen_off;
    struct tiny_stream_out *active_out;
    struct tiny_stream_in *active_in;
};

struct tiny_stream_out {
    struct audio_stream_out stream;

    pthread_mutex_t lock; /* see note below on mutex acquisition order */
    struct pcm *pcm;
    struct pcm_config *pcm_config;
    bool standby;

    struct resampler_itfe *resampler;
    int16_t *buffer;
    size_t buffer_frames;

    int write_threshold;
    int cur_write_threshold;
    int buffer_type;

    struct tiny_audio_device *adev;
};

struct tiny_stream_in {
    struct audio_stream_in stream;

    pthread_mutex_t lock; /* see note below on mutex acquisition order */
    struct pcm *pcm;
    struct pcm_config *pcm_config;
    bool standby;

    unsigned int requested_rate;
    struct resampler_itfe *resampler;
    struct resampler_buffer_provider buf_provider;
    int16_t *buffer;
    size_t buffer_size;
    size_t frames_in;
    int read_status;

    struct tiny_audio_device *adev;
};

static uint32_t out_get_sample_rate(const struct audio_stream *stream);
static size_t out_get_buffer_size(const struct audio_stream *stream);
static audio_format_t out_get_format(const struct audio_stream *stream);
static uint32_t in_get_sample_rate(const struct audio_stream *stream);
static size_t in_get_buffer_size(const struct audio_stream *stream);
static audio_format_t in_get_format(const struct audio_stream *stream);
static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                                   struct resampler_buffer* buffer);
static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                                  struct resampler_buffer* buffer);


/* The enable flag when 0 makes the assumption that enums are disabled by
 * "Off" and integers/booleans by 0 */
static int set_route_by_array(struct mixer *mixer, struct route_setting *route,
                  unsigned int len)
{
    struct mixer_ctl *ctl;
    unsigned int i, j, ret;

    /* Go through the route array and set each value */
    for (i = 0; i < len; i++) {
        ctl = mixer_get_ctl_by_name(mixer, route[i].ctl_name);
        if (!ctl) {
            ALOGE("Unknown control '%s'\n", route[i].ctl_name);
                return -EINVAL;
        }

        if (route[i].strval) {
            ret = mixer_ctl_set_enum_by_string(ctl, route[i].strval);
            if (ret != 0) {
                ALOGE("Failed to set '%s' to '%s'\n",
                     route[i].ctl_name, route[i].strval);
            } else {
                ALOGV("Set '%s' to '%s'\n",
                     route[i].ctl_name, route[i].strval);
            }
        
        } else {
            /* This ensures multiple (i.e. stereo) values are set jointly */
            for (j = 0; j < mixer_ctl_get_num_values(ctl); j++) {
                ret = mixer_ctl_set_value(ctl, j, route[i].intval);
                if (ret != 0) {
                    ALOGE("Failed to set '%s'.%d to %d\n",
                     route[i].ctl_name, j, route[i].intval);
                } else {
                    ALOGV("Set '%s'.%d to %d\n",
                     route[i].ctl_name, j, route[i].intval);
                }
            }
        }
    }

    return 0;
}

static void adev_config_start(void *data, const XML_Char *elem,
                  const XML_Char **attr)
{
    struct config_parse_state *s = data;
    struct tiny_dev_cfg *dev_cfg;
    const XML_Char *name = NULL;
    const XML_Char *val = NULL;
    const XML_Char *alsa_card = NULL;
    const XML_Char *alsa_dev = NULL;
    unsigned int i, j;

    for (i = 0; attr[i]; i += 2) {
        if (strcmp(attr[i], "name") == 0)
            name = attr[i + 1];

        if (strcmp(attr[i], "val") == 0)
            val = attr[i + 1];
    }

    if (strcmp(elem, "device") == 0) {
        if (!name) {
            ALOGE("Unnamed device\n");
            return;
        }

        for (i = 0; i < sizeof(dev_names) / sizeof(dev_names[0]); i++) {
            if (strcmp(dev_names[i].name, name) == 0) {
                ALOGI("Allocating device %s\n", name);
                dev_cfg = realloc(s->adev->dev_cfgs,
                          (s->adev->num_dev_cfgs + 1)
                          * sizeof(*dev_cfg));
                if (!dev_cfg) {
                    ALOGE("Unable to allocate dev_cfg\n");
                    return;
                }

                s->dev = &dev_cfg[s->adev->num_dev_cfgs];
                memset(s->dev, 0, sizeof(*s->dev));
                s->dev->mask = dev_names[i].mask;
                s->adev->dev_cfgs = dev_cfg;
                s->adev->num_dev_cfgs++;
            }
        }

    } else if (strcmp(elem, "path") == 0) {
        if (s->path_len)
            ALOGW("Nested paths\n");

        /* If this a path for a device it must have a role */
        if (s->dev) {
            /* Need to refactor a bit... */
            if (strcmp(name, "on") == 0) {
                s->on = true;
            } else if (strcmp(name, "off") == 0) {
                s->on = false;
            } else {
                ALOGW("Unknown path name %s\n", name);
            }
        }

    } else if (strcmp(elem, "ctl") == 0) {
    struct route_setting *r;

    if (!name) {
        ALOGE("Unnamed control\n");
        return;
    }

    if (!val) {
        ALOGE("No value specified for %s\n", name);
        return;
    }

    ALOGV("Parsing control %s => %s\n", name, val);

    r = realloc(s->path, sizeof(*r) * (s->path_len + 1));
    if (!r) {
        ALOGE("Out of memory handling %s => %s\n", name, val);
        return;
    }

    r[s->path_len].ctl_name = strdup(name);
    r[s->path_len].strval = NULL;

    /* This can be fooled but it'll do */
    r[s->path_len].intval = atoi(val);
    if (!r[s->path_len].intval && strcmp(val, "0") != 0)
        r[s->path_len].strval = strdup(val);

    s->path = r;
    s->path_len++;
    }
}


static void adev_config_end(void *data, const XML_Char *name)
{
    struct config_parse_state *s = data;
    unsigned int i;

    if (strcmp(name, "path") == 0) {
        if (!s->path_len)
            ALOGW("Empty path\n");

        if (!s->dev) {
            ALOGV("Applying %d element default route\n", s->path_len);

            set_route_by_array(s->adev->mixer, s->path, s->path_len);

            for (i = 0; i < s->path_len; i++) {
                free(s->path[i].ctl_name);
                free(s->path[i].strval);
            }

            free(s->path);

            /* Refactor! */
        } else if (s->on) {
            ALOGV("%d element on sequence\n", s->path_len);
            s->dev->on = s->path;
            s->dev->on_len = s->path_len;

        } else {
            ALOGV("%d element off sequence\n", s->path_len);

            /* Apply it, we'll reenable anything that's wanted later */
            set_route_by_array(s->adev->mixer, s->path, s->path_len);

            s->dev->off = s->path;
            s->dev->off_len = s->path_len;
        }

        s->path_len = 0;
        s->path = NULL;

    } else if (strcmp(name, "device") == 0) {
        s->dev = NULL;
    }
}

static int adev_config_parse(struct tiny_audio_device *adev)
{
    struct config_parse_state s;
    FILE *f;
    XML_Parser p;
    char file[80];
    int ret = 0;
    bool eof = false;
    int len;

    snprintf(file, sizeof(file), "/system/etc/sound/tiny_hw.xml");

    ALOGV("Reading configuration from %s\n", file);
    f = fopen(file, "r");
    if (!f) {
        ALOGE("Failed to open %s\n", file);
        return -ENODEV;
    }

    p = XML_ParserCreate(NULL);
    if (!p) {
        ALOGE("Failed to create XML parser\n");
        ret = -ENOMEM;
        goto out;
    }

    memset(&s, 0, sizeof(s));
    s.adev = adev;
    XML_SetUserData(p, &s);

    XML_SetElementHandler(p, adev_config_start, adev_config_end);

    while (!eof) {
        len = fread(file, 1, sizeof(file), f);
        if (ferror(f)) {
            ALOGE("I/O error reading config\n");
            ret = -EIO;
            goto out_parser;
        }
        eof = feof(f);

        if (XML_Parse(p, file, len, eof) == XML_STATUS_ERROR) {
            ALOGE("Parse error at line %u:\n%s\n",
             (unsigned int)XML_GetCurrentLineNumber(p),
             XML_ErrorString(XML_GetErrorCode(p)));
            ret = -EINVAL;
            goto out_parser;
        }
    }

 out_parser:
    XML_ParserFree(p);
 out:
    fclose(f);

    return ret;
}

/* Must be called with route_lock */
void select_devices(struct tiny_audio_device *adev)
{
    int i;
    if (adev->active_devices == adev->devices)
	    return;
    ALOGV("Changing devices to %x\n", adev->devices);

    /* ...then disable old ones. */
    for (i = 0; i < adev->num_dev_cfgs; i++)
    if (!(adev->devices & adev->dev_cfgs[i].mask) &&
        (adev->active_devices & adev->dev_cfgs[i].mask))
        set_route_by_array(adev->mixer, adev->dev_cfgs[i].off,
                   adev->dev_cfgs[i].off_len);

    /* Turn on new devices first so we don't glitch due to powerdown... */
    for (i = 0; i < adev->num_dev_cfgs; i++)
    if ((adev->devices & adev->dev_cfgs[i].mask) &&
        !(adev->active_devices & adev->dev_cfgs[i].mask))
        set_route_by_array(adev->mixer, adev->dev_cfgs[i].on,
                   adev->dev_cfgs[i].on_len);
				   
    adev->active_devices = adev->devices;
}

/* must be called with hw device and output stream mutexes locked */
static void do_out_standby(struct tiny_stream_out *out)
{
    struct tiny_audio_device *adev = out->adev;

    if (!out->standby) {
        pcm_close(out->pcm);
        out->pcm = NULL;
        adev->active_out = NULL;
        if (out->resampler) {
            release_resampler(out->resampler);
            out->resampler = NULL;
        }
        if (out->buffer) {
            free(out->buffer);
            out->buffer = NULL;
        }
        out->standby = true;
    }
}

/* must be called with hw device and input stream mutexes locked */
static void do_in_standby(struct tiny_stream_in *in)
{
    struct tiny_audio_device *adev = in->adev;

    if (!in->standby) {
        pcm_close(in->pcm);
        in->pcm = NULL;
        adev->active_in = NULL;
        if (in->resampler) {
            release_resampler(in->resampler);
            in->resampler = NULL;
        }
        if (in->buffer) {
            free(in->buffer);
            in->buffer = NULL;
        }
        in->standby = true;
    }
}

/* must be called with hw device and output stream mutexes locked */
static int start_output_stream(struct tiny_stream_out *out)
{
    struct tiny_audio_device *adev = out->adev;
    unsigned int device;
    int ret;

    /*
     * Due to the lack of sample rate converters in the SoC,
     * it greatly simplifies things to have only the main
     * (speaker/headphone) PCM or the BC SCO PCM open at
     * the same time.
     */
    if (adev->devices & AUDIO_DEVICE_OUT_ALL_SCO) {
        device = PCM_DEVICE_SCO;
        out->pcm_config = &pcm_config_sco;
    } else if (adev->devices & AUDIO_DEVICE_OUT_AUX_DIGITAL) {
        device = PCM_DEVICE_AUX;
        out->pcm_config = &pcm_config_out;
        out->buffer_type = OUT_BUFFER_TYPE_UNKNOWN;
    } else {
        device = PCM_DEVICE;
        out->pcm_config = &pcm_config_out;
        out->buffer_type = OUT_BUFFER_TYPE_UNKNOWN;
    }

    /*
     * All open PCMs can only use a single group of rates at once:
     * Group 1: 11.025, 22.05, 44.1
     * Group 2: 8, 16, 32, 48
     * Group 1 is used for digital audio playback since 44.1 is
     * the most common rate, but group 2 is required for SCO.
     */
    if (adev->active_in) {
        pthread_mutex_lock(&adev->active_in->lock);
        if (((out->pcm_config->rate % 8000 == 0) &&
                 (adev->active_in->pcm_config->rate % 8000) != 0) ||
                 ((out->pcm_config->rate % 11025 == 0) &&
                 (adev->active_in->pcm_config->rate % 11025) != 0))
            do_in_standby(adev->active_in);
        pthread_mutex_unlock(&adev->active_in->lock);
    }

    out->pcm = pcm_open(PCM_CARD, device, PCM_OUT | PCM_NORESTART, out->pcm_config);

    if (out->pcm && !pcm_is_ready(out->pcm)) {
        ALOGE("pcm_open(out) failed: %s", pcm_get_error(out->pcm));
        pcm_close(out->pcm);
        return -ENOMEM;
    }

    /*
     * If the stream rate differs from the PCM rate, we need to
     * create a resampler.
     */
    if (out_get_sample_rate(&out->stream.common) != out->pcm_config->rate) {
        ret = create_resampler(out_get_sample_rate(&out->stream.common),
                               out->pcm_config->rate,
                               out->pcm_config->channels,
                               RESAMPLER_QUALITY_DEFAULT,
                               NULL,
                               &out->resampler);
        out->buffer_frames = (pcm_config_out.period_size * out->pcm_config->rate) /
                out_get_sample_rate(&out->stream.common) + 1;

        out->buffer = malloc(pcm_frames_to_bytes(out->pcm, out->buffer_frames));
    }

    adev->active_out = out;

    return 0;
}

/* must be called with hw device and input stream mutexes locked */
static int start_input_stream(struct tiny_stream_in *in)
{
    struct tiny_audio_device *adev = in->adev;
    unsigned int device;
    int ret;

    /*
     * Due to the lack of sample rate converters in the SoC,
     * it greatly simplifies things to have only the main
     * mic PCM or the BC SCO PCM open at the same time.
     */
    if (adev->devices & AUDIO_DEVICE_IN_ALL_SCO) {
        device = PCM_DEVICE_SCO;
        in->pcm_config = &pcm_config_sco;
    } else {
        device = PCM_DEVICE;
        in->pcm_config = &pcm_config_in;
    }

    /*
     * All open PCMs can only use a single group of rates at once:
     * Group 1: 11.025, 22.05, 44.1
     * Group 2: 8, 16, 32, 48
     * Group 1 is used for digital audio playback since 44.1 is
     * the most common rate, but group 2 is required for SCO.
     */
    if (adev->active_out) {
        pthread_mutex_lock(&adev->active_out->lock);
        if (((in->pcm_config->rate % 8000 == 0) &&
                 (adev->active_out->pcm_config->rate % 8000) != 0) ||
                 ((in->pcm_config->rate % 11025 == 0) &&
                 (adev->active_out->pcm_config->rate % 11025) != 0))
            do_out_standby(adev->active_out);
        pthread_mutex_unlock(&adev->active_out->lock);
    }

    in->pcm = pcm_open(PCM_CARD, device, PCM_IN, in->pcm_config);

    if (in->pcm && !pcm_is_ready(in->pcm)) {
        ALOGE("pcm_open(in) failed: %s", pcm_get_error(in->pcm));
        pcm_close(in->pcm);
        return -ENOMEM;
    }

    /*
     * If the stream rate differs from the PCM rate, we need to
     * create a resampler.
     */
    if (in_get_sample_rate(&in->stream.common) != in->pcm_config->rate) {
        in->buf_provider.get_next_buffer = get_next_buffer;
        in->buf_provider.release_buffer = release_buffer;

        ret = create_resampler(in->pcm_config->rate,
                               in_get_sample_rate(&in->stream.common),
                               1,
                               RESAMPLER_QUALITY_DEFAULT,
                               &in->buf_provider,
                               &in->resampler);
    }
    in->buffer_size = pcm_frames_to_bytes(in->pcm,
                                          in->pcm_config->period_size);
    in->buffer = malloc(in->buffer_size);

    adev->active_in = in;

    return 0;
}

static int get_next_buffer(struct resampler_buffer_provider *buffer_provider,
                                   struct resampler_buffer* buffer)
{
    struct tiny_stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return -EINVAL;

    in = (struct tiny_stream_in *)((char *)buffer_provider -
                                   offsetof(struct tiny_stream_in, buf_provider));

    if (in->pcm == NULL) {
        buffer->raw = NULL;
        buffer->frame_count = 0;
        in->read_status = -ENODEV;
        return -ENODEV;
    }

    if (in->frames_in == 0) {
        in->read_status = pcm_read(in->pcm,
                                   (void*)in->buffer,
                                   in->buffer_size);
        if (in->read_status != 0) {
            ALOGE("get_next_buffer() pcm_read error %d", in->read_status);
            buffer->raw = NULL;
            buffer->frame_count = 0;
            return in->read_status;
        }
        in->frames_in = in->pcm_config->period_size;
        if (in->pcm_config->channels == 2) {
            unsigned int i;

            /* Discard right channel */
            for (i = 1; i < in->frames_in; i++)
                in->buffer[i] = in->buffer[i * 2];
        }
    }

    buffer->frame_count = (buffer->frame_count > in->frames_in) ?
                                in->frames_in : buffer->frame_count;
    buffer->i16 = in->buffer + (in->pcm_config->period_size - in->frames_in);

    return in->read_status;

}

static void release_buffer(struct resampler_buffer_provider *buffer_provider,
                                  struct resampler_buffer* buffer)
{
    struct tiny_stream_in *in;

    if (buffer_provider == NULL || buffer == NULL)
        return;

    in = (struct tiny_stream_in *)((char *)buffer_provider -
                                   offsetof(struct tiny_stream_in, buf_provider));

    in->frames_in -= buffer->frame_count;
}

/* read_frames() reads frames from kernel driver, down samples to capture rate
 * if necessary and output the number of frames requested to the buffer specified */
static ssize_t read_frames(struct tiny_stream_in *in, void *buffer, ssize_t frames)
{
    ssize_t frames_wr = 0;

    while (frames_wr < frames) {
        size_t frames_rd = frames - frames_wr;
        if (in->resampler != NULL) {
            in->resampler->resample_from_provider(in->resampler,
                    (int16_t *)((char *)buffer +
                            frames_wr * audio_stream_frame_size(&in->stream.common)),
                    &frames_rd);
        } else {
            struct resampler_buffer buf = {
                    { raw : NULL, },
                    frame_count : frames_rd,
            };
            get_next_buffer(&in->buf_provider, &buf);
            if (buf.raw != NULL) {
                memcpy((char *)buffer +
                           frames_wr * audio_stream_frame_size(&in->stream.common),
                        buf.raw,
                        buf.frame_count * audio_stream_frame_size(&in->stream.common));
                frames_rd = buf.frame_count;
            }
            release_buffer(&in->buf_provider, &buf);
        }
        /* in->read_status is updated by getNextBuffer() also called by
         * in->resampler->resample_from_provider() */
        if (in->read_status != 0)
            return in->read_status;

        frames_wr += frames_rd;
    }
    return frames_wr;
}

static uint32_t out_get_sample_rate(const struct audio_stream *stream)
{
    return pcm_config_out.rate;
}

static int out_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    return -ENOSYS;
}

static size_t out_get_buffer_size(const struct audio_stream *stream)
{
    return pcm_config_out.period_size *
               audio_stream_frame_size((struct audio_stream *)stream);
}

static uint32_t out_get_channels(const struct audio_stream *stream)
{
    return AUDIO_CHANNEL_OUT_STEREO;
}

static audio_format_t out_get_format(const struct audio_stream *stream)
{
    return AUDIO_FORMAT_PCM_16_BIT;
}

static int out_set_format(struct audio_stream *stream, audio_format_t format)
{
    return -ENOSYS;
}

static int out_standby(struct audio_stream *stream)
{
    struct tiny_stream_out *out = (struct tiny_stream_out *)stream;

    pthread_mutex_lock(&out->adev->lock);
    pthread_mutex_lock(&out->lock);
    do_out_standby(out);
    pthread_mutex_unlock(&out->lock);
    pthread_mutex_unlock(&out->adev->lock);

    return 0;
}

static int out_dump(const struct audio_stream *stream, int fd)
{
    return 0;
}

static int out_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct tiny_stream_out *out = (struct tiny_stream_out *)stream;
    struct tiny_audio_device *adev = out->adev;
    struct str_parms *parms;
    char value[32];
    int ret;
    unsigned int val;

    parms = str_parms_create_str(kvpairs);

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING,
                            value, sizeof(value));
    pthread_mutex_lock(&adev->lock);
    if (ret >= 0) {
        val = atoi(value);
        if (((adev->devices & AUDIO_DEVICE_OUT_ALL) != val) && (val != 0)) {
            /*
             * If SCO is turned on/off, we need to put audio into standby
             * because SCO uses a different PCM.
             */
            if ((val & AUDIO_DEVICE_OUT_ALL_SCO) ^
                    (adev->devices & AUDIO_DEVICE_OUT_ALL_SCO)) {
                pthread_mutex_lock(&out->lock);
                do_out_standby(out);
                pthread_mutex_unlock(&out->lock);
            }

            adev->devices &= ~AUDIO_DEVICE_OUT_ALL;
            adev->devices |= val;
            select_devices(adev);
        }
    }
    pthread_mutex_unlock(&adev->lock);

    str_parms_destroy(parms);
    return ret;
}

static char * out_get_parameters(const struct audio_stream *stream, const char *keys)
{
    return strdup("");
}

static uint32_t out_get_latency(const struct audio_stream_out *stream)
{
    struct tiny_stream_out *out = (struct tiny_stream_out *)stream;
    struct tiny_audio_device *adev = out->adev;
    size_t period_count;

    pthread_mutex_lock(&adev->lock);

    if (adev->screen_off && !adev->active_in && !(adev->devices & AUDIO_DEVICE_OUT_ALL_SCO))
        period_count = OUT_LONG_PERIOD_COUNT;
    else
        period_count = OUT_SHORT_PERIOD_COUNT;

    pthread_mutex_unlock(&adev->lock);

    return (pcm_config_out.period_size * period_count * 1000) / pcm_config_out.rate;
}

static int out_set_volume(struct audio_stream_out *stream, float left,
                          float right)
{
    return -ENOSYS;
}

static ssize_t out_write(struct audio_stream_out *stream, const void* buffer,
                         size_t bytes)
{
    int ret = 0;
    struct tiny_stream_out *out = (struct tiny_stream_out *)stream;
    struct tiny_audio_device *adev = out->adev;
    size_t frame_size = audio_stream_frame_size(&out->stream.common);
    int16_t *in_buffer = (int16_t *)buffer;
    size_t in_frames = bytes / frame_size;
    size_t out_frames;
    int buffer_type;
    int kernel_frames;
    bool sco_on;

    /*
     * acquiring hw device mutex systematically is useful if a low
     * priority thread is waiting on the output stream mutex - e.g.
     * executing out_set_parameters() while holding the hw device
     * mutex
     */
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&out->lock);
    if (out->standby) {
        ret = start_output_stream(out);
        if (ret != 0) {
            pthread_mutex_unlock(&adev->lock);
            goto exit;
        }
        out->standby = false;
    }
    buffer_type = (adev->screen_off && !adev->active_in) ?
            OUT_BUFFER_TYPE_LONG : OUT_BUFFER_TYPE_SHORT;
    sco_on = (adev->devices & AUDIO_DEVICE_OUT_ALL_SCO);
    pthread_mutex_unlock(&adev->lock);

    /* detect changes in screen ON/OFF state and adapt buffer size
     * if needed. Do not change buffer size when routed to SCO device. */
    if (!sco_on && (buffer_type != out->buffer_type)) {
        size_t period_count;

        if (buffer_type == OUT_BUFFER_TYPE_LONG)
            period_count = OUT_LONG_PERIOD_COUNT;
        else
            period_count = OUT_SHORT_PERIOD_COUNT;

        out->write_threshold = out->pcm_config->period_size * period_count;
        /* reset current threshold if exiting standby */
        if (out->buffer_type == OUT_BUFFER_TYPE_UNKNOWN)
            out->cur_write_threshold = out->write_threshold;
        out->buffer_type = buffer_type;
    }

    /* Reduce number of channels, if necessary */
    if (popcount(out_get_channels(&stream->common)) >
                 (int)out->pcm_config->channels) {
        unsigned int i;

        /* Discard right channel */
        for (i = 1; i < in_frames; i++)
            in_buffer[i] = in_buffer[i * 2];

        /* The frame size is now half */
        frame_size /= 2;
    }

    /* Change sample rate, if necessary */
    if (out_get_sample_rate(&stream->common) != out->pcm_config->rate) {
        out_frames = out->buffer_frames;
        out->resampler->resample_from_input(out->resampler,
                                            in_buffer, &in_frames,
                                            out->buffer, &out_frames);
        in_buffer = out->buffer;
    } else {
        out_frames = in_frames;
    }

    if (!sco_on) {
        int total_sleep_time_us = 0;
        size_t period_size = out->pcm_config->period_size;

        /* do not allow more than out->cur_write_threshold frames in kernel
         * pcm driver buffer */
        do {
            struct timespec time_stamp;
            if (pcm_get_htimestamp(out->pcm,
                                   (unsigned int *)&kernel_frames,
                                   &time_stamp) < 0)
                break;
            kernel_frames = pcm_get_buffer_size(out->pcm) - kernel_frames;

            if (kernel_frames > out->cur_write_threshold) {
                int sleep_time_us =
                    (int)(((int64_t)(kernel_frames - out->cur_write_threshold)
                                    * 1000000) / out->pcm_config->rate);
                if (sleep_time_us < MIN_WRITE_SLEEP_US)
                    break;
                total_sleep_time_us += sleep_time_us;
                if (total_sleep_time_us > MAX_WRITE_SLEEP_US) {
                    ALOGW("out_write() limiting sleep time %d to %d",
                          total_sleep_time_us, MAX_WRITE_SLEEP_US);
                    sleep_time_us = MAX_WRITE_SLEEP_US -
                                        (total_sleep_time_us - sleep_time_us);
                }
                usleep(sleep_time_us);
            }

        } while ((kernel_frames > out->cur_write_threshold) &&
                (total_sleep_time_us <= MAX_WRITE_SLEEP_US));

        /* do not allow abrupt changes on buffer size. Increasing/decreasing
         * the threshold by steps of 1/4th of the buffer size keeps the write
         * time within a reasonable range during transitions.
         * Also reset current threshold just above current filling status when
         * kernel buffer is really depleted to allow for smooth catching up with
         * target threshold.
         */
        if (out->cur_write_threshold > out->write_threshold) {
            out->cur_write_threshold -= period_size / 4;
            if (out->cur_write_threshold < out->write_threshold) {
                out->cur_write_threshold = out->write_threshold;
            }
        } else if (out->cur_write_threshold < out->write_threshold) {
            out->cur_write_threshold += period_size / 4;
            if (out->cur_write_threshold > out->write_threshold) {
                out->cur_write_threshold = out->write_threshold;
            }
        } else if ((kernel_frames < out->write_threshold) &&
            ((out->write_threshold - kernel_frames) >
                (int)(period_size * OUT_SHORT_PERIOD_COUNT))) {
            out->cur_write_threshold = (kernel_frames / period_size + 1) * period_size;
            out->cur_write_threshold += period_size / 4;
        }
    }

    ret = pcm_write(out->pcm, in_buffer, out_frames * frame_size);
    if (ret == -EPIPE) {
        /* In case of underrun, don't sleep since we want to catch up asap */
        pthread_mutex_unlock(&out->lock);
        return ret;
    }

exit:
    pthread_mutex_unlock(&out->lock);

    if (ret != 0) {
        usleep(bytes * 1000000 / audio_stream_frame_size(&stream->common) /
               out_get_sample_rate(&stream->common));
    }

    return bytes;
}

static int out_get_render_position(const struct audio_stream_out *stream,
                                   uint32_t *dsp_frames)
{
    return -EINVAL;
}

static int out_add_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    return 0;
}

static int out_remove_audio_effect(const struct audio_stream *stream, effect_handle_t effect)
{
    return 0;
}

static int out_get_next_write_timestamp(const struct audio_stream_out *stream,
                                        int64_t *timestamp)
{
    return -EINVAL;
}

/** audio_stream_in implementation **/
static uint32_t in_get_sample_rate(const struct audio_stream *stream)
{
    struct tiny_stream_in *in = (struct tiny_stream_in *)stream;

    return in->requested_rate;
}

static int in_set_sample_rate(struct audio_stream *stream, uint32_t rate)
{
    return 0;
}

static size_t in_get_buffer_size(const struct audio_stream *stream)
{
    struct tiny_stream_in *in = (struct tiny_stream_in *)stream;
    size_t size;

    /*
     * take resampling into account and return the closest majoring
     * multiple of 16 frames, as audioflinger expects audio buffers to
     * be a multiple of 16 frames
     */
    size = (in->pcm_config->period_size * in_get_sample_rate(stream)) /
            in->pcm_config->rate;
    size = ((size + 15) / 16) * 16;

    return size * audio_stream_frame_size((struct audio_stream *)stream);
}

static uint32_t in_get_channels(const struct audio_stream *stream)
{
    return AUDIO_CHANNEL_IN_MONO;
}

static audio_format_t in_get_format(const struct audio_stream *stream)
{
    return AUDIO_FORMAT_PCM_16_BIT;
}

static int in_set_format(struct audio_stream *stream, audio_format_t format)
{
    return -ENOSYS;
}

static int in_standby(struct audio_stream *stream)
{
    struct tiny_stream_in *in = (struct tiny_stream_in *)stream;
    int ret = 0;

    pthread_mutex_lock(&in->adev->lock);
    pthread_mutex_lock(&in->lock);
    do_in_standby(in);
    pthread_mutex_unlock(&in->lock);
    pthread_mutex_unlock(&in->adev->lock);

    return 0;
}

static int in_dump(const struct audio_stream *stream, int fd)
{
    return 0;
}

static int in_set_parameters(struct audio_stream *stream, const char *kvpairs)
{
    struct tiny_stream_in *in = (struct tiny_stream_in *)stream;
    struct tiny_audio_device *adev = in->adev;
    struct str_parms *parms;
    char value[32];
    int ret;
    unsigned int val;

    parms = str_parms_create_str(kvpairs);

    ret = str_parms_get_str(parms, AUDIO_PARAMETER_STREAM_ROUTING,
                            value, sizeof(value));
    pthread_mutex_lock(&adev->lock);
    if (ret >= 0) {
        val = atoi(value);
        if (((adev->devices & AUDIO_DEVICE_IN_ALL) != val) && (val != 0)) {
            /*
             * If SCO is turned on/off, we need to put audio into standby
             * because SCO uses a different PCM.
             */
            if ((val & AUDIO_DEVICE_IN_ALL_SCO) ^
                    (adev->devices & AUDIO_DEVICE_IN_ALL_SCO)) {
                pthread_mutex_lock(&in->lock);
                do_in_standby(in);
                pthread_mutex_unlock(&in->lock);
            }

            adev->devices &= ~AUDIO_DEVICE_IN_ALL;
            adev->devices |= val;
            select_devices(adev);
        }
    }
    pthread_mutex_unlock(&adev->lock);

    str_parms_destroy(parms);
    return ret;
}

static char * in_get_parameters(const struct audio_stream *stream,
                                const char *keys)
{
    return strdup("");
}

static int in_set_gain(struct audio_stream_in *stream, float gain)
{
    return 0;
}

static ssize_t in_read(struct audio_stream_in *stream, void* buffer,
                       size_t bytes)
{
    int ret = 0;
    struct tiny_stream_in *in = (struct tiny_stream_in *)stream;
    struct tiny_audio_device *adev = in->adev;
    size_t frames_rq = bytes / audio_stream_frame_size(&stream->common);

    /*
     * acquiring hw device mutex systematically is useful if a low
     * priority thread is waiting on the input stream mutex - e.g.
     * executing in_set_parameters() while holding the hw device
     * mutex
     */
    pthread_mutex_lock(&adev->lock);
    pthread_mutex_lock(&in->lock);
    if (in->standby) {
        ret = start_input_stream(in);
        if (ret == 0)
            in->standby = 0;
    }
    pthread_mutex_unlock(&adev->lock);

    if (ret < 0)
        goto exit;

    /*if (in->num_preprocessors != 0) {
        ret = process_frames(in, buffer, frames_rq);
    } else */if (in->resampler != NULL) {
        ret = read_frames(in, buffer, frames_rq);
    } else if (in->pcm_config->channels == 2) {
        /*
         * If the PCM is stereo, capture twice as many frames and
         * discard the right channel.
         */
        unsigned int i;
        int16_t *in_buffer = (int16_t *)buffer;

        ret = pcm_read(in->pcm, in->buffer, bytes * 2);

        /* Discard right channel */
        for (i = 0; i < frames_rq; i++)
            in_buffer[i] = in->buffer[i * 2];
    } else {
        ret = pcm_read(in->pcm, buffer, bytes);
    }

    if (ret > 0)
        ret = 0;

    /*
     * Instead of writing zeroes here, we could trust the hardware
     * to always provide zeroes when muted.
     */
    if (ret == 0 && adev->mic_mute)
        memset(buffer, 0, bytes);

exit:
    if (ret < 0)
        usleep(bytes * 1000000 / audio_stream_frame_size(&stream->common) /
               in_get_sample_rate(&stream->common));

    pthread_mutex_unlock(&in->lock);
    return bytes;
}

static uint32_t in_get_input_frames_lost(struct audio_stream_in *stream)
{
    return 0;
}

static int in_add_audio_effect(const struct audio_stream *stream,
                               effect_handle_t effect)
{
    return 0;
}

static int in_remove_audio_effect(const struct audio_stream *stream,
                                  effect_handle_t effect)
{
    return 0;
}

static int adev_open_output_stream(struct audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out)
{
    struct tiny_audio_device *adev = (struct tiny_audio_device *)dev;
    struct tiny_stream_out *out;
    int i, ret;

    out = (struct tiny_stream_out *)calloc(1, sizeof(struct tiny_stream_out));
    if (!out)
        return -ENOMEM;

    out->stream.common.get_sample_rate = out_get_sample_rate;
    out->stream.common.set_sample_rate = out_set_sample_rate;
    out->stream.common.get_buffer_size = out_get_buffer_size;
    out->stream.common.get_channels = out_get_channels;
    out->stream.common.get_format = out_get_format;
    out->stream.common.set_format = out_set_format;
    out->stream.common.standby = out_standby;
    out->stream.common.dump = out_dump;
    out->stream.common.set_parameters = out_set_parameters;
    out->stream.common.get_parameters = out_get_parameters;
    out->stream.common.add_audio_effect = out_add_audio_effect;
    out->stream.common.remove_audio_effect = out_remove_audio_effect;
    out->stream.get_latency = out_get_latency;
    out->stream.set_volume = out_set_volume;
    out->stream.write = out_write;
    out->stream.get_render_position = out_get_render_position;
    out->stream.get_next_write_timestamp = out_get_next_write_timestamp;

    out->adev = adev;

    config->format = out_get_format(&out->stream.common);
    config->channel_mask = out_get_channels(&out->stream.common);
    config->sample_rate = out_get_sample_rate(&out->stream.common);

    out->standby = true;

    *stream_out = &out->stream;
    return 0;

err_open:
    free(out);
    *stream_out = NULL;
    return ret;
}

static void adev_close_output_stream(struct audio_hw_device *dev,
                                     struct audio_stream_out *stream)
{
    out_standby(&stream->common);
    free(stream);
}

static int adev_set_parameters(struct audio_hw_device *dev, const char *kvpairs)
{
    struct tiny_audio_device *adev = (struct tiny_audio_device *)dev;
    struct str_parms *parms;
    char *str;
    char value[32];
    int ret;
    ALOGD("adev_set_parameters\n");

    parms = str_parms_create_str(kvpairs);
    ret = str_parms_get_str(parms, "orientation", value, sizeof(value));
    if (ret >= 0) {
        int j, orientation = 0;
        ALOGD("Parm orientation '%s'\n", value);
        if (strcmp(value, "portrait") == 0)
            orientation = 1;

        pthread_mutex_lock(&adev->lock);
        if (orientation != adev->orientation) {
            struct mixer_ctl *ctl;
            adev->orientation = orientation;

            select_devices(adev);
            ctl = mixer_get_ctl_by_name(adev->mixer, "Speaker Orientation");
            if(!ctl)
                ALOGE("Unknown control 'Speaker Orientation'\n");
            else {
                for (j = 0; j < mixer_ctl_get_num_values(ctl); j++) {
                    ret = mixer_ctl_set_value(ctl, j, orientation);
                    if (ret != 0) {
                        ALOGE("Failed to set 'Speaker Orientation'.%d to %d\n",
                            j, orientation);
                    } else {
                        ALOGV("Set 'Speaker Orientation'.%d to %d\n",
                            j, orientation);
                    }
                }
            }
                
        }
        pthread_mutex_unlock(&adev->lock);
    }

    ret = str_parms_get_str(parms, "screen_state", value, sizeof(value));
    if (ret >= 0) {
        if (strcmp(value, AUDIO_PARAMETER_VALUE_ON) == 0)
            adev->screen_off = false;
        else
            adev->screen_off = true;
    }
    str_parms_destroy(parms);
    return ret;
}

static char * adev_get_parameters(const struct audio_hw_device *dev,
                                  const char *keys)
{
    return strdup("");
}

static int adev_init_check(const struct audio_hw_device *dev)
{
    return 0;
}

static int adev_set_voice_volume(struct audio_hw_device *dev, float volume)
{
    return -ENOSYS;
}

static int adev_set_master_volume(struct audio_hw_device *dev, float volume)
{
    return -ENOSYS;
}

static int adev_set_mode(struct audio_hw_device *dev, audio_mode_t mode)
{
    return 0;
}

static int adev_set_mic_mute(struct audio_hw_device *dev, bool state)
{
    struct tiny_audio_device *adev = (struct tiny_audio_device *)dev;

    adev->mic_mute = state;

    return 0;
}

static int adev_get_mic_mute(const struct audio_hw_device *dev, bool *state)
{
    struct tiny_audio_device *adev = (struct tiny_audio_device *)dev;

    *state = adev->mic_mute;

    return 0;
}

static size_t adev_get_input_buffer_size(const struct audio_hw_device *dev,
                                         const struct audio_config *config)
{
    size_t size;

    /*
     * take resampling into account and return the closest majoring
     * multiple of 16 frames, as audioflinger expects audio buffers to
     * be a multiple of 16 frames
     */
    size = (pcm_config_in.period_size * config->sample_rate) / pcm_config_in.rate;
    size = ((size + 15) / 16) * 16;

    return (size * popcount(config->channel_mask) *
                audio_bytes_per_sample(config->format));
}

static int adev_open_input_stream(struct audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in)
{
    struct tiny_audio_device *adev = (struct tiny_audio_device *)dev;
    struct tiny_stream_in *in;
    int ret;

    *stream_in = NULL;

    /* Respond with a request for mono if a different format is given. */
    if (config->channel_mask != AUDIO_CHANNEL_IN_MONO) {
        config->channel_mask = AUDIO_CHANNEL_IN_MONO;
        return -EINVAL;
    }

    in = (struct tiny_stream_in *)calloc(1, sizeof(struct tiny_stream_in));
    if (!in)
        return -ENOMEM;

    in->stream.common.get_sample_rate = in_get_sample_rate;
    in->stream.common.set_sample_rate = in_set_sample_rate;
    in->stream.common.get_buffer_size = in_get_buffer_size;
    in->stream.common.get_channels = in_get_channels;
    in->stream.common.get_format = in_get_format;
    in->stream.common.set_format = in_set_format;
    in->stream.common.standby = in_standby;
    in->stream.common.dump = in_dump;
    in->stream.common.set_parameters = in_set_parameters;
    in->stream.common.get_parameters = in_get_parameters;
    in->stream.common.add_audio_effect = in_add_audio_effect;
    in->stream.common.remove_audio_effect = in_remove_audio_effect;
    in->stream.set_gain = in_set_gain;
    in->stream.read = in_read;
    in->stream.get_input_frames_lost = in_get_input_frames_lost;

    in->adev = adev;
    in->standby = true;
    in->requested_rate = config->sample_rate;
    in->pcm_config = &pcm_config_in; /* default PCM config */

    *stream_in = &in->stream;
    return 0;
}

static void adev_close_input_stream(struct audio_hw_device *dev,
                                   struct audio_stream_in *stream)
{
    struct tiny_stream_in *in = (struct tiny_stream_in *)stream;

    in_standby(&stream->common);
    free(stream);
}

static int adev_dump(const audio_hw_device_t *device, int fd)
{
    return 0;
}

static int adev_close(hw_device_t *device)
{
    free(device);
    return 0;
}

static uint32_t adev_get_supported_devices(const struct audio_hw_device *dev)
{
    struct tiny_audio_device *adev = (struct tiny_audio_device *)dev;
    uint32_t supported = 0;
    int i;

    for (i = 0; i < adev->num_dev_cfgs; i++)
        supported |= adev->dev_cfgs[i].mask;

    return supported;
}

static int adev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device)
{
    struct tiny_audio_device *adev;
    int ret;

    if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    adev = calloc(1, sizeof(struct tiny_audio_device));
    if (!adev)
        return -ENOMEM;

    adev->hw_device.common.tag = HARDWARE_DEVICE_TAG;
    adev->hw_device.common.version = AUDIO_DEVICE_API_VERSION_1_0;
    adev->hw_device.common.module = (struct hw_module_t *) module;
    adev->hw_device.common.close = adev_close;

    adev->hw_device.get_supported_devices = adev_get_supported_devices;
    adev->hw_device.init_check = adev_init_check;
    adev->hw_device.set_voice_volume = adev_set_voice_volume;
    adev->hw_device.set_master_volume = adev_set_master_volume;
    adev->hw_device.set_mode = adev_set_mode;
    adev->hw_device.set_mic_mute = adev_set_mic_mute;
    adev->hw_device.get_mic_mute = adev_get_mic_mute;
    adev->hw_device.set_parameters = adev_set_parameters;
    adev->hw_device.get_parameters = adev_get_parameters;
    adev->hw_device.get_input_buffer_size = adev_get_input_buffer_size;
    adev->hw_device.open_output_stream = adev_open_output_stream;
    adev->hw_device.close_output_stream = adev_close_output_stream;
    adev->hw_device.open_input_stream = adev_open_input_stream;
    adev->hw_device.close_input_stream = adev_close_input_stream;
    adev->hw_device.dump = adev_dump;

    adev->mixer = mixer_open(0);
    if (!adev->mixer) {
        ALOGE("Failed to open mixer 0\n");
        goto err;
    }
    
    ret = adev_config_parse(adev);
    if (ret != 0)
        goto err_mixer;

    *device = &adev->hw_device.common;

    return 0;

err_mixer:
    mixer_close(adev->mixer);
err:
    return -EINVAL;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "startablet audio HW HAL",
        .author = "Mark Brown <broonie@opensource.wolfsonmicro.com>",
        .methods = &hal_module_methods,
    },
};
