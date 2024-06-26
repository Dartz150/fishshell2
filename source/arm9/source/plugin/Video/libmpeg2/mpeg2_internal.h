/*
 * mpeg2_internal.h
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
 * $Id: mpeg2_internal.h,v 1.21 2005/02/19 02:32:12 diego Exp $
 */
#define ATTR_ALIGN16 __attribute__((aligned(16)))
#define ATTR_ALIGN32 __attribute__((aligned(32)))
#define ATTR_ALIGN64 __attribute__((aligned(64)))

/* macroblock modes */
#define MACROBLOCK_INTRA 1
#define MACROBLOCK_PATTERN 2
#define MACROBLOCK_MOTION_BACKWARD 4
#define MACROBLOCK_MOTION_FORWARD 8
#define MACROBLOCK_QUANT 16
#define DCT_TYPE_INTERLACED 32
/* motion_type */
#define MOTION_TYPE_SHIFT 6
#define MC_FIELD 1
#define MC_FRAME 2
#define MC_16X8 2
#define MC_DMV 3

/* picture structure */
#define TOP_FIELD 1
#define BOTTOM_FIELD 2
#define FRAME_PICTURE 3

/* picture coding type */
#define I_TYPE 1
#define P_TYPE 2
#define B_TYPE 3
#define D_TYPE 4

typedef void mpeg2_mc_fct (uint8_t *, const uint8_t *, int, int);

typedef struct {
//    uint8_t * ref[2][3];
    uint8_t * ref1[3];
//    uint8_t ** ref2[2];
    int pmv[2][2];
    int f_code[2];
} motion_t;

typedef struct motions_s {
    motion_t b_motion;
    motion_t f_motion;
} motions_t;

typedef void motion_parser_t (mpeg2_decoder_t * decoder,
			      motion_t * motion,
			      mpeg2_mc_fct * const * table);

struct mpeg2_decoder_s {
    /* first, state that carries information from one macroblock to the */
    /* next inside a slice, and is never used outside of mpeg2_slice() */

    /* bit parsing stuff */
    uint32_t bitstream_buf;		/* current 32 bit working set */
    int bitstream_bits;			/* used bits in working set */
    const uint8_t * bitstream_ptr;	/* buffer with stream data */

    uint8_t * dest[3]; // for slice.c

    int offset; // for slice.c
    int stride; // for slice.c
    int uv_stride; // for slice.c
    int slice_stride; // for slice.c
    int slice_uv_stride; // for slice.c
    // int stride_frame; // for slice.c one call
    unsigned int limit_x; // for slice.c
    unsigned int limit_y_16;
    // unsigned int limit_y_8;
    unsigned int limit_y; // for slice.c

    /* Motion vectors */
    /* The f_ and b_ correspond to the forward and backward motion */
    /* predictors */
    motions_t *motions; // for slice.c
    //motion_parser_t * motion_parser[5]; // for slice.c little call

    /* predictor for DC coefficients in intra blocks */
    int16_t dc_dct_pred[3]; // for slice.c

    // uint8_t * picture_dest[3]; // for slice.c one call
    void (* convert) (void * convert_id, uint8_t * const * src,
		      unsigned int v_offset);
    void * convert_id;

//    int dmv_offset;
    unsigned int v_offset; // for slice.c

    /* now non-slice-specific information */

    /* sequence header stuff */
    uint16_t * quantizer_matrix[2]; // old4 for slice.c
    //uint16_t (* chroma_quantizer[2])[64]; // for header.c
    //uint16_t (*quantizer_prescale[4])[32][64]; // for slice.c one call

    /* The width and height of the picture snapped to macroblock units */
    int width; // for slice.c
    //int height; // for slice.c init
    //int vertical_position_extension; // for slice.c init
    int chroma_format;

    /* picture header stuff */

    /* what type of picture this is (I, P, B, D) */
    int coding_type; // for slice.c

    /* picture coding extension stuff */

    /* quantization factor for intra dc coefficients */
    int intra_dc_precision; // for slice.c
    /* top/bottom/both fields */
    int picture_structure; // for slice.c
    /* bool to indicate all predictions are frame based */
    int frame_pred_frame_dct; // for slice.c
    /* bool to indicate whether intra blocks have motion vectors */
    /* (for concealment) */
    int concealment_motion_vectors; // for slice.c
    /* bool to use different vlc tables */
//    int intra_vlc_format;
    /* used for DMV MC */
    // int top_field_first; // for header.c

    /* stuff derived from bitstream */

    /* pointer to the zigzag scan we're supposed to be using */
    const uint8_t * scan;

    // moved for optimize
    
    int stride_frame; // for slice.c one call
    
    motion_parser_t * motion_parser[5]; // for slice.c little call
    
    uint8_t * picture_dest[3]; // for slice.c one call

    /* The width and height of the picture snapped to macroblock units */
    int height; // for slice.c init
    int vertical_position_extension; // for slice.c init
    
    /* sequence header stuff */
    uint16_t (* chroma_quantizer[2])[64]; // for header.c
    uint16_t (*quantizer_prescale[4])[32][64]; // for slice.c one call
    
    /* used for DMV MC */
    int top_field_first; // for header.c
    
//    int second_field;

//    int mpeg1;

    /* DCT coefficients */
    int16_t DCTblock[64];

    /* for MPlayer: */
/*
    int quantizer_scales[32];
    int quantizer_scale;
    char* quant_store;
    int quant_stride;
*/
};

typedef struct {
    mpeg2_fbuf_t fbuf;
} fbuf_alloc_t;

struct mpeg2dec_s {
    mpeg2_decoder_t *decoder;
    
    mpeg2_info_t info;
    
    uint32_t shift;
    int is_display_initialized;
    mpeg2_state_t (* action) (struct mpeg2dec_s * mpeg2dec);
    mpeg2_state_t state;
    uint32_t ext_state;

    /* allocated in init - gcc has problems allocating such big structures */
    uint8_t * chunk_buffer;
    /* pointer to start of the current chunk */
    uint8_t * chunk_start;
    /* pointer to current position in chunk_buffer */
    uint8_t * chunk_ptr;
    /* last start code ? */
    uint8_t code;

    /* picture tags */
    uint32_t tag_current, tag2_current, tag_previous, tag2_previous;
    int num_tags;
    int bytes_since_tag;

    int first;
    int alloc_index_user;
    int alloc_index;
    uint8_t first_decode_slice;
    uint8_t nb_decode_slices;

    unsigned int user_data_len;

    mpeg2_sequence_t new_sequence;
    mpeg2_sequence_t sequence;
    mpeg2_gop_t new_gop;
    mpeg2_gop_t gop;
    mpeg2_picture_t new_picture;
    mpeg2_picture_t pictures[4];
    mpeg2_picture_t * picture;
    /*const*/ mpeg2_fbuf_t * fbuf[3];	/* 0: current fbuf, 1-2: prediction fbufs */

    fbuf_alloc_t fbuf_alloc[3]; // offset 500byte
    int custom_fbuf;

    uint8_t * yuv_buf[3][3];
    int yuv_index;
    mpeg2_convert_t * convert;
    void * convert_arg;
    unsigned int convert_id_size;
    int convert_stride;
    void (* convert_start) (void * id, const mpeg2_fbuf_t * fbuf,
			    const mpeg2_picture_t * picture,
			    const mpeg2_gop_t * gop);

    int16_t display_offset_x, display_offset_y;

    uint8_t * buf_start;
    uint8_t * buf_end;

    int copy_matrix;
    int8_t q_scale_type, scaled[4];
    uint8_t quantizer_matrix[4][64];
    uint8_t new_quantizer_matrix[4][64];
    
    /* for MPlayer: */
/*
    unsigned char *pending_buffer;
    int pending_length;
*/
};

/* decode.c */
mpeg2_state_t mpeg2_seek_header (mpeg2dec_t * mpeg2dec);
mpeg2_state_t mpeg2_parse_header (mpeg2dec_t * mpeg2dec);

/* header.c */
void mpeg2_header_state_init (mpeg2dec_t * mpeg2dec);
void mpeg2_reset_info (mpeg2_info_t * info);
int mpeg2_header_sequence (mpeg2dec_t * mpeg2dec);
int mpeg2_header_gop (mpeg2dec_t * mpeg2dec);
mpeg2_state_t mpeg2_header_picture_start (mpeg2dec_t * mpeg2dec);
int mpeg2_header_picture (mpeg2dec_t * mpeg2dec);
int mpeg2_header_extension (mpeg2dec_t * mpeg2dec);
int mpeg2_header_user_data (mpeg2dec_t * mpeg2dec);
void mpeg2_header_sequence_finalize (mpeg2dec_t * mpeg2dec);
void mpeg2_header_gop_finalize (mpeg2dec_t * mpeg2dec);
void mpeg2_header_picture_finalize (mpeg2dec_t * mpeg2dec, uint32_t accels);
mpeg2_state_t mpeg2_header_slice_start (mpeg2dec_t * mpeg2dec);
mpeg2_state_t mpeg2_header_end (mpeg2dec_t * mpeg2dec);
void mpeg2_set_fbuf (mpeg2dec_t * mpeg2dec, int b_type);

/* idct.c */
void mpeg2_idct_init (uint32_t accel);

/* idct_mmx.c */
void mpeg2_idct_copy_mmxext (int16_t * block, uint8_t * dest, int stride);
void mpeg2_idct_add_mmxext (int last, int16_t * block,
			    uint8_t * dest, int stride);
void mpeg2_idct_copy_mmx (int16_t * block, uint8_t * dest, int stride);
void mpeg2_idct_add_mmx (int last, int16_t * block,
			 uint8_t * dest, int stride);
void mpeg2_idct_mmx_init (void);

/* idct_altivec.c */
void mpeg2_idct_copy_altivec (int16_t * block, uint8_t * dest, int stride);
void mpeg2_idct_add_altivec (int last, int16_t * block,
			     uint8_t * dest, int stride);
void mpeg2_idct_altivec_init (void);

/* idct_alpha.c */
void mpeg2_idct_copy_mvi (int16_t * block, uint8_t * dest, int stride);
void mpeg2_idct_add_mvi (int last, int16_t * block,
			 uint8_t * dest, int stride);
void mpeg2_idct_copy_alpha (int16_t * block, uint8_t * dest, int stride);
void mpeg2_idct_add_alpha (int last, int16_t * block,
			   uint8_t * dest, int stride);
void mpeg2_idct_alpha_init (void);

/* motion_comp.c */
void mpeg2_mc_init (uint32_t accel);

typedef struct {
    mpeg2_mc_fct * put [8];
    mpeg2_mc_fct * avg [8];
} mpeg2_mc_t;

#define MPEG2_MC_EXTERN(x) mpeg2_mc_t mpeg2_mc_##x = {			  \
    {MC_put_o_16_##x, MC_put_x_16_##x, MC_put_y_16_##x, MC_put_xy_16_##x, \
     MC_put_o_8_##x,  MC_put_x_8_##x,  MC_put_y_8_##x,  MC_put_xy_8_##x}, \
    {MC_avg_o_16_##x, MC_avg_x_16_##x, MC_avg_y_16_##x, MC_avg_xy_16_##x, \
     MC_avg_o_8_##x,  MC_avg_x_8_##x,  MC_avg_y_8_##x,  MC_avg_xy_8_##x}  \
};

extern mpeg2_mc_t mpeg2_mc_c;
extern mpeg2_mc_t mpeg2_mc_arm;
