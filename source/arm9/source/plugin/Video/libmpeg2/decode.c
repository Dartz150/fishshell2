/*
 * decode.c
 * Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
 * Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
 *
 * This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
 * See http://libmpeg2.sourceforge.net/ for updates.
 *
 * mpeg2dec is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * mpeg2dec is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Modified for use with MPlayer, see libmpeg-0.4.0.diff for the exact changes.
 * detailed CVS changelog at http://www.mplayerhq.hu/cgi-bin/cvsweb.cgi/main/
 * $Id: decode.c,v 1.27 2005/02/19 02:32:12 diego Exp $
 */

#include "config.h"

#include <string.h>	/* memcmp/memset, try to remove */
#include <stdlib.h>
#include "inttypes.h"

#include "mpeg2.h"
#include "attributes.h"
#include "mpeg2_internal.h"

#define inline __inline__ __attribute__((always_inline))

static const int mpeg2_accels = 0;

//#define BUFFER_SIZE (1194 * 1024)
#define BUFFER_SIZE (128 * 1024)

const mpeg2_info_t * mpeg2_info (mpeg2dec_t * mpeg2dec)
{
    return &(mpeg2dec->info);
}

static inline int skip_chunk (mpeg2dec_t * mpeg2dec, int bytes)
{
    uint8_t * current;
    uint32_t shift;
    uint8_t * limit;
    uint8_t byte;

    if (!bytes)
	return 0;

    current = mpeg2dec->buf_start;
    shift = mpeg2dec->shift;
    limit = current + bytes;
    
    do {
	byte = *current++;
	if (shift == 0x00000100) {
	    int skipped;

	    mpeg2dec->shift = 0xffffff00;
	    skipped = current - mpeg2dec->buf_start;
	    mpeg2dec->buf_start = current;
	    return skipped;
	}
	shift = (shift | byte) << 8;
    } while (current < limit);

    mpeg2dec->shift = shift;
    mpeg2dec->buf_start = current;
    return 0;
}

static inline int copy_chunk (mpeg2dec_t * mpeg2dec, int bytes)
{
    uint8_t * current;
    uint32_t shift;
    uint8_t * chunk_ptr;
    uint8_t * limit;
    uint8_t byte;

    if (!bytes)
	return 0;

    current = mpeg2dec->buf_start;
    shift = mpeg2dec->shift;
    chunk_ptr = mpeg2dec->chunk_ptr;
    limit = current + bytes;

    do {
	byte = *current++;
	if (shift == 0x00000100) {
	    int copied;

	    mpeg2dec->shift = 0xffffff00;
	    mpeg2dec->chunk_ptr = chunk_ptr + 1;
	    copied = current - mpeg2dec->buf_start;
	    mpeg2dec->buf_start = current;
	    
/*
	    static int maxsize=0;
	    int chunksize=(int)mpeg2dec->chunk_ptr-(int)mpeg2dec->chunk_start;
	    if(maxsize<chunksize) maxsize=chunksize;
	    _consolePrintf("ChunkSize=%d,%d\n",maxsize,chunksize);
*/
	    return copied;
	}
	shift = (shift | byte) << 8;
	*chunk_ptr++ = byte;
    } while (current < limit);
    
    mpeg2dec->shift = shift;
    mpeg2dec->buf_start = current;
    return 0;
}

void mpeg2_buffer (mpeg2dec_t * mpeg2dec, uint8_t * start, uint8_t * end)
{
    mpeg2dec->buf_start = start;
    mpeg2dec->buf_end = end;
}

int mpeg2_getpos (mpeg2dec_t * mpeg2dec)
{
    return mpeg2dec->buf_end - mpeg2dec->buf_start;
}

static inline mpeg2_state_t seek_chunk (mpeg2dec_t * mpeg2dec)
{
    int size, skipped;

    size = mpeg2dec->buf_end - mpeg2dec->buf_start;
    skipped = skip_chunk (mpeg2dec, size);
    if (!skipped) {
	mpeg2dec->bytes_since_tag += size;
	return STATE_BUFFER;
    }
    mpeg2dec->bytes_since_tag += skipped;
    mpeg2dec->code = mpeg2dec->buf_start[-1];
    return (mpeg2_state_t)-1;
}

