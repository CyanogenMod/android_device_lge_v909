LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ALL_PREBUILT += $(INSTALLED_KERNEL_TARGET)

subdir_makefiles:= \
    $(LOCAL_PATH)/audio/Android.mk

# include the non-open-source counterpart to this file
-include vendor/lge/v909/AndroidBoardVendor.mk
