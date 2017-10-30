
ifneq (1,1)
ifeq (0,1)
# External
else
DIRS += gsm
endif
endif

ifneq (,1)
DIRS += ilbc
endif

# Exclude Speex?
EXCLUDE_SPEEX = 0
ifeq (,1)
ifneq (,)
EXCLUDE_SPEEX = 1
endif
endif

ifneq ($(EXCLUDE_SPEEX),1)
ifeq (0,1)
# External speex
else
DIRS += speex
endif
endif

ifneq (,1)
DIRS += g7221
endif

ifneq ($(findstring pa,alsa),)
ifeq (0,1)
# External PA
else
#DIRS += portaudio
endif
endif

ifneq (0,0)
# External SRTP
else
DIRS += srtp

ifeq (1,0)
CIPHERS_SRC = crypto/cipher/aes.o crypto/cipher/aes_icm.o       \
              # crypto/cipher/aes_cbc.o
HASHES_SRC  = crypto/hash/sha1.o crypto/hash/hmac.o 		\
	      # crypto/hash/tmmhv2.o
RNG_SRC     = # crypto/rng/rand_source.o crypto/rng/prng.o 	\
	      # crypto/rng/ctr_prng.o
else
CIPHERS_SRC = crypto/cipher/aes_icm_ossl.o crypto/cipher/aes_gcm_ossl.o
HASHES_SRC  = crypto/hash/hmac_ossl.o
RNG_SRC     = # crypto/rng/rand_source_ossl.o
SRTP_OTHER_CFLAGS = -DOPENSSL
endif


endif

ifeq (libresample,libresample)
DIRS += resample
endif

ifneq (,1)
ifeq (0,1)
# External yuv
else
DIRS += yuv
endif
endif

ifneq (,1)
ifeq (0,1)
# External webrtc
else
DIRS += webrtc
WEBRTC_OTHER_CFLAGS = -fexceptions -DWEBRTC_POSIX=1 
ifneq ($(findstring sse2,sse2),)
    WEBRTC_SRC = \
    	      modules/audio_processing/aec/aec_core_sse2.o		 \
	      modules/audio_processing/aec/aec_rdft_sse2.o	         \
	      modules/audio_processing/aecm/aecm_core_c.o	         \
	      modules/audio_processing/ns/nsx_core_c.o	                 \
	      system_wrappers/source/cpu_features.o
else ifneq ($(findstring neon,sse2),)
    WEBRTC_SRC = \
       	      modules/audio_processing/aec/aec_core_neon.o               \
	      modules/audio_processing/aec/aec_rdft_neon.o               \
	      modules/audio_processing/aecm/aecm_core_c.o                \
	      modules/audio_processing/aecm/aecm_core_neon.o             \
	      modules/audio_processing/ns/nsx_core_c.o                   \
	      modules/audio_processing/ns/nsx_core_neon.o                \
	      common_audio/signal_processing/cross_correlation_neon.o    \
	      common_audio/signal_processing/downsample_fast_neon.o      \
	      common_audio/signal_processing/min_max_operations_neon.o
    WEBRTC_OTHER_CFLAGS += -DWEBRTC_HAS_NEON
else ifneq ($(findstring mips,sse2),)
    WEBRTC_SRC = \
              modules/audio_processing/aec/aec_core_mips.o               \
	      modules/audio_processing/aec/aec_rdft_mips.o               \
	      modules/audio_processing/aecm/aecm_core_mips.o             \
	      modules/audio_processing/ns/nsx_core_mips.o                \
	      common_audio/signal_processing/cross_correlation_mips.o    \
	      common_audio/signal_processing/downsample_fast_mips.o      \
	      common_audio/signal_processing/min_max_operations_mips.o

    WEBRTC_OTHER_CFLAGS += -DMIPS_FPU_LE
else # Generic fixed point
    WEBRTC_SRC = \
	      modules/audio_processing/aecm/aecm_core_c.o                \
	      modules/audio_processing/ns/nsx_core_c.o                   \
	      common_audio/signal_processing/complex_fft.o
endif
endif
endif
