#include "audio_decoder.h"
#define WAVE_FORMAT_PCM 0x0001			// PCM
#define WAVE_FORMAT_IEEE_FLOAT 0x0003	// IEEE float
#define WAVE_FORMAT_ALAW 0x0006			// 8 - bit ITU - T G.711 A - law
#define WAVE_FORMAT_MULAW 0x0007		// 8 - bit ITU - T G.711 µ - law
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE	// Determined by SubFormat



typedef enum ChunkID
{
	FMT = 'f' | 'm' << 8 | 't' << 16 | ' ' << 24,
	FACT = 'f' | 'a' << 8 | 'c' << 16 | 't' << 24,
	DATA = 'd' | 'a' << 8 | 't' << 16 | 'a' << 24
};

typedef struct
{
	byte riff_ID[4];			// 'R','I','F','F'
	int32 riff_size;
	byte riff_format[4];		// 'W','A','V','E'
} RIFFHeader;

typedef struct  
{
	byte ID[4];
	uint32 data_size;
}JUNKChunk;

typedef struct
{
	uint16 format_tag;			/* 格式类型 */
	uint16 channels;			/* 通道数 */
	uint32 samples_per_sec;		/* 采样率 */
	uint32 bytes_per_sec;		/* 字节率 */
	uint16 block_align;			/* 数据块大小 */
	uint16 bits_pre_sample;		/* 位深 */
} WAVEFormat;

typedef struct
{
	byte data_ID[4];			// 'd','a','t','a'
	uint32 data_size;
} DataBlock;

typedef struct  
{
	byte list_ID[4];
	uint32 data_size;
}ListBlock;

typedef struct
{
	byte fact_ID[4];			// 'f','a','c','t'
	uint32 fact_size;
} FactBlock;

typedef struct
{
	byte fmt_ID[4];				// 'f','m','t',' '
	uint32 fmt_size;
	WAVEFormat wav_format;
} FormatBlock;

typedef struct  
{
	byte ckiD[4];
	uint32 ckSize;
}ChunkHead;

