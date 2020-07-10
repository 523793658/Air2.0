LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/intersection.c \
    src/line.c \
    src/matrix4.c \
    src/obb.c \
    src/quaternion.c \
    src/random.c \
    src/sequence.c \
    src/spatial.c \
    src/vector3.c

LOCAL_C_INCLUDES := \
    $(JNI_H_INCLUDE) \
    $(LOCAL_PATH)/include \
    $(LOCAL_PATH)/../lib/include \

LOCAL_SHARED_LIBRARIES := \
    libpi_lib

LOCAL_CFLAGS := -Wall

LOCAL_LDLIBS += -lz -llog -ldl

LOCAL_MODULE:= libpi_math

include $(BUILD_SHARED_LIBRARY)
