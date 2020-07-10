#include "audio_decoder.h"

#include "ogg/ogg.h"
#include "opus/opus.h"


#define MAX_FEED_SIZE 65536

typedef struct
{
	uint8 version;
	uint8 channels; /* Number of channels: 1..255 */
	uint16 preskip;
	uint32 input_sample_rate;
	int16 gain; /* in dB S7.8 should be zero whenever possible */
	uint8 channel_mapping;
	/* The rest is only used if channel_mapping > 0 */
	uint8 nb_streams;
	uint8 nb_coupled;
	uint8 stream_map[255];
} OpusHeader;

typedef struct
{
	char **user_comments;
	int *comment_lengths;
	int comments;
	char *vendor;
} OpusComment;

typedef struct
{
	PiBool            init;
	uint       block_align; /* 数据块对齐字节，通道数乘以每采样字节数，跟WAV格式的block_align含义相同 */
	PiBool  packet_pending;
	OpusHeader      header;
	OpusComment    comment;

	ogg_sync_state      oy; /* sync and verify incoming physical bitstream */
	ogg_stream_state    os; /* take physical pages, weld into a logical stream of packets */
	ogg_page            og; /* one Ogg bitstream page. opus packets are inside */
	ogg_packet          op; /* one raw packet of data for decode */

	OpusDecoder      *impl;

	uint avalible_size;
	uint size;
	byte* out_buffer;

	byte tempBuffer[MAX_FEED_SIZE];
	void* file;
	uint offset;
	AudioWaveData* data;
} PiOpusDecoder;

static uint16 _read_uint16(const byte *data)
{
	uint16 val;
	val = (uint16)data[0];
	val |= (uint16)data[1] << 8;

	return val;
}


static uint32 _read_uint32(const byte *data)
{
	uint32 val;
	val = (uint32)data[0];
	val |= (uint32)data[1] << 8;
	val |= (uint32)data[2] << 16;
	val |= (uint32)data[3] << 24;

	return val;
}

static PiBool _opus_header_parse(const byte *packet_data, int length, OpusHeader *header)
{
	unsigned char ch;
	uint16 shortval;
	const byte *data = packet_data;

	if (length < 19)
	{
		return FALSE;
	}

	if (pi_memcmp(data, "OpusHead", 8) != PI_COMP_EQUAL)
	{
		return FALSE;
	}
	data += 8;

	ch = data[0];
	data++;
	if ((ch & 240) != 0) /* Only major version 0 supported. */
	{
		return FALSE;
	}
	header->version = ch;

	ch = data[0];
	data++;
	if (ch == 0)
	{
		return FALSE;
	}
	header->channels = ch;

	shortval = _read_uint16(data);
	data += 2;
	header->preskip = shortval;

	header->input_sample_rate = _read_uint32(data);
	data += 4;

	shortval = _read_uint16(data);
	data += 2;
	header->gain = shortval;

	ch = data[0];
	data++;
	header->channel_mapping = ch;

	if (header->channel_mapping > 0)
	{
		int i;
		if (length < 21 + header->channels)
		{
			return FALSE;
		}

		ch = data[0];
		data++;
		if (ch < 1)
		{
			return FALSE;
		}
		header->nb_streams = ch;

		ch = data[0];
		data++;
		if (ch > header->nb_streams || (ch + header->nb_streams) > 255)
		{
			return FALSE;
		}
		header->nb_coupled = ch;

		/* Multi-stream support */
		for (i = 0; i < header->channels; i++)
		{
			header->stream_map[i] = data[0];
			data++;

			if (header->stream_map[i] >(header->nb_streams + header->nb_coupled) && header->stream_map[i] != 255)
			{
				return FALSE;
			}
		}
	}
	else
	{
		if (header->channels > 2)
		{
			return FALSE;
		}
		header->nb_streams = 1;
		header->nb_coupled = header->channels > 1;
		header->stream_map[0] = 0;
		header->stream_map[1] = 1;
	}
	/*For version 0/1 we know there won't be any more data
	so reject any that have data past the end.*/
	if ((header->version == 0 || header->version == 1) && packet_data + length != data)
	{
		return FALSE;
	}
	return TRUE;
}


static PiBool _feed(PiOpusDecoder *decoder)
{
	uint input_size = 0;

	input_size = (uint)pi_vfs_file_read(decoder->file, decoder->offset, FALSE, (char*)decoder->tempBuffer, MAX_FEED_SIZE);


	if (input_size == 0)
	{
		return FALSE;
	}
	else
	{
		char *buffer = ogg_sync_buffer(&decoder->oy, input_size);
		pi_memcpy(buffer, decoder->tempBuffer, input_size);
		ogg_sync_wrote(&decoder->oy, input_size);
		return TRUE;
	}
}