mpeg2_state_t mpeg2_seek_header (mpeg2dec_t * mpeg2dec)
{
    while (mpeg2dec->code != 0xb3 &&
	   ((mpeg2dec->code != 0xb7 && mpeg2dec->code != 0xb8 &&
	     mpeg2dec->code) || mpeg2dec->sequence.width == (unsigned)-1))
	if (seek_chunk (mpeg2dec) == STATE_BUFFER)
	    return STATE_BUFFER;
    mpeg2dec->chunk_start = mpeg2dec->chunk_ptr = mpeg2dec->chunk_buffer;
    mpeg2dec->user_data_len = 0;
    return (mpeg2dec->code ? mpeg2_parse_header (mpeg2dec) :
	    mpeg2_header_picture_start (mpeg2dec));
}

#define RECEIVED(code,state) (((state) << 8) + (code))

CODE_IN_ITCM_DPG mpeg2_state_t mpeg2_parse (mpeg2dec_t * mpeg2dec)
{
  int size_buffer, size_chunk, copied;

  if (mpeg2dec->action) {
    mpeg2_state_t state;

	  state = mpeg2dec->action (mpeg2dec);
	  if ((int)state >= 0) return state;
  }

  while (1) {
  	while ((unsigned) (mpeg2dec->code - mpeg2dec->first_decode_slice) < mpeg2dec->nb_decode_slices) {
      size_buffer = mpeg2dec->buf_end - mpeg2dec->buf_start;
	    size_chunk = (mpeg2dec->chunk_buffer + BUFFER_SIZE - mpeg2dec->chunk_ptr);
  	  if (size_buffer <= size_chunk) {
	  	  copied = copy_chunk (mpeg2dec, size_buffer);
		    if (!copied) {
  		    mpeg2dec->bytes_since_tag += size_buffer;
	  	    mpeg2dec->chunk_ptr += size_buffer;
		      return STATE_BUFFER;
  		  }
	      } else {
		    copied = copy_chunk (mpeg2dec, size_chunk);
		    if (!copied) {
  		    /* filled the chunk buffer without finding a start code */
	  	    mpeg2dec->bytes_since_tag += size_chunk;
		      mpeg2dec->action = seek_chunk;
		      return STATE_INVALID;
  		  }
	    }
	    mpeg2dec->bytes_since_tag += copied;

  	  if(mpeg2_slice_start ((mpeg2dec->decoder), mpeg2dec->code, mpeg2dec->chunk_start)==true){
  	    mpeg2_slice ((mpeg2dec->decoder), mpeg2dec->code, mpeg2dec->chunk_start);
  	  }
	    mpeg2dec->code = mpeg2dec->buf_start[-1];
	    mpeg2dec->chunk_ptr = mpeg2dec->chunk_start;
  	}
	  if ((unsigned) (mpeg2dec->code - 1) >= 0xb0 - 1) break;
  	if (seek_chunk (mpeg2dec) == STATE_BUFFER) return STATE_BUFFER;
  }

  switch (mpeg2dec->code) {
    case 0x00:
	    mpeg2dec->action = mpeg2_header_picture_start;
	    return mpeg2dec->state;
    case 0xb7:
	    mpeg2dec->action = mpeg2_header_end;
      break;
    case 0xb3:
    case 0xb8:
      mpeg2dec->action = mpeg2_parse_header;
	    break;
    default:
	    mpeg2dec->action = seek_chunk;
      return STATE_INVALID;
  }
  return (mpeg2dec->state == STATE_SLICE) ? STATE_SLICE : STATE_INVALID;
}

