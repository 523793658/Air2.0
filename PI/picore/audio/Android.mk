LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/android/audio.c \

LOCAL_C_INCLUDES := \
    $(JNI_H_INCLUDE) \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/../lib/include
    
LOCAL_SHARED_LIBRARIES := \
    libpi_lib
    
LOCAL_CFLAGS := -Wall

LOCAL_LDLIBS += -lz -llog -ldl

LOCAL_MODULE:= libpi_audio

include $(BUILD_SHARED_LIBRARY)