static void _opus_comments_clear(OpusComment *opus_comment)
{
	int i;
	if (opus_comment->vendor != NULL)
	{
		pi_free(opus_comment->vendor);
	}

	if (opus_comment->comment_lengths != NULL)
	{
		pi_free(opus_comment->comment_lengths);
	}

	for (i = 0; i < opus_comment->comments; i++)
	{
		if (opus_comment->user_comments[i] != NULL)
		{
			pi_free(opus_comment->user_comments[i]);
		}
	}

	pi_memset(opus_comment, 0, sizeof(opus_comment));
}

static PiBool _opus_comments_parse(const byte *packet_data, int length, OpusComment *opus_comment)
{
	const byte *data = packet_data;
	int strlen, i, nb_fields;

	if (length < 16)
	{
		return FALSE;
	}

	if (pi_memcmp(data, "OpusTags", 8) != PI_COMP_EQUAL)
	{
		return FALSE;
	}
	data += 8;
	length -= 8;

	// 读取vendor
	strlen = _read_uint32(data);
	data += 4;
	length -= 4;
	if (strlen < 0 || strlen > length)
	{
		return FALSE;
	}
	opus_comment->vendor = pi_new0(char, strlen + 1);
	pi_memcpy(opus_comment->vendor, data, strlen);
	data += strlen;
	length -= strlen;

	// 读取标签个数
	nb_fields = _read_uint32(data);
	data += 4;
	length -= 4;
	if (nb_fields < 0 || nb_fields >(length >> 2))
	{
		_opus_comments_clear(opus_comment);
		return FALSE;
	}
	opus_comment->comments = nb_fields;

	// 读取所有标签
	opus_comment->user_comments = pi_new0(char *, nb_fields);
	opus_comment->comment_lengths = pi_new0(int, nb_fields);

	for (i = 0; i < nb_fields; i++)
	{
		if (length < 4)
		{
			_opus_comments_clear(opus_comment);
			return FALSE;
		}
		strlen = _read_uint32(data);
		data += 4;
		length -= 4;
		if (strlen < 0 || strlen > length)
		{
			_opus_comments_clear(opus_comment);
			return FALSE;
		}
		opus_comment->user_comments[i] = pi_new0(char, strlen + 1);
		pi_memcpy(opus_comment->user_comments[i], data, strlen);
		opus_comment->comment_lengths[i] = strlen;

		data += strlen;
		length -= strlen;
	}

	return TRUE;
}

static PiBool _packet_decode(PiOpusDecoder *opus_decoder, DecodeResult *ret)
{
	int samples = opus_decoder_get_nb_samples(opus_decoder->impl, opus_decoder->op.packet, opus_decoder->op.bytes);

	if (samples > 0)
	{
		if (opus_decoder->avalible_size > samples * opus_decoder->block_align)
		{
			opus_int16 *pcm = (opus_int16 *)(opus_decoder->out_buffer + opus_decoder->size);
			int decoded_samples = opus_decode(opus_decoder->impl, opus_decoder->op.packet, opus_decoder->op.bytes, pcm, samples, 0);
			if (decoded_samples == samples)
			{
				uint decoded = decoded_samples * opus_decoder->block_align;
				opus_decoder->avalible_size -= decoded;
				opus_decoder->size += decoded;

				return TRUE;
			}

			*ret = DR_ERROR;
			return FALSE;
		}

		opus_decoder->packet_pending = TRUE;
		return FALSE;
	}

	return TRUE;
}

static PiBool _page_decode(PiOpusDecoder *opus_decoder, DecodeResult *ret)
{
	while (1)
	{
		int result = ogg_stream_packetout(&opus_decoder->os, &opus_decoder->op);

		if (result == 0)
		{
			break;
		}
		if (result > 0)
		{
			if (!_packet_decode(opus_decoder, ret))
			{
				return FALSE;
			}
		}
	}

	if (ogg_page_eos(&opus_decoder->og))
	{
		*ret = DR_DONE;
		return FALSE;
	}

	return TRUE;
}