CODE_IN_ITCM_DPG mpeg2_state_t mpeg2_parse_header (mpeg2dec_t * mpeg2dec)
{
    static DATA_IN_MTCM_SET int (* process_header[]) (mpeg2dec_t * mpeg2dec) = {
	mpeg2_header_picture, mpeg2_header_extension, mpeg2_header_user_data,
	mpeg2_header_sequence, NULL, NULL, NULL, NULL, mpeg2_header_gop
    };
    int size_buffer, size_chunk, copied;

    mpeg2dec->action = mpeg2_parse_header;
    mpeg2dec->info.user_data = NULL;	mpeg2dec->info.user_data_len = 0;
    while (1) {
	size_buffer = mpeg2dec->buf_end - mpeg2dec->buf_start;
	size_chunk = (mpeg2dec->chunk_buffer + BUFFER_SIZE -
		      mpeg2dec->chunk_ptr);
	if (size_buffer <= size_chunk) {
	    copied = copy_chunk (mpeg2dec, size_buffer);
	    if (!copied) {
		mpeg2dec->bytes_since_tag += size_buffer;
		mpeg2dec->chunk_ptr += size_buffer;
		return STATE_BUFFER;
	    }
	} else {
	    copied = copy_chunk (mpeg2dec, size_chunk);
	    if (!copied) {
		/* filled the chunk buffer without finding a start code */
		mpeg2dec->bytes_since_tag += size_chunk;
		mpeg2dec->code = 0xb4;
		mpeg2dec->action = mpeg2_seek_header;
		return STATE_INVALID;
	    }
	}
	mpeg2dec->bytes_since_tag += copied;

	if (process_header[mpeg2dec->code & 0x0b] (mpeg2dec)) {
	    mpeg2dec->code = mpeg2dec->buf_start[-1];
	    mpeg2dec->action = mpeg2_seek_header;
	    return STATE_INVALID;
	}

	mpeg2dec->code = mpeg2dec->buf_start[-1];
	switch (RECEIVED (mpeg2dec->code, mpeg2dec->state)) {

	/* state transition after a sequence header */
	case RECEIVED (0x00, STATE_SEQUENCE):
	    mpeg2dec->action = mpeg2_header_picture_start;
	case RECEIVED (0xb8, STATE_SEQUENCE):
	    mpeg2_header_sequence_finalize (mpeg2dec);
	    break;

	/* other legal state transitions */
	case RECEIVED (0x00, STATE_GOP):
	    mpeg2_header_gop_finalize (mpeg2dec);
	    mpeg2dec->action = mpeg2_header_picture_start;
	    break;
	case RECEIVED (0x01, STATE_PICTURE):
//	case RECEIVED (0x01, STATE_PICTURE_2ND):
	    mpeg2_header_picture_finalize (mpeg2dec, mpeg2_accels);
	    mpeg2dec->action = mpeg2_header_slice_start;
	    break;

	/* legal headers within a given state */
	case RECEIVED (0xb2, STATE_SEQUENCE):
	case RECEIVED (0xb2, STATE_GOP):
	case RECEIVED (0xb2, STATE_PICTURE):
//	case RECEIVED (0xb2, STATE_PICTURE_2ND):
	case RECEIVED (0xb5, STATE_SEQUENCE):
	case RECEIVED (0xb5, STATE_PICTURE):
//	case RECEIVED (0xb5, STATE_PICTURE_2ND):
	    mpeg2dec->chunk_ptr = mpeg2dec->chunk_start;
	    continue;

	default:
	    mpeg2dec->action = mpeg2_seek_header;
	    return STATE_INVALID;
	}

	mpeg2dec->chunk_start = mpeg2dec->chunk_ptr = mpeg2dec->chunk_buffer;
	mpeg2dec->user_data_len = 0;
	return mpeg2dec->state;
    }
}

int mpeg2_convert (mpeg2dec_t * mpeg2dec, mpeg2_convert_t convert, void * arg)
{
    mpeg2_convert_init_t convert_init;
    int error;

    error = convert (MPEG2_CONVERT_SET, NULL, &(mpeg2dec->sequence), 0,
		     mpeg2_accels, arg, &convert_init);
    if (!error) {
	mpeg2dec->convert = convert;
	mpeg2dec->convert_arg = arg;
	mpeg2dec->convert_id_size = convert_init.id_size;
	mpeg2dec->convert_stride = 0;
    }
    return error;
}

int mpeg2_stride (mpeg2dec_t * mpeg2dec, int stride)
{
    if (!mpeg2dec->convert) {
	if (stride < (int) mpeg2dec->sequence.width)
	    stride = mpeg2dec->sequence.width;
	mpeg2dec->decoder->stride_frame = stride;
    } else {
	mpeg2_convert_init_t convert_init;

	stride = mpeg2dec->convert (MPEG2_CONVERT_STRIDE, NULL,
				    &(mpeg2dec->sequence), stride,
				    mpeg2_accels, mpeg2dec->convert_arg,
				    &convert_init);
	mpeg2dec->convert_id_size = convert_init.id_size;
	mpeg2dec->convert_stride = stride;
    }
    return stride;
}

void mpeg2_set_buf (mpeg2dec_t * mpeg2dec, uint8_t * buf[3], void * id)
{
    mpeg2_fbuf_t * fbuf;

    if (mpeg2dec->custom_fbuf) {
	if (mpeg2dec->state == STATE_SEQUENCE) {
	    mpeg2dec->fbuf[2] = mpeg2dec->fbuf[1];
	    mpeg2dec->fbuf[1] = mpeg2dec->fbuf[0];
	}
	mpeg2_set_fbuf (mpeg2dec, (mpeg2dec->decoder->coding_type ==
				   PIC_FLAG_CODING_TYPE_B));
	fbuf = mpeg2dec->fbuf[0];
    } else {
	fbuf = &(mpeg2dec->fbuf_alloc[mpeg2dec->alloc_index].fbuf);
	mpeg2dec->alloc_index_user = ++mpeg2dec->alloc_index;
    }
    fbuf->buf[0] = buf[0];
    fbuf->buf[1] = buf[1];
    fbuf->buf[2] = buf[2];
    fbuf->id = id;
    // HACK! FIXME! At first I frame, copy pointers to prediction frame too!
    if (mpeg2dec->custom_fbuf && !mpeg2dec->fbuf[1]->buf[0]){
	mpeg2dec->fbuf[1]->buf[0]=buf[0];
	mpeg2dec->fbuf[1]->buf[1]=buf[1];
	mpeg2dec->fbuf[1]->buf[2]=buf[2];
	mpeg2dec->fbuf[1]->id=NULL;
    }
//        printf("libmpeg2: FBUF 0:%p 1:%p 2:%p\n",
//	    mpeg2dec->fbuf[0]->buf[0],mpeg2dec->fbuf[1]->buf[0],mpeg2dec->fbuf[2]->buf[0]);
}

void mpeg2_custom_fbuf (mpeg2dec_t * mpeg2dec, int custom_fbuf)
{
    mpeg2dec->custom_fbuf = custom_fbuf;
}

void mpeg2_skip (mpeg2dec_t * mpeg2dec, int skip)
{
    mpeg2dec->first_decode_slice = 1;
    mpeg2dec->nb_decode_slices = skip ? 0 : (0xb0 - 1);
}

void mpeg2_slice_region (mpeg2dec_t * mpeg2dec, int start, int end)
{
    start = (start < 1) ? 1 : (start > 0xb0) ? 0xb0 : start;
    end = (end < start) ? start : (end > 0xb0) ? 0xb0 : end;
    mpeg2dec->first_decode_slice = start;
    mpeg2dec->nb_decode_slices = end - start;
}

void mpeg2_tag_picture (mpeg2dec_t * mpeg2dec, uint32_t tag, uint32_t tag2)
{
    mpeg2dec->tag_previous = mpeg2dec->tag_current;
    mpeg2dec->tag2_previous = mpeg2dec->tag2_current;
    mpeg2dec->tag_current = tag;
    mpeg2dec->tag2_current = tag2;
    mpeg2dec->num_tags++;
    mpeg2dec->bytes_since_tag = 0;
}

uint32_t mpeg2_accel (uint32_t accel)
{
    if (!mpeg2_accels) {
/*
	if (accel & MPEG2_ACCEL_DETECT)
	    accel |= mpeg2_detect_accel ();
*/
//	mpeg2_accels = accel |= MPEG2_ACCEL_DETECT;
/*
	mpeg2_cpu_state_init (accel);
*/
	mpeg2_idct_init (accel);
	mpeg2_mc_init (accel);
    }
    return mpeg2_accels & ~MPEG2_ACCEL_DETECT;
}

void mpeg2_reset (mpeg2dec_t * mpeg2dec, int full_reset)
{
    mpeg2dec->buf_start = mpeg2dec->buf_end = NULL;
    mpeg2dec->num_tags = 0;
    mpeg2dec->shift = 0xffffff00;
    mpeg2dec->code = 0xb4;
    mpeg2dec->action = mpeg2_seek_header;
    mpeg2dec->state = STATE_INVALID;
    mpeg2dec->first = 1;

    mpeg2_reset_info(&(mpeg2dec->info));
    mpeg2dec->info.gop = NULL;
    mpeg2dec->info.user_data = NULL;
    mpeg2dec->info.user_data_len = 0;
    if (full_reset) {
	mpeg2dec->info.sequence = NULL;
	mpeg2_header_state_init (mpeg2dec);
    }

}

static DATA_IN_MTCM_VAR  u8 buf1[sizeof(mpeg2dec_t)] ATTR_ALIGN16;
static DATA_IN_MTCM_VAR ATTR_ALIGN16 u8 buf2[sizeof(mpeg2_decoder_t)];
static DATA_IN_MTCM_VAR ATTR_ALIGN16 u8 buf3[sizeof(motions_t)];
static DATA_IN_MTCM_VAR ATTR_ALIGN16 u8 buf4[32*64*2];
static DATA_IN_MTCM_VAR ATTR_ALIGN16 u8 buf5[32*64*2];
static DATA_IN_MTCM_VAR ATTR_ALIGN16 u8 buf6[32*64*2];
static DATA_IN_MTCM_VAR ATTR_ALIGN16 u8 buf7[32*64*2];

