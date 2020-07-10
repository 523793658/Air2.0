#include "audio_decoder.h"
#include "libvorbis/include/vorbis/vorbisfile.h"
AudioDecodeResult PI_API pi_audio_decode_ogg(const wchar* path, AudioWaveData** out_data)
{
	AudioWaveData* data;
	OggVorbis_File oggFile;
	memset(&oggFile, 0, sizeof(OggVorbis_File));
	char tempBuffer[4096];
	wchar realPath[256];
	char* realPatha;
	int64 require_size;
	wchar work_path[NAME_LENGTH];
	pi_file_get_cwd(work_path, NAME_LENGTH);
	pi_vfs_get_real_path(realPath, path);
	realPatha = pi_wstr_to_str(realPath + pi_wstrlen(work_path) + 1, PI_CP_UTF8);
	if (ov_fopen(realPatha, &oggFile) != 0)
	{
		return ADR_FILE_NOT_EXIST;
	}
	pi_free(realPatha);
	data = pi_audio_decode_data_create();
	vorbis_info *oggInfo = ov_info(&oggFile, -1);
	ogg_int64_t samples = ov_pcm_total(&oggFile, -1);
	data->duration = ov_time_total(&oggFile, -1);
	data->audioInfo.data_size = samples * 2 * oggInfo->channels;
	data->channel = oggInfo->channels;
	data->audioInfo.sampling_rate = oggInfo->rate;
	data->blockAlign = data->audioInfo.sampling_rate * oggInfo->channels / 8;
	data->bytesPerSecond = data->audioInfo.sampling_rate * data->blockAlign;
	require_size = data->audioInfo.data_size;
	data->audioInfo.data_format = SF_SHORT;
	switch (data->channel)
	{
	case 1:
		data->audioInfo.channel_format = CF_MONO;
		break;
	case 2:
		data->audioInfo.channel_format = CF_STEREO;
		break;
	default:
		break;
	}
	int current_section;
	WaveDataBlock block;
	byte* buffer;
	int64 avalible_size = pi_audio_decode_create_buffer((uint)require_size, &buffer);
	block.data = buffer;
	block.size = 0;
	require_size -= avalible_size;
	int eof = FALSE;
	while (!eof)
	{
		long ret = ov_read(&oggFile, tempBuffer, sizeof(tempBuffer), 0, 2, 1, &current_section);
		char* srcBuffer = tempBuffer;
		if (ret == 0){
			eof = TRUE;
		}
		else if (ret < 0)
		{
			pi_log_print(LOG_WARNING, "audio data error! path is %ls", path);
			pi_audio_wave_data_free(data);

			return ADR_FORMAT_ERROR;
		}
		else{
			while (ret > avalible_size){
				pi_memcpy(buffer, srcBuffer, (uint)avalible_size);
				block.size += avalible_size;
				pi_dvector_push(&data->blocks, &block);



				srcBuffer += avalible_size;
				ret -= (long)avalible_size;

				avalible_size = pi_audio_decode_create_buffer(require_size, &buffer);
				require_size -= avalible_size;
				block.data = buffer;
				block.size = 0;
			}
			pi_memcpy(buffer, srcBuffer, ret);
			buffer += ret;
			block.size += ret;
			avalible_size -= ret;
		}
	}
	pi_dvector_push(&data->blocks, &block);
	ov_clear(&oggFile);
	*out_data = data;
	return ADR_OK;
}