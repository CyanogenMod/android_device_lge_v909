# Copyright (C) 2011 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Prebuilt libraries that are needed to build open-source libraries
PREBUILT_PATH := $(LOCAL_PATH)/proprietary

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/init.startablet.rc:root/init.startablet.rc \
    $(LOCAL_PATH)/init.startablet.usb.rc:root/init.startablet.usb.rc \
    $(LOCAL_PATH)/ueventd.startablet.rc:root/ueventd.startablet.rc \
    $(LOCAL_PATH)/fstab.v909:root/fstab.v909 \
    $(PREBUILT_PATH)/bin/aleglstream:system/bin/aleglstream \
    $(PREBUILT_PATH)/bin/alvcplayer:system/bin/alvcplayer \
    $(PREBUILT_PATH)/bin/alvcrecorder:system/bin/alvcrecorder \
    $(PREBUILT_PATH)/bin/avp_load:system/bin/avp_load \
    $(PREBUILT_PATH)/bin/brcm_patchram_plus_lge:system/bin/brcm_patchram_plus_lge \
    $(PREBUILT_PATH)/bin/bdaddr_init:system/bin/bdaddr_init \
    $(PREBUILT_PATH)/bin/bluetoothd:system/bin/bluetoothd \
    $(PREBUILT_PATH)/bin/bluetoothhiddend:system/bin/bluetoothhiddend \
    $(PREBUILT_PATH)/bin/cal_test_only:system/bin/cal_test_only \
    $(PREBUILT_PATH)/bin/clear-kernel-log:system/bin/clear-kernel-log \
    $(PREBUILT_PATH)/bin/etalog:system/bin/etalog \
    $(PREBUILT_PATH)/bin/eventd:system/bin/eventd \
    $(PREBUILT_PATH)/bin/lg_calibration:system/bin/lg_calibration \
    $(PREBUILT_PATH)/bin/lg_check_cal_pass:system/bin/lg_check_cal_pass \
    $(PREBUILT_PATH)/bin/lg_check_cal_result:system/bin/lg_check_cal_result \
    $(PREBUILT_PATH)/bin/lgemodem:system/bin/lgemodem \
    $(PREBUILT_PATH)/bin/lgesystemd:system/bin/lgesystemd \
    $(PREBUILT_PATH)/bin/lg_golden_test:system/bin/lg_golden_test \
    $(PREBUILT_PATH)/bin/lg_init_calibration:system/bin/lg_init_calibration \
    $(PREBUILT_PATH)/bin/motion:system/bin/motion \
    $(PREBUILT_PATH)/bin/multiplayer:system/bin/multiplayer \
    $(PREBUILT_PATH)/bin/nv_hciattach:system/bin/nv_hciattach \
    $(PREBUILT_PATH)/bin/nvcpud:system/bin/nvcpud \
    $(PREBUILT_PATH)/bin/nvdmmultidisplay:system/bin/nvdmmultidisplay \
    $(PREBUILT_PATH)/bin/nvtest:system/bin/nvtest \
    $(PREBUILT_PATH)/bin/omxplayer2:system/bin/omxplayer2 \
    $(PREBUILT_PATH)/bin/tegrastats:system/bin/tegrastats \
    $(PREBUILT_PATH)/bin/wifihiddend:system/bin/wifihiddend \
    $(PREBUILT_PATH)/bin/wl:system/bin/wl \
    $(PREBUILT_PATH)/bin/write_lgit_val:system/bin/write_lgit_val \
    $(PREBUILT_PATH)/etc/audio_policy.conf:system/etc/audio_policy.conf \
    $(PREBUILT_PATH)/etc/media_codecs.xml:system/etc/media_codecs.xml \
    $(PREBUILT_PATH)/etc/media_profiles.xml:system/etc/media_profiles.xml \
    $(PREBUILT_PATH)/etc/nvcamera.conf:system/etc/nvcamera.conf \
    $(PREBUILT_PATH)/etc/save_kernel_log.sh:system/etc/save_kernel_log.sh \
    $(PREBUILT_PATH)/etc/vold.conf:system/etc/vold.conf \
    $(PREBUILT_PATH)/etc/vold.fstab:system/etc/vold.fstab \
    $(PREBUILT_PATH)/etc/bluetooth/BCM4329B1_002.002.023.0875.0883.hcd:system/etc/bluetooth/BCM4329B1_002.002.023.0875.0883.hcd \
    $(PREBUILT_PATH)/etc/bluetooth/main.conf:system/etc/bluetooth/main.conf \
    $(PREBUILT_PATH)/etc/dhcpcd/dhcpcd.conf:system/etc/dhcpcd/dhcpcd.conf \
    $(PREBUILT_PATH)/etc/firmware/arb_test.axf:system/etc/firmware/arb_test.axf \
    $(PREBUILT_PATH)/etc/firmware/load_test0.axf:system/etc/firmware/load_test0.axf \
    $(PREBUILT_PATH)/etc/firmware/load_test1.axf:system/etc/firmware/load_test1.axf \
    $(PREBUILT_PATH)/etc/firmware/load_test2.axf:system/etc/firmware/load_test2.axf \
    $(PREBUILT_PATH)/etc/firmware/load_test3.axf:system/etc/firmware/load_test3.axf \
    $(PREBUILT_PATH)/etc/firmware/load_test4.axf:system/etc/firmware/load_test4.axf \
    $(PREBUILT_PATH)/etc/firmware/memory_stress.axf:system/etc/firmware/memory_stress.axf \
    $(PREBUILT_PATH)/etc/firmware/nvavp_os_00001000.bin:system/etc/firmware/nvavp_os_00001000.bin \
    $(PREBUILT_PATH)/etc/firmware/nvavp_os_0ff00000.bin:system/etc/firmware/nvavp_os_0ff00000.bin \
    $(PREBUILT_PATH)/etc/firmware/nvavp_os_e0000000.bin:system/etc/firmware/nvavp_os_e0000000.bin \
    $(PREBUILT_PATH)/etc/firmware/nvavp_os_eff00000.bin:system/etc/firmware/nvavp_os_eff00000.bin \
    $(PREBUILT_PATH)/etc/firmware/nvavp_vid_ucode_alt.bin:system/etc/firmware/nvavp_vid_ucode_alt.bin \
    $(PREBUILT_PATH)/etc/firmware/nvavp_vid_ucode.bin:system/etc/firmware/nvavp_vid_ucode.bin \
    $(PREBUILT_PATH)/etc/firmware/nvmm_aacdec.axf:system/etc/firmware/nvmm_aacdec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_adtsdec.axf:system/etc/firmware/nvmm_adtsdec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_h264dec2x.axf:system/etc/firmware/nvmm_h264dec2x.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_h264dec.axf:system/etc/firmware/nvmm_h264dec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_jpegdec.axf:system/etc/firmware/nvmm_jpegdec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_jpegenc.axf:system/etc/firmware/nvmm_jpegenc.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_manager.axf:system/etc/firmware/nvmm_manager.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_mp3dec.axf:system/etc/firmware/nvmm_mp3dec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_mpeg2dec.axf:system/etc/firmware/nvmm_mpeg2dec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_mpeg4dec.axf:system/etc/firmware/nvmm_mpeg4dec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_reference.axf:system/etc/firmware/nvmm_reference.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_service.axf:system/etc/firmware/nvmm_service.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_vc1dec_2x.axf:system/etc/firmware/nvmm_vc1dec_2x.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_vc1dec.axf:system/etc/firmware/nvmm_vc1dec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_wavdec.axf:system/etc/firmware/nvmm_wavdec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_wmadec.axf:system/etc/firmware/nvmm_wmadec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvmm_wmaprodec.axf:system/etc/firmware/nvmm_wmaprodec.axf \
    $(PREBUILT_PATH)/etc/firmware/nvrm_avp_0ff00000.bin:system/etc/firmware/nvrm_avp_0ff00000.bin \
    $(PREBUILT_PATH)/etc/firmware/nvrm_avp_8e000000.bin:system/etc/firmware/nvrm_avp_8e000000.bin \
    $(PREBUILT_PATH)/etc/firmware/nvrm_avp_9e000000.bin:system/etc/firmware/nvrm_avp_9e000000.bin \
    $(PREBUILT_PATH)/etc/firmware/nvrm_avp_be000000.bin:system/etc/firmware/nvrm_avp_be000000.bin \
    $(PREBUILT_PATH)/etc/firmware/nvrm_avp.bin:system/etc/firmware/nvrm_avp.bin \
    $(PREBUILT_PATH)/etc/firmware/nvrm_avp_eff00000.bin:system/etc/firmware/nvrm_avp_eff00000.bin \
    $(PREBUILT_PATH)/etc/firmware/transport_stress.axf:system/etc/firmware/transport_stress.axf \
    $(PREBUILT_PATH)/etc/spn-conf.xml:system/etc/spn-conf.xml \
    $(PREBUILT_PATH)/etc/sound/tiny_hw.xml:system/etc/sound/tiny_hw.xml \
    $(PREBUILT_PATH)/etc/wifi/config:system/etc/wifi/config \
    $(PREBUILT_PATH)/etc/wifi/nvram.txt:system/etc/wifi/nvram.txt \
    $(PREBUILT_PATH)/etc/wifi/wpa_supplicant.conf:system/etc/wifi/wpa_supplicant.conf \
    $(PREBUILT_PATH)/lib/lge-ril.so:system/lib/lge-ril.so \
    $(PREBUILT_PATH)/lib/libardrv_dynamic.so:system/lib/libardrv_dynamic.so \
    $(PREBUILT_PATH)/lib/libbcmwl.so:system/lib/libbcmwl.so \
    $(PREBUILT_PATH)/lib/libblueset.so:system/lib/libblueset.so \
    $(PREBUILT_PATH)/lib/libcgdrv.so:system/lib/libcgdrv.so \
    $(PREBUILT_PATH)/lib/libeglstreamtexture.so:system/lib/libeglstreamtexture.so \
    $(PREBUILT_PATH)/lib/liblgcalibration.so:system/lib/liblgcalibration.so \
    $(PREBUILT_PATH)/lib/liblgerft.so:system/lib/liblgerft.so \
    $(PREBUILT_PATH)/lib/liblgopencv.so:system/lib/liblgopencv.so \
    $(PREBUILT_PATH)/lib/libnvapputil.so:system/lib/libnvapputil.so \
    $(PREBUILT_PATH)/lib/libnvasfparserhal.so:system/lib/libnvasfparserhal.so \
    $(PREBUILT_PATH)/lib/libnvaviparserhal.so:system/lib/libnvaviparserhal.so \
    $(PREBUILT_PATH)/lib/libnvavp.so:system/lib/libnvavp.so \
    $(PREBUILT_PATH)/lib/libnvcapclk.so:system/lib/libnvcapclk.so \
    $(PREBUILT_PATH)/lib/libnvcap.so:system/lib/libnvcap.so \
    $(PREBUILT_PATH)/lib/libnvcap_video.so:system/lib/libnvcap_video.so \
    $(PREBUILT_PATH)/lib/libnvcontrol_jni.so:system/lib/libnvcontrol_jni.so \
    $(PREBUILT_PATH)/lib/libnvcpud_client.so:system/lib/libnvcpud_client.so \
    $(PREBUILT_PATH)/lib/libnvcpud.so:system/lib/libnvcpud.so \
    $(PREBUILT_PATH)/lib/libnvddk_2d.so:system/lib/libnvddk_2d.so \
    $(PREBUILT_PATH)/lib/libnvddk_2d_v2.so:system/lib/libnvddk_2d_v2.so \
    $(PREBUILT_PATH)/lib/libnvdispmgr_d.so:system/lib/libnvdispmgr_d.so \
    $(PREBUILT_PATH)/lib/libnvhdmi3dplay_jni.so:system/lib/libnvhdmi3dplay_jni.so \
    $(PREBUILT_PATH)/lib/libnvmm_asfparser.so:system/lib/libnvmm_asfparser.so \
    $(PREBUILT_PATH)/lib/libnvmm_audio.so:system/lib/libnvmm_audio.so \
    $(PREBUILT_PATH)/lib/libnvmm_aviparser.so:system/lib/libnvmm_aviparser.so \
    $(PREBUILT_PATH)/lib/libnvmm_camera.so:system/lib/libnvmm_camera.so \
    $(PREBUILT_PATH)/lib/libnvmm_contentpipe.so:system/lib/libnvmm_contentpipe.so \
    $(PREBUILT_PATH)/lib/libnvmm_image.so:system/lib/libnvmm_image.so \
    $(PREBUILT_PATH)/lib/libnvmmlite_audio.so:system/lib/libnvmmlite_audio.so \
    $(PREBUILT_PATH)/lib/libnvmmlite_image.so:system/lib/libnvmmlite_image.so \
    $(PREBUILT_PATH)/lib/libnvmmlite_msaudio.so:system/lib/libnvmmlite_msaudio.so \
    $(PREBUILT_PATH)/lib/libnvmmlite.so:system/lib/libnvmmlite.so \
    $(PREBUILT_PATH)/lib/libnvmmlite_utils.so:system/lib/libnvmmlite_utils.so \
    $(PREBUILT_PATH)/lib/libnvmmlite_video.so:system/lib/libnvmmlite_video.so \
    $(PREBUILT_PATH)/lib/libnvmm_manager.so:system/lib/libnvmm_manager.so \
    $(PREBUILT_PATH)/lib/libnvmm_misc.so:system/lib/libnvmm_misc.so \
    $(PREBUILT_PATH)/lib/libnvmm_msaudio.so:system/lib/libnvmm_msaudio.so \
    $(PREBUILT_PATH)/lib/libnvmm_parser.so:system/lib/libnvmm_parser.so \
    $(PREBUILT_PATH)/lib/libnvmm_service.so:system/lib/libnvmm_service.so \
    $(PREBUILT_PATH)/lib/libnvmm.so:system/lib/libnvmm.so \
    $(PREBUILT_PATH)/lib/libnvmm_utils.so:system/lib/libnvmm_utils.so \
    $(PREBUILT_PATH)/lib/libnvmm_vc1_video.so:system/lib/libnvmm_vc1_video.so \
    $(PREBUILT_PATH)/lib/libnvmm_video.so:system/lib/libnvmm_video.so \
    $(PREBUILT_PATH)/lib/libnvmm_writer.so:system/lib/libnvmm_writer.so \
    $(PREBUILT_PATH)/lib/libnvodm_dtvtuner.so:system/lib/libnvodm_dtvtuner.so \
    $(PREBUILT_PATH)/lib/libnvodm_hdmi.so:system/lib/libnvodm_hdmi.so \
    $(PREBUILT_PATH)/lib/libnvodm_imager.so:system/lib/libnvodm_imager.so \
    $(PREBUILT_PATH)/lib/libnvodm_misc.so:system/lib/libnvodm_misc.so \
    $(PREBUILT_PATH)/lib/libnvodm_query.so:system/lib/libnvodm_query.so \
    $(PREBUILT_PATH)/lib/libnvomxadaptor.so:system/lib/libnvomxadaptor.so \
    $(PREBUILT_PATH)/lib/libnvomxilclient.so:system/lib/libnvomxilclient.so \
    $(PREBUILT_PATH)/lib/libnvomx.so:system/lib/libnvomx.so \
    $(PREBUILT_PATH)/lib/libnvos.so:system/lib/libnvos.so \
    $(PREBUILT_PATH)/lib/libnvparser.so:system/lib/libnvparser.so \
    $(PREBUILT_PATH)/lib/libnvrm_graphics.so:system/lib/libnvrm_graphics.so \
    $(PREBUILT_PATH)/lib/libnvrm.so:system/lib/libnvrm.so \
    $(PREBUILT_PATH)/lib/libnvsm.so:system/lib/libnvsm.so \
    $(PREBUILT_PATH)/lib/libnvstereoutils_jni.so:system/lib/libnvstereoutils_jni.so \
    $(PREBUILT_PATH)/lib/libnvsystemuiext_jni.so:system/lib/libnvsystemuiext_jni.so \
    $(PREBUILT_PATH)/lib/libnvtestio.so:system/lib/libnvtestio.so \
    $(PREBUILT_PATH)/lib/libnvtestresults.so:system/lib/libnvtestresults.so \
    $(PREBUILT_PATH)/lib/libnvtvmr.so:system/lib/libnvtvmr.so \
    $(PREBUILT_PATH)/lib/libnvwinsys.so:system/lib/libnvwinsys.so \
    $(PREBUILT_PATH)/lib/libnvwsi.so:system/lib/libnvwsi.so \
    $(PREBUILT_PATH)/lib/libril.so:system/lib/libril.so \
    $(PREBUILT_PATH)/lib/libstagefrighthw.so:system/lib/libstagefrighthw.so \
    $(PREBUILT_PATH)/lib/omxplayer.so:system/lib/omxplayer.so \
    $(PREBUILT_PATH)/lib/egl/libEGL_perfhud.so:system/lib/egl/libEGL_perfhud.so \
    $(PREBUILT_PATH)/lib/egl/libEGL_tegra.so:system/lib/egl/libEGL_tegra.so \
    $(PREBUILT_PATH)/lib/egl/libGLES_android.so:system/lib/egl/libGLES_android.so \
    $(PREBUILT_PATH)/lib/egl/libGLESv1_CM_perfhud.so:system/lib/egl/libGLESv1_CM_perfhud.so \
    $(PREBUILT_PATH)/lib/egl/libGLESv1_CM_tegra.so:system/lib/egl/libGLESv1_CM_tegra.so \
    $(PREBUILT_PATH)/lib/egl/libGLESv2_perfhud.so:system/lib/egl/libGLESv2_perfhud.so \
    $(PREBUILT_PATH)/lib/egl/libGLESv2_tegra.so:system/lib/egl/libGLESv2_tegra.so \
    $(PREBUILT_PATH)/lib/hw/camera.tegra.so:system/lib/hw/camera.tegra.so \
    $(PREBUILT_PATH)/lib/hw/gps.startablet.so:system/lib/hw/gps.startablet.so \
    $(PREBUILT_PATH)/lib/hw/gralloc.tegra.so:system/lib/hw/gralloc.tegra.so \
    $(PREBUILT_PATH)/lib/hw/hwcomposer.tegra.so:system/lib/hw/hwcomposer.tegra.so \
    $(PREBUILT_PATH)/lib/hw/lights.startablet.so:system/lib/hw/lights.startablet.so \
    $(PREBUILT_PATH)/lib/hw/sensors.startablet.so:system/lib/hw/sensors.startablet.so \
    $(PREBUILT_PATH)/lib/hw/sensors.tegra.so:system/lib/hw/sensors.tegra.so \
    $(PREBUILT_PATH)/usr/idc/atmel-maxtouch.idc:system/usr/idc/atmel-maxtouch.idc

PRODUCT_COPY_FILES += \
    system/core/rootdir/init.trace.rc:root/init.trace.rc \
    system/core/rootdir/init.usb.rc:root/init.usb.rc

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.camera.xml:system/etc/permissions/android.hardware.camera.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.location.xml:system/etc/permissions/android.hardware.location.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/etc/permissions/android.hardware.sensor.compass.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.xml:system/etc/permissions/android.hardware.touchscreen.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/etc/permissions/android.hardware.wifi.direct.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.software.sip.xml:system/etc/permissions/android.software.sip.xml \
    frameworks/native/data/etc/tablet_core_hardware.xml:system/etc/permissions/tablet_core_hardware.xml \
    packages/wallpapers/LivePicker/android.software.live_wallpaper.xml:system/etc/permissions/android.software.live_wallpaper.xml

ifneq ($(TARGET_PREBUILT_KERNEL),)
    PRODUCT_COPY_FILES += \
        $(TARGET_PREBUILT_KERNEL):kernel
endif