mpeg2dec_t * mpeg2_init (void)
{
    mpeg2dec_t * mpeg2dec;
	u32 idx;

    mpeg2_accel (MPEG2_ACCEL_DETECT);

//    mpeg2dec = (mpeg2dec_t *) mpeg2_malloc (sizeof (mpeg2dec_t), MPEG2_ALLOC_MPEG2DEC);
    mpeg2dec = (mpeg2dec_t *) buf1;
    memset (mpeg2dec,0,sizeof(mpeg2dec_t));
    
//    mpeg2dec->decoder = (mpeg2_decoder_t *) mpeg2_malloc (sizeof (mpeg2_decoder_t), MPEG2_ALLOC_MPEG2DEC);
    mpeg2dec->decoder = (mpeg2_decoder_t *) buf2;
    memset (mpeg2dec->decoder,0,sizeof(mpeg2_decoder_t));
    
//    mpeg2dec->decoder->motions = (motions_t *) mpeg2_malloc (sizeof (motions_t), MPEG2_ALLOC_MPEG2DEC);
    mpeg2dec->decoder->motions = (motions_t *) buf3;
    memset (mpeg2dec->decoder->motions,0,sizeof(motions_t));
    
    mpeg2dec->decoder->quantizer_prescale[0] = (uint16_t(*)[32][64]) buf4;
    mpeg2dec->decoder->quantizer_prescale[1] = (uint16_t(*)[32][64]) mpeg2_malloc (32*64*2, MPEG2_ALLOC_MPEG2DEC);
    mpeg2dec->decoder->quantizer_prescale[2] = (uint16_t(*)[32][64]) mpeg2_malloc (32*64*2, MPEG2_ALLOC_MPEG2DEC);
    mpeg2dec->decoder->quantizer_prescale[3] = (uint16_t(*)[32][64]) mpeg2_malloc (32*64*2, MPEG2_ALLOC_MPEG2DEC);
    
    for(idx=0;idx<4;idx++){
      memset (mpeg2dec->decoder->quantizer_prescale[idx],0,32*64*2);
    }

    memset (mpeg2dec->decoder->DCTblock, 0, 64 * sizeof (int16_t));
    memset (mpeg2dec->quantizer_matrix, 0, 4 * 64 * sizeof (uint8_t));

    mpeg2dec->chunk_buffer = (uint8_t *) mpeg2_malloc (BUFFER_SIZE + 4, MPEG2_ALLOC_CHUNK);
    memset (mpeg2dec->chunk_buffer,0,BUFFER_SIZE + 4);

    mpeg2dec->sequence.width = (unsigned)-1;
    mpeg2_reset (mpeg2dec, 1);
    
    /*
    _consolePrintf("sizeof(mpeg2dec_t)=%d\n",sizeof(mpeg2dec_t));
    _consolePrintf("sizeof(mpeg2_decoder_t)=%d\n",sizeof(mpeg2_decoder_t));
    _consolePrintf("sizeof(motions_t)=%d\n",sizeof(motions_t));

    {
	  mpeg2dec_t *pt=0;
      _consolePrintf("%d %d,%d,%d\n",&pt->new_sequence,sizeof(mpeg2_sequence_t),sizeof(mpeg2_gop_t),sizeof(mpeg2_picture_t));
	}
	*/
	
	/*
	sizeof(mpeg2dec_t)=1100
    _consolePrintf("sizeof(mpeg2_decoder_t)=196
    _consolePrintf("sizeof(motions_t)=72
    
	&pt->new_sequence 120
	sizeof(mpeg2_sequence_t) 60
	sizeof(mpeg2_gop_t) 8
	sizeof(mpeg2_picture_t) 36
	
	&pt->scan 116
	*/
	
    return mpeg2dec;
}

void mpeg2_close (mpeg2dec_t * mpeg2dec)
{
    mpeg2_header_state_init (mpeg2dec);
    mpeg2_free (mpeg2dec->chunk_buffer);
    mpeg2_free (mpeg2dec->decoder->quantizer_prescale[3]);
    mpeg2_free (mpeg2dec->decoder->quantizer_prescale[2]);
    mpeg2_free (mpeg2dec->decoder->quantizer_prescale[1]);
//    mpeg2_free (mpeg2dec->decoder->quantizer_prescale[0]);
//    mpeg2_free (mpeg2dec->decoder->motions);
//    mpeg2_free (mpeg2dec->decoder);
//    mpeg2_free (mpeg2dec);
}
