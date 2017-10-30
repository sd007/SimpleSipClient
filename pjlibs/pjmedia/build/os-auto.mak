# pjmedia/build/os-auto.mak.  Generated from os-auto.mak.in by configure.

# Define the desired video device backend
# Valid values are:
#   - darwin_os
#   - android_os
AC_PJMEDIA_VIDEO = 

# SDL flags
SDL_CFLAGS = -DPJMEDIA_VIDEO_DEV_HAS_SDL=1 -I/usr/include/SDL2 -D_REENTRANT
SDL_LDFLAGS = -L/usr/lib/x86_64-linux-gnu -lSDL2

# FFMPEG dlags
FFMPEG_CFLAGS =   -DPJMEDIA_USE_OLD_FFMPEG=1 
FFMPEG_LDFLAGS =   

# Video4Linux2
V4L2_CFLAGS = 
V4L2_LDFLAGS = 

# QT
AC_PJMEDIA_VIDEO_HAS_QT = 
# QT_CFLAGS = 

# Darwin (Mac and iOS)
DARWIN_CFLAGS = 

# Android
ANDROID_CFLAGS = 

# openh264
OPENH264_CFLAGS = 
OPENH264_LDFLAGS = 


# PJMEDIA features exclusion
export CFLAGS +=    \
		 $(SDL_CFLAGS) $(FFMPEG_CFLAGS) $(V4L2_CFLAGS) $(QT_CFLAGS) \
		 $(DARWIN_CFLAGS) $(ANDROID_CFLAGS) \
		 $(OPENH264_CFLAGS)
export LDFLAGS += $(SDL_LDFLAGS) $(FFMPEG_LDFLAGS) $(V4L2_LDFLAGS) \
		  $(OPENH264_LDFLAGS)

# Define the desired sound device backend
# Valid values are:
#   - pa_unix:	    	PortAudio on Unix (OSS or ALSA)
#   - pa_darwinos:  	PortAudio on MacOSX (CoreAudio)
#   - pa_old_darwinos:  PortAudio on MacOSX (old CoreAudio, for OSX 10.2)
#   - pa_win32:	    	PortAudio on Win32 (WMME)
#   - win32:	    	Win32 MME (wmme_dev.c)
#   - coreaudio:	MaxOSX CoreAudio (coreaudio_dev.m)
#   - alsa:		Unix ALSA (alsa_dev.c)
#   - null:	    	Null sound device (nullsound.c)
#   - external:		Link with no sounddev (app will provide)
AC_PJMEDIA_SND=alsa

# For Unix, specify if ALSA should be supported
AC_PA_USE_ALSA=1

# Additional PortAudio CFLAGS are in  -DHAVE_SYS_SOUNDCARD_H -DHAVE_LINUX_SOUNDCARD_H -DPA_LITTLE_ENDIAN

#
# Codecs
#
AC_NO_G711_CODEC=1
AC_NO_L16_CODEC=1
AC_NO_GSM_CODEC=1
AC_NO_SPEEX_CODEC=
AC_NO_ILBC_CODEC=
AC_NO_G722_CODEC=1
AC_NO_G7221_CODEC=
AC_NO_OPENCORE_AMRNB=1
AC_NO_OPENCORE_AMRWB=1
AC_NO_BCG729=1

export CODEC_OBJS=

export PJMEDIA_AUDIODEV_OBJS +=  

