
#ifndef INCLUDE_AUDIO_DECODER_H
#define INCLUDE_AUDIO_DECODER_H

#include <pi_lib.h>

#define MAX_BUFFER_SIZE 0xA00000
typedef enum
{
	DR_OK,						/* 解码成功 */
	DR_DONE,					/* 解码完成 */
	DR_ERROR,					/* 失败 */
} DecodeResult;					/* 解码操作返回值 */

typedef enum
{
	AF_MP3,								/* mp3音频格式 */
	AF_OGG,								/* ogg音频格式 */
	AF_OPUS,							/* opus音频格式 */
	AF_WAV								/* wav音频格式 */
} AudioFormat;							/* 音频格式 */

typedef enum
{
	SF_MONO8,
	SF_MONO16,
	SF_STEREO8,
	SF_STEREO16
}StreamFormat;

typedef enum
{
	SF_UNSIGNED_BYTE,			/* unsigned char, 8bit pcm */
	SF_SHORT,					/* short, 16bit pcm */
	SF_FLOAT32,					/* float, 32bit pcm */
	SF_FLOAT64,					/* double float, 64bit pcm */
	SF_ALAW,					/* ITU-T G.711 A-law, 8bit log-PCM */
	SF_MULAW					/* ITU-T G.711 µ-law, 8bit log-PCM */
} DataFormat;					/* 数据类型 */

typedef enum
{
	CF_MONO,					/* 单声道 */
	CF_STEREO					/* 立体声声道 */
} ChannelFormat;				/* 声道类型 */

typedef struct
{
	DataFormat data_format;			/* 数据类型 */
	ChannelFormat channel_format;	/* 声道类型 */
	uint sampling_rate;				/* 采样率 */
	uint samples;					/* 总采样次数 */
	int64 header_size;
	int64 data_size;
} AudioInfo;

typedef struct  
{
	int64 size;
	byte* data;
}WaveDataBlock;

typedef struct
{
	int channel;
	int blockAlign;
	int bytesPerSecond;
	double duration;
	PiDvector blocks;
	int buffer_count;
	AudioInfo audioInfo;
}AudioWaveData;

typedef enum
{
	ADR_OK,
	ADR_FILE_NOT_EXIST,
	ADR_FORMAT_ERROR
}AudioDecodeResult;

PI_BEGIN_DECLS

AudioDecodeResult PI_API pi_audio_decode(const wchar* path, AudioFormat format, AudioWaveData** data);

AudioDecodeResult PI_API pi_audio_decode_ogg(const wchar* path, AudioWaveData** out_data);

AudioDecodeResult PI_API pi_audio_decode_mp3(const wchar* path, AudioWaveData** out_data);

AudioDecodeResult PI_API pi_audio_decode_opus(const wchar* path, AudioWaveData** out_data);

AudioDecodeResult PI_API pi_audio_decode_wav(const wchar* path, AudioWaveData** out_data);

int64 PI_API pi_audio_decode_create_buffer(int64 size, byte** buffer);

AudioWaveData* PI_API pi_audio_decode_data_create();

void PI_API pi_audio_wave_data_free(AudioWaveData* data);
PI_END_DECLS

#endif /* INCLUDE_AUDIO_DECODER_H */