static PiBool _decode_head(char* tempBuffer, int64 _size, AudioInfo* audioInfo)
{
	RIFFHeader riff_header;
	int64 size = _size;
	PiBool has_fact_chunk = FALSE;
	uint32 bytes_per_sample = 0;
	uint chuck_head_size = sizeof(ChunkHead);
	WAVEFormat* waveFormat = NULL;
	uint32 head_size = 0;
	if (size < sizeof(RIFFHeader))
	{
		return FALSE;
	}
	pi_memcpy(&riff_header, tempBuffer, sizeof(RIFFHeader));
	if (riff_header.riff_ID[0] != 'R' ||
		riff_header.riff_ID[1] != 'I' ||
		riff_header.riff_ID[2] != 'F' ||
		riff_header.riff_ID[3] != 'F' ||
		riff_header.riff_format[0] != 'W' ||
		riff_header.riff_format[1] != 'A' ||
		riff_header.riff_format[2] != 'V' ||
		riff_header.riff_format[3] != 'E')
	{
		return FALSE;
	}
	tempBuffer += sizeof(RIFFHeader);
	size -= sizeof(RIFFHeader);
	head_size += sizeof(RIFFHeader);
	while (size >= chuck_head_size)
	{
		ChunkHead* chunk_head = (ChunkHead*)tempBuffer;
		uint32 id = *((uint32*)chunk_head->ckiD);
		if (size < chuck_head_size + chunk_head->ckSize)
		{
			return FALSE;
		}
		switch (id)
		{
		case FMT:
		{
			waveFormat = (WAVEFormat*)(tempBuffer + chuck_head_size);
			if (chunk_head->ckSize != 16 &&
				chunk_head->ckSize != 18 &&
				chunk_head->ckSize != 40)
			{
				return FALSE;
			}
			
			if (waveFormat->format_tag != WAVE_FORMAT_PCM &&
				waveFormat->format_tag != WAVE_FORMAT_IEEE_FLOAT &&
				waveFormat->format_tag != WAVE_FORMAT_ALAW &&
				waveFormat->format_tag != WAVE_FORMAT_MULAW &&
				waveFormat->format_tag != WAVE_FORMAT_EXTENSIBLE)
			{
				return FALSE;
			}
			
			if (waveFormat->format_tag != WAVE_FORMAT_PCM)
			{
				if (chunk_head->ckSize != 18 &&
					chunk_head->ckSize != 40)
				{
					return FALSE;
				}
			}

			bytes_per_sample = waveFormat->bytes_per_sec / (waveFormat->channels * waveFormat->samples_per_sec);
			if (bytes_per_sample != (uint32)(waveFormat->block_align / waveFormat->channels) ||
				bytes_per_sample != (uint32)(waveFormat->bits_pre_sample / 8))
			{
				return FALSE;
			}
			audioInfo->sampling_rate = waveFormat->samples_per_sec;
			if (waveFormat->channels == 1)
			{
				audioInfo->channel_format = CF_MONO;
			}
			else if (waveFormat->channels == 2)
			{
				audioInfo->channel_format = CF_STEREO;
			}
			else
			{
				return FALSE;
			}
			switch (waveFormat->format_tag)
			{
			case WAVE_FORMAT_PCM:
				if (bytes_per_sample == 1)
				{
					audioInfo->data_format = SF_UNSIGNED_BYTE;
				}
				else if (bytes_per_sample == 2)
				{
					audioInfo->data_format = SF_SHORT;
				}
				else
				{
					return FALSE;
				}
				break;
			case WAVE_FORMAT_IEEE_FLOAT:
				if (bytes_per_sample == 4)
				{
					audioInfo->data_format = SF_FLOAT32;
				}
				else if (bytes_per_sample == 8)
				{
					audioInfo->data_format = SF_FLOAT64;
				}
				else{
					return FALSE;
				}
				break;
			case WAVE_FORMAT_ALAW:
				if (bytes_per_sample == 1)
				{
					audioInfo->data_format = SF_ALAW;
				}
				else
				{
					return FALSE;
				}
				break;
			case WAVE_FORMAT_MULAW:
				if (bytes_per_sample == 1)
				{
					audioInfo->data_format = SF_MULAW;
				}
				else
				{
					return FALSE;
				}
				break;
			default:
				return FALSE;
				break;
			}
		}
		break;
		case FACT:
		{
			has_fact_chunk = TRUE;
		}
		break;
		case DATA:
		{
			audioInfo->data_size = chunk_head->ckSize;
			audioInfo->header_size = head_size + chuck_head_size;
		}
		break;
		default:
			break;
		}
		size -= chuck_head_size + chunk_head->ckSize;
		head_size += chuck_head_size + chunk_head->ckSize;
		tempBuffer += chuck_head_size + chunk_head->ckSize;
	}

	if (waveFormat == NULL)
	{
		return FALSE;
	}
	if (waveFormat->format_tag != WAVE_FORMAT_PCM && !has_fact_chunk)
	{
		return FALSE;
	}
	return TRUE;
}

AudioDecodeResult PI_API pi_audio_decode_wav(const wchar* path, AudioWaveData** out_data)
{
	void* file = pi_vfs_file_open(path, FILE_OPEN_READ);
	if (file == NULL)
	{
		return ADR_FILE_NOT_EXIST;
	}
	int64 size;
	AudioWaveData* waveData = NULL;
	AudioInfo audioInfo;
	int64 require_size;
	char* fileBuffer;
	int64 bytes_copy = 0;
	pi_vfs_file_size(file, &size);
	fileBuffer = pi_malloc((uint32)size);
	pi_vfs_file_read(file, 0, FALSE, fileBuffer, (uint)size);

	if (_decode_head(fileBuffer, size, &audioInfo))
	{
		waveData = pi_audio_decode_data_create();
		pi_memcpy(&waveData->audioInfo, &audioInfo, sizeof(AudioInfo));

		require_size = audioInfo.data_size;
		while (require_size > 0){
			WaveDataBlock block;
			byte* buffer;
			int64 block_size;
			block_size = pi_audio_decode_create_buffer(require_size, &buffer);
			block.data = buffer;
			block.size = block_size;
			pi_dvector_push(&waveData->blocks, &block);
			pi_memcpy_inline(buffer, fileBuffer + audioInfo.header_size + bytes_copy, (uint)block_size);
			require_size -= block_size;
			bytes_copy += block_size;
		}
	}
	else
	{
		return ADR_FORMAT_ERROR;
	}
	pi_free(fileBuffer);
	pi_vfs_file_close(file);
	*out_data = waveData;
	return ADR_OK;
}