$(call inherit-product, build/target/product/full_base.mk)

# The gps config appropriate for this device
$(call inherit-product, device/common/gps/gps_us_supl.mk)

# Temp make file for pulling the local vendor tree -- Aaron Echols
$(call inherit-product, $(LOCAL_PATH)/device.mk)

#$(call inherit-product-if-exists, vendor/lge/v909/v909-vendor.mk)

DEVICE_PACKAGE_OVERLAYS += device/lge/v909/overlay

$(call inherit-product-if-exists, hardware/broadcom/wlan/bcmdhd/firmware/bcm4329/device-bcm.mk)

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/media_profiles.xml:system/etc/media_profiles.xml \
    $(LOCAL_PATH)/egl.cfg:system/lib/egl/egl.cfg \
    $(LOCAL_PATH)/prebuilt/rild:system/bin/rild \
    $(LOCAL_PATH)/wpa_supplicant.conf:data/misc/wifi/wpa_supplicant.conf

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/init.startablet.rc:root/init.startablet.rc \
    $(LOCAL_PATH)/init.startablet.usb.rc:root/init.startablet.usb.rc \
    $(LOCAL_PATH)/ueventd.startablet.rc:root/ueventd.startablet.rc \
    $(LOCAL_PATH)/fstab.v909:root/fstab.v909

PRODUCT_COPY_FILES += \
    $(LOCAL_PATH)/keylayout/gpio-keys.kl:system/usr/keylayout/gpio-keys.kl \
    $(LOCAL_PATH)/atmel-maxtouch.idc:system/usr/idc/atmel-maxtouch.idc

PRODUCT_COPY_FILES += \
     $(LOCAL_PATH)/gps.xml:system/etc/gps.xml \
     $(LOCAL_PATH)/gps.conf:system/etc/gps.conf

PRODUCT_COPY_FILES += \
     $(LOCAL_PATH)/tiny_hw.xml:/system/etc/sound/tiny_hw.xml

PRODUCT_PROPERTY_OVERRIDES := \
    wifi.interface=eth0 \
    wifi.supplicant_scan_interval=15

PRODUCT_COPY_FILES += \
     $(LOCAL_PATH)/permissions/com.google.widevine.software.drm.xml:system/etc/permissions/com.google.widevine.software.drm.xml

PRODUCT_PACKAGES += \
    HoloSpiralWallpaper \
    LiveWallpapersPicker \
    VisualizationWallpapers \
    Camera

## LGE stuffs
PRODUCT_PACKAGES += \
    LGEServices \
    bridgeutil \
    libbridges \
    libbridge \
    libbridge_jni \
    libaudioutils \
    libtinyalsa \
    audio.primary.startablet \
    lights.startablet \
    secureclockd \
    libsecureclock \
    screencap \
    hwprops \
    com.android.future.usb.accessory \
    e2fsck \
    make_ext4fs \
    vold \
    setup_fs

PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

PRODUCT_CHARACTERISTICS := tablet,nosdcard

PRODUCT_BUILD_PROP_OVERRIDES += BUILD_UTC_DATE=0
PRODUCT_NAME := lge_v909
PRODUCT_DEVICE := v909
PRODUCT_MODEL := LG-V909