ifeq ($(AC_NO_G711_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_G711_CODEC=0
else
export CODEC_OBJS +=
endif

ifeq ($(AC_NO_L16_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_L16_CODEC=0
else
export CODEC_OBJS += l16.o
endif

ifeq ($(AC_NO_GSM_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_GSM_CODEC=0
else
export CODEC_OBJS += gsm.o
endif

ifeq ($(AC_NO_SPEEX_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_SPEEX_CODEC=0
else
export CFLAGS += -I$(THIRD_PARTY)/build/speex -I$(THIRD_PARTY)/speex/include
export CODEC_OBJS += speex_codec.o

ifeq (,)
export PJMEDIA_OBJS += echo_speex.o
endif

endif

ifeq ($(AC_NO_ILBC_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_ILBC_CODEC=0
else
export CODEC_OBJS += ilbc.o
endif

ifeq ($(AC_NO_G722_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_G722_CODEC=0
else
export CODEC_OBJS += g722.o g722/g722_enc.o g722/g722_dec.o
endif

ifeq ($(AC_NO_G7221_CODEC),1)
export CFLAGS += -DPJMEDIA_HAS_G7221_CODEC=0
else
export CODEC_OBJS += g7221.o
export G7221_CFLAGS += -I$(THIRD_PARTY)
endif

ifeq ($(AC_NO_OPENCORE_AMRNB),1)
export CFLAGS += -DPJMEDIA_HAS_OPENCORE_AMRNB_CODEC=0
else
export CODEC_OBJS += opencore_amr.o
endif

ifeq ($(AC_NO_OPENCORE_AMRWB),1)
export CFLAGS += -DPJMEDIA_HAS_OPENCORE_AMRWB_CODEC=0
else
ifeq ($(AC_NO_OPENCORE_AMRNB),1)
export CODEC_OBJS += opencore_amr.o
endif
endif

ifeq ($(AC_NO_BCG729),)
export CODEC_OBJS += bcg729.o
endif


#
# SRTP
#
ifneq (0,0)
# External SRTP
export CFLAGS += -DPJMEDIA_EXTERNAL_SRTP=0
# SRTP srtp_deinit()/srtp_shutdown() API availability settings
export CFLAGS += -DPJMEDIA_SRTP_HAS_DEINIT= \
		 -DPJMEDIA_SRTP_HAS_SHUTDOWN=
else
# Our SRTP in third_party
export CFLAGS += -I$(THIRD_PARTY)/build/srtp \
	 -I$(THIRD_PARTY)/srtp/crypto/include \
	 -I$(THIRD_PARTY)/srtp/include

endif

#
# Resample
#
AC_PJMEDIA_RESAMPLE=libresample

ifeq ($(AC_PJMEDIA_RESAMPLE),none)
# No resample support
export CFLAGS += -DPJMEDIA_RESAMPLE_IMP=PJMEDIA_RESAMPLE_NONE
endif

ifeq ($(AC_PJMEDIA_RESAMPLE),libresample)
export CFLAGS += -DPJMEDIA_RESAMPLE_IMP=PJMEDIA_RESAMPLE_LIBRESAMPLE
endif

ifeq ($(AC_PJMEDIA_RESAMPLE),libsamplerate)
export CFLAGS += -DPJMEDIA_RESAMPLE_IMP=PJMEDIA_RESAMPLE_LIBSAMPLERATE
endif

ifeq ($(AC_PJMEDIA_RESAMPLE),speex)
export CFLAGS += -DPJMEDIA_RESAMPLE_IMP=PJMEDIA_RESAMPLE_SPEEX
endif

#
# PortAudio
#
ifeq (0,1)
# External PA
export CFLAGS += -DPJMEDIA_AUDIO_DEV_HAS_PORTAUDIO=1
endif

#
# libyuv
#
ifeq (,1)
export CFLAGS += -DPJMEDIA_HAS_LIBYUV=0
else
export CFLAGS += -DPJMEDIA_HAS_LIBYUV=1
ifeq (0,0)
export CFLAGS += -I$(THIRD_PARTY)/yuv/include
endif
endif

#
# libwebrtc
#
ifeq (,1)
export CFLAGS += -DPJMEDIA_HAS_WEBRTC_AEC=0
else
export CFLAGS += -DPJMEDIA_HAS_WEBRTC_AEC=1
ifneq ($(findstring arm,$(sse2)),)
export CFLAGS += -DPJMEDIA_WEBRTC_AEC_USE_MOBILE=1
endif

ifeq (0,0)
export CFLAGS += -I$(THIRD_PARTY)/webrtc/src
endif
endif


#
# MacOSX specific
#
ifneq ($(findstring coreaudio,$(AC_PJMEDIA_SND)),)
export CFLAGS += -DPJMEDIA_AUDIO_DEV_HAS_COREAUDIO=1 \
		 -DPJMEDIA_AUDIO_DEV_HAS_PORTAUDIO=0 \
		 -DPJMEDIA_AUDIO_DEV_HAS_WMME=0
endif

#
# Unix specific
#
ifneq ($(findstring alsa,$(AC_PJMEDIA_SND)),)
export CFLAGS += -DPJMEDIA_AUDIO_DEV_HAS_ALSA=1 \
		 -DPJMEDIA_AUDIO_DEV_HAS_PORTAUDIO=0 \
		 -DPJMEDIA_AUDIO_DEV_HAS_WMME=0
endif

#
# Windows specific
#
ifneq ($(findstring win32,$(AC_PJMEDIA_SND)),)
export CFLAGS += -DPJMEDIA_AUDIO_DEV_HAS_WMME=1 \
		 -DPJMEDIA_AUDIO_DEV_HAS_PORTAUDIO=0
else
export CFLAGS += -DPJMEDIA_AUDIO_DEV_HAS_WMME=0
endif

#
# Null sound device
#
ifeq ($(AC_PJMEDIA_SND),null)
export CFLAGS += -DPJMEDIA_AUDIO_DEV_HAS_PORTAUDIO=0 \
		 -DPJMEDIA_AUDIO_DEV_HAS_WMME=0
endif

#
# External sound device
#
ifeq ($(AC_PJMEDIA_SND),external)
export CFLAGS += -DPJMEDIA_AUDIO_DEV_HAS_PORTAUDIO=0 \
		 -DPJMEDIA_AUDIO_DEV_HAS_WMME=0
endif

#
# QT video device
#
ifeq ($(AC_PJMEDIA_VIDEO_HAS_QT),yes)
export PJMEDIA_VIDEODEV_OBJS += qt_dev.o
endif

#
# Darwin video device
#
ifeq ($(AC_PJMEDIA_VIDEO_HAS_DARWIN),yes)
export PJMEDIA_VIDEODEV_OBJS += darwin_dev.o
endif

#
# VideoToolbox codec
#
ifeq ($(AC_PJMEDIA_VIDEO_HAS_VTOOLBOX),yes)
export CODEC_OBJS += vid_toolbox.o
endif

#
# iOS OpenGL video device
#
ifeq ($(AC_PJMEDIA_VIDEO_HAS_IOS_OPENGL),yes)
export PJMEDIA_VIDEODEV_OBJS += ios_opengl_dev.o
endif

#
# Android video device
#
ifeq ($(AC_PJMEDIA_VIDEO),android_os)
export PJMEDIA_VIDEODEV_OBJS += android_dev.o android_opengl.o
endif

#
# Determine whether we should compile the obj-c version of a particular
# source code
#
ifneq (,$(filter $(AC_PJMEDIA_VIDEO),darwin_os))
# Mac and iPhone OS specific, use obj-c
export PJMEDIA_VIDEODEV_OBJS += sdl_dev_m.o
else
# Other platforms, compile .c
export PJMEDIA_VIDEODEV_OBJS += sdl_dev.o
endif