AudioDecodeResult PI_API pi_audio_decode_opus(const wchar* path, AudioWaveData** out_data)
{

	PiOpusDecoder decoder;
	AudioWaveData* data = pi_audio_decode_data_create();
	decoder.data = data;
	AudioInfo info;
	decoder.avalible_size = 0;
	decoder.file = pi_vfs_file_open(path, FILE_OPEN_READ);
	if (decoder.file == NULL)
	{
		return ADR_FILE_NOT_EXIST;
	}
	decoder.offset = 0;

	ogg_sync_init(&decoder.oy);

	if (!_feed(&decoder))
	{
		return ADR_FORMAT_ERROR;
	}

	while (1)
	{
		int result = ogg_sync_pageout(&decoder.oy, &decoder.og);
		if (result > 0)
		{
			ogg_stream_init(&decoder.os, ogg_page_serialno(&decoder.og));
			if (ogg_stream_pagein(&decoder.os, &decoder.og) < 0)
			{
				return ADR_FORMAT_ERROR;
			}
			result = ogg_stream_packetout(&decoder.os, &decoder.op);
			if (result > 0)
			{
				if (!_opus_header_parse(decoder.op.packet, decoder.op.bytes, &decoder.header))
				{
					return ADR_FORMAT_ERROR;
				}
			}
			else
			{
				return ADR_FORMAT_ERROR;
			}
			break;
		}
		else if (result < 0)
		{
			return ADR_FORMAT_ERROR;
		}
		else
		{
			if (!_feed(&decoder))
			{
				return ADR_FORMAT_ERROR;
			}
		}
	}

	while (1)
	{
		int result = ogg_sync_pageout(&decoder.oy, &decoder.og);
		if (result > 0)
		{
			if (ogg_stream_pagein(&decoder.os, &decoder.og) < 0)
			{
				return ADR_FORMAT_ERROR;
			}
			result = ogg_stream_packetout(&decoder.os, &decoder.op);
			if (result > 0)
			{
				if (!_opus_comments_parse(decoder.op.packet, decoder.op.bytes, &decoder.comment))
				{
					return ADR_FORMAT_ERROR;
				}
				break;
			}
			else if (result < 0)
			{
				return ADR_FORMAT_ERROR;
			}
		}
		else if (result < 0)
		{
			return ADR_FORMAT_ERROR;
		}
		else
		{
			if (!_feed(&decoder))
			{
				return ADR_FORMAT_ERROR;
			}
		}
	}

	switch (decoder.header.channels)
	{
	case 1:
		info.channel_format = CF_MONO;
		break;
	case 2:
		info.channel_format = CF_STEREO;
		break;
	default:
		return ADR_FORMAT_ERROR;
	}
	info.data_format = SF_SHORT;
	info.sampling_rate = decoder.header.input_sample_rate;
	if (info.sampling_rate == 0)
	{
		info.sampling_rate = 48000;
	}
	if (info.sampling_rate < 8000 || info.sampling_rate > 192000)
	{
		info.sampling_rate = 48000;
	}
	decoder.block_align = sizeof(opus_int16)* decoder.header.channels;
	decoder.impl = opus_decoder_create(48000, decoder.header.channels, NULL);

	DecodeResult ret = DR_OK;

	WaveDataBlock block;
	decoder.avalible_size = (uint)pi_audio_decode_create_buffer(MAX_BUFFER_SIZE, &block.data);
	decoder.out_buffer = block.data;
	decoder.size = 0;
	block.size = 0;

	if (decoder.packet_pending)
	{
		if (!_packet_decode(&decoder, &ret))
		{
			return ADR_FORMAT_ERROR;
		}
		decoder.packet_pending = FALSE;
		if (!_page_decode(&decoder, &ret))
		{
			return ADR_FORMAT_ERROR;
		}
	}
	while (1)
	{
		while (1)
		{
			int result = ogg_sync_pageout(&decoder.oy, &decoder.og);
			if (result == 0)
			{
				break;
			}
			if (result > 0)
			{
				ogg_stream_pagein(&decoder.os, &decoder.og);
				if (!_page_decode(&decoder, &ret))
				{
					return ADR_FORMAT_ERROR;
				}
			}
		}
		if (decoder.avalible_size > 0)
		{
			if (!_feed(&decoder))
			{
				if (block.size > 0)
				{
					block.size = decoder.size;
					pi_dvector_push(&decoder.data->blocks, &block);
				}
				pi_memcpy(&data->audioInfo, &info, sizeof(AudioInfo));
				*out_data = data;
				return ADR_OK;
			}
		}
		else{
			block.size = decoder.size;
			pi_dvector_push(&decoder.data->blocks, &block);

			decoder.avalible_size = (uint)pi_audio_decode_create_buffer(MAX_BUFFER_SIZE, &block.data);
			decoder.out_buffer = block.data;
			decoder.size = 0;
			block.size = 0;
		}
	}
}