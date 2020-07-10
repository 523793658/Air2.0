#include "audio_decoder.h"
#include "mpg123/mpg123.h"
#define MAX_FEED_SIZE 65536			/* 每次最多读取64kB数据 */



static PiBool is_mpg123_inited = FALSE;

typedef struct
{
	int channels;
	uint sampler_size;
}MPG123Info;


static PiBool _decode(mpg123_handle* mh, AudioInfo* info, MPG123Info* mpgInfo, AudioWaveData* data)
{
	int64 require_size = mpg123_length(mh) * mpgInfo->sampler_size * mpgInfo->channels / 2;
	int eof = FALSE;
	byte tempBuffer[4096];
	size_t done;
	byte* buffer;
	WaveDataBlock block;
	int64 avalible_size = pi_audio_decode_create_buffer((uint)require_size, &buffer);
	block.data = buffer;
	block.size = 0;
	require_size -= avalible_size;
	while (!eof)
	{
		int err = mpg123_read(mh, tempBuffer, sizeof(tempBuffer), &done);
		if (err == MPG123_ERR)
		{
			pi_free(block.data);
			return FALSE;
		}

		byte* srcBuffer = tempBuffer;
		if (err == MPG123_DONE){
			eof = TRUE;
		}
		if (done > 0)
		{
			while (done > avalible_size){
				pi_memcpy(buffer, srcBuffer, (uint)avalible_size);
				block.size += avalible_size;
				pi_dvector_push(&data->blocks, &block);


				srcBuffer += avalible_size;
				done -= (long)avalible_size;

				avalible_size = pi_audio_decode_create_buffer(require_size, &buffer);
				require_size -= avalible_size;
				block.data = buffer;
				block.size = 0;
			}
			pi_memcpy(buffer, srcBuffer, done);
			buffer += done;
			avalible_size -= done;
			block.size += done;
		}
	}
	pi_dvector_push(&data->blocks, &block);
	return TRUE;
}

static PiBool _decode_header(mpg123_handle* mh, AudioInfo* info, MPG123Info *mpgInfo)
{
	long rate;
	int channels, enc;
	mpg123_getformat(mh, &rate, &channels, &enc);
	info->sampling_rate = (uint)rate;
	if (channels == MPG123_MONO)
	{
		info->channel_format = CF_MONO;
	}
	else if (channels == MPG123_STEREO)
	{
		info->channel_format = CF_STEREO;
	}
	else
	{
		return FALSE;
	}
	switch (enc)
	{
	case MPG123_ENC_UNSIGNED_8:
		info->data_format = SF_UNSIGNED_BYTE;
		mpgInfo->sampler_size = 8;
		break;
	case MPG123_ENC_SIGNED_16:
		info->data_format = SF_SHORT;
		mpgInfo->sampler_size = 16;
		break;
	case MPG123_ENC_FLOAT_32:
		info->data_format = SF_FLOAT32;
		mpgInfo->sampler_size = 32;
		break;
	case MPG123_ENC_FLOAT_64:
		info->data_format = SF_FLOAT64;
		mpgInfo->sampler_size = 64;
		break;
	case MPG123_ENC_ALAW_8:
		info->data_format = SF_ALAW;
		mpgInfo->sampler_size = 8;
		break;
	case MPG123_ENC_ULAW_8:
		info->data_format = SF_MULAW;
		mpgInfo->sampler_size = 8;
		break;
	default:
		return FALSE;
	}
	mpgInfo->channels = channels;
	return TRUE;
}

AudioDecodeResult PI_API pi_audio_decode_mp3(const wchar* path, AudioWaveData** out_data)
{
	mpg123_handle *mh;
	AudioWaveData* waveData = NULL;
	AudioInfo info;
	MPG123Info mpgInfo;
	wchar realPath[256];
	char* realPatha;
	pi_vfs_get_real_path(realPath, path);
	realPatha = pi_wstr_to_str(realPath, PI_CP_UTF8);
	int err;
	if (!is_mpg123_inited)
	{
		err = mpg123_init();
	}
	mh = mpg123_new(NULL, &err);
	if (mh == NULL)
	{
		return ADR_FORMAT_ERROR;
	}
	err = mpg123_open(mh, realPatha);
	if (err == MPG123_ERR)
	{
		return ADR_FILE_NOT_EXIST;
	}
	pi_free(realPatha);
	if (_decode_header(mh, &info, &mpgInfo))
	{
		waveData = pi_audio_decode_data_create();
		pi_memcpy(&waveData->audioInfo, &info, sizeof(AudioInfo));
		_decode(mh, &info, &mpgInfo, waveData);
	}
	else
	{
		return ADR_FORMAT_ERROR;
	}
	mpg123_close(mh);
	mpg123_delete(mh);
	*out_data = waveData;
	return ADR_OK;
}