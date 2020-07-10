#include "audio_decoder.h"

void PI_API pi_audio_wave_data_free(AudioWaveData* data)
{
	uint size = pi_dvector_size(&data->blocks);
	uint i;
	for (i = 0; i < size; i++)
	{
		WaveDataBlock* block = pi_dvector_get(&data->blocks, i);
		pi_free(block->data);
	}
	pi_dvector_clear(&data->blocks, TRUE);
	pi_free(data);
}

AudioDecodeResult PI_API pi_audio_decode(const wchar* path, AudioFormat format, AudioWaveData** out_data)
{

	AudioDecodeResult result = ADR_FORMAT_ERROR;
	switch (format)
	{
	case AF_MP3:
		result= pi_audio_decode_mp3(path, out_data);
		break;
	case AF_WAV:
		result = pi_audio_decode_wav(path, out_data);
		break;
	case AF_OGG:
		result = pi_audio_decode_ogg(path, out_data);
		break;
	case AF_OPUS:
		result = pi_audio_decode_opus(path, out_data);
		break;
	default:
		break;
	}
	return result;
}

int64 PI_API pi_audio_decode_create_buffer(int64 size, byte** buffer)
{
	int64 block_size = min(size, MAX_BUFFER_SIZE);
	*buffer = pi_malloc((uint)block_size);
	return block_size;
}

AudioWaveData* PI_API pi_audio_decode_data_create()
{
	AudioWaveData* data = pi_new0(AudioWaveData, 1);
	pi_dvector_init(&data->blocks, sizeof(WaveDataBlock));
	return data;
}