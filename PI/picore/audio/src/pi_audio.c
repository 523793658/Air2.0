
#include "pi_audio.h"

#define AL_LIBTYPE_STATIC
#include "openal/al.h"
#include "openal/alc.h"
#include "openal/alext.h"
#undef AL_LIBTYPE_STATIC

#include "mp3_decoder.h"
#include "ogg_decoder.h"
#include "opus_decoder.h"
#include "wav_decoder.h"
static ALenum map[6][2] =
{
	{ AL_FORMAT_MONO8, AL_FORMAT_STEREO8 },
	{ AL_FORMAT_MONO16, AL_FORMAT_STEREO16 },
	{ AL_FORMAT_MONO_FLOAT32, AL_FORMAT_STEREO_FLOAT32 },
	{ AL_FORMAT_MONO_DOUBLE_EXT, AL_FORMAT_STEREO_DOUBLE_EXT },
	{ AL_FORMAT_MONO_ALAW_EXT, AL_FORMAT_STEREO_ALAW_EXT },
	{ AL_FORMAT_MONO_MULAW_EXT, AL_FORMAT_STEREO_MULAW_EXT }
};

#define MAX_BUFFER_COUNT 10

struct PiAudio
{
	PiBool loop;						/* 是否循环 */
	uint buffer_count;					/*缓冲数量*/
	ALuint source;						/* openal声音源 */
	ALuint buffers[MAX_BUFFER_COUNT];	/* openal声音存储数据 */
	ALenum format;
	AudioWaveData* data;
};



typedef struct
{
	ALCdevice *device;					/* 音频数据块 */
	ALCcontext *context;				/* 音频数据块 */
} AudioSystem;

static AudioSystem *g_audio_system = NULL;

AudioSystemInitRet PI_API pi_audio_context_init()
{
	if (!g_audio_system)
	{
		g_audio_system = pi_new0(AudioSystem, 1);

		g_audio_system->device = alcOpenDevice(NULL);
		if (!g_audio_system->device)
		{
			pi_free(g_audio_system);
			g_audio_system = NULL;
			return ASIR_DEVICE_FAILED;
		}

		g_audio_system->context = alcCreateContext(g_audio_system->device, NULL);
		if (!g_audio_system->context)
		{
			alcCloseDevice(g_audio_system->device);
			pi_free(g_audio_system);
			g_audio_system = NULL;
			return ASIR_CONTEXT_FAILED;
		}

		alcMakeContextCurrent(g_audio_system->context);
	}

	return ASIR_OK;
}

void PI_API pi_audio_context_free()
{
	if (g_audio_system)
	{
		alcMakeContextCurrent(NULL);
		if (g_audio_system->context)
		{
			alcDestroyContext(g_audio_system->context);
		}
		if (g_audio_system->device)
		{
			alcCloseDevice(g_audio_system->device);
		}
		pi_free(g_audio_system);
	}
}

ALenum _format_convert(AudioInfo* info)
{
	
	return map[info->data_format][info->channel_format];
}

PiAudio *PI_API pi_audio_create()
{
	PiAudio *audio = NULL;
	audio = pi_new0(PiAudio, 1);
	alGenSources(1, &audio->source);
	return audio;
}

PiBool PI_API pi_audio_delete(PiAudio *audio, PiBool is_free)
{
	if (!audio)
	{
		return FALSE;
	}
	alDeleteSources(1, &audio->source);
	alDeleteBuffers(audio->buffer_count, audio->buffers);
	pi_free(audio);
	return TRUE;
}

PiBool PI_API pi_audio_set_data(PiAudio *audio, AudioWaveData* data)
{
	uint i;
	if (audio == NULL)
	{
		return FALSE;
	}
	if (data == NULL){
		return FALSE;
	}
	audio->buffer_count = pi_dvector_size(&data->blocks);
	alGenBuffers(audio->buffer_count, audio->buffers);
	audio->format = _format_convert(&data->audioInfo);

	for (i = 0; i < audio->buffer_count; i++)
	{
		WaveDataBlock* block = pi_dvector_get(&data->blocks, i);
		alBufferData(audio->buffers[i], audio->format, block->data, (ALsizei)block->size, data->audioInfo.sampling_rate);
	}
	alSourceQueueBuffers(audio->source, audio->buffer_count, audio->buffers);

	audio->data = data;
	return TRUE;
}

PiBool PI_API pi_audio_play(PiAudio *audio)
{
	ALint state;
	if (!audio)
	{
		return FALSE;
	}
	if (audio->data == NULL){
		return FALSE;
	}

	alGetSourcei(audio->source, AL_SOURCE_STATE, &state);
	if (state != AL_INITIAL)
	{
		return FALSE;
	}
	alSourcePlay(audio->source);
	return TRUE;
}


double PI_API pi_audio_get_duration(PiAudio *audio)
{
	return audio->data->duration;
}

PiBool PI_API pi_audio_set_loop(PiAudio *audio, PiBool is_loop)
{
	if (audio == NULL)
	{
		return FALSE;
	}
	audio->loop = is_loop;
	alSourcei(audio->source, AL_LOOPING, is_loop);
	return TRUE;
}

PiBool PI_API pi_audio_get_loop(PiAudio *audio)
{
	return audio->loop;
}

PiBool PI_API pi_audio_set_volume(PiAudio *audio, float volume)
{
	if (audio == NULL)
	{
		return FALSE;
	}
	alSourcef(audio->source, AL_GAIN, volume);
	return TRUE;
}

float PI_API pi_audio_get_volume(PiAudio *audio)
{
	ALfloat volume;
	if (audio == NULL)
	{
		return FALSE;
	}
	alGetSourcef(audio->source, AL_GAIN, &volume);
	return volume;
}

PiBool PI_API pi_audio_set_position(PiAudio *audio, const PiVector3 *position)
{
	if (audio == NULL)
	{
		return FALSE;
	}
	alSource3f(audio->source, AL_POSITION, position->x, position->y, position->z);
	return TRUE;
}

PiBool PI_API pi_audio_get_position(PiAudio *audio, PiVector3 *position)
{
	if (audio == NULL)
	{
		return FALSE;
	}
	alGetSource3f(audio->source, AL_POSITION, &position->x, &position->y, &position->z);
	return TRUE;
}

PiBool PI_API pi_audio_set_distance_attenuation(PiAudio *audio, float min_distance, float max_distance, float rolloff_factor)
{
	if (audio == NULL)
	{
		return FALSE;
	}

	alSourcef(audio->source, AL_REFERENCE_DISTANCE, min_distance);
	alSourcef(audio->source, AL_MAX_DISTANCE, max_distance);
	alSourcef(audio->source, AL_ROLLOFF_FACTOR, rolloff_factor);

	return TRUE;
}

PiBool PI_API pi_audio_set_cone_attenuation(PiAudio *audio, float inner_angle, float outer_angle, float outer_volume)
{
	if (audio == NULL)
	{
		return FALSE;
	}

	alSourcef(audio->source, AL_CONE_INNER_ANGLE, inner_angle);
	alSourcef(audio->source, AL_CONE_OUTER_ANGLE, outer_angle);
	alSourcef(audio->source, AL_CONE_OUTER_GAIN, outer_volume);

	return TRUE;
}

PiBool PI_API pi_audio_set_direction(PiAudio *audio, const PiVector3 *direction)
{
	if (audio == NULL)
	{
		return FALSE;
	}

	alSource3f(audio->source, AL_DIRECTION, direction->x, direction->y, direction->z);

	return TRUE;
}

PiBool PI_API pi_audio_set_volume_range(PiAudio *audio, float min_volume, float max_volume)
{
	if (audio == NULL)
	{
		return FALSE;
	}

	alSourcef(audio->source, AL_MIN_GAIN, min_volume);
	alSourcef(audio->source, AL_MAX_GAIN, max_volume);

	return TRUE;
}

void PI_API pi_audio_set_listener_position(const PiVector3 *position)
{
	alListener3f(AL_POSITION, position->x, position->y, position->z);
}

void PI_API pi_audio_get_listener_position(PiVector3 *position)
{
	alGetListener3f(AL_POSITION, &position->x, &position->y, &position->z);
}

void PI_API pi_audio_set_listener_orientation(const PiVector3 *forward, const PiVector3 *up)
{
	ALfloat orientation[6];

	orientation[0] = forward->x;
	orientation[1] = forward->y;
	orientation[2] = forward->z;
	orientation[3] = up->x;
	orientation[4] = up->y;
	orientation[5] = up->z;

	alListenerfv(AL_ORIENTATION, orientation);
}

void PI_API pi_audio_get_listener_orientation(PiVector3 *forward, PiVector3 *up)
{
	ALfloat orientation[6];

	alGetListenerfv(AL_ORIENTATION, orientation);

	forward->x = orientation[0];
	forward->y = orientation[1];
	forward->z = orientation[2];
	up->x = orientation[3];
	up->y = orientation[4];
	up->z = orientation[5];
}

void PI_API pi_audio_set_distance_attenuation_model(DistanceAttenuationModel model)
{
	int distance_attenuation_model;

	switch (model)
	{
	case DAM_NONE:
		distance_attenuation_model = AL_NONE;
		break;
	case DAM_INVERSE_DISTANCE:
		distance_attenuation_model = AL_INVERSE_DISTANCE;
		break;
	case DAM_INVERSE_DISTANCE_CLAMPED:
		distance_attenuation_model = AL_INVERSE_DISTANCE_CLAMPED;
		break;
	case DAM_LINEAR_DISTANCE:
		distance_attenuation_model = AL_LINEAR_DISTANCE;
		break;
	case DAM_LINEAR_DISTANCE_CLAMPED:
		distance_attenuation_model = AL_LINEAR_DISTANCE_CLAMPED;
		break;
	case DAM_EXPONENT_DISTANCE:
		distance_attenuation_model = AL_EXPONENT_DISTANCE;
		break;
	case DAM_EXPONENT_DISTANCE_CLAMPED:
		distance_attenuation_model = AL_EXPONENT_DISTANCE_CLAMPED;
		break;
	default:
		return;
	}

	alDistanceModel(distance_attenuation_model);
}
