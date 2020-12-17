/*
 * VC-1 and WMV3 decoder
 * Copyright (c) 2011 Mashiat Sarker Shakkhar
 * Copyright (c) 2006-2007 Konstantin Shishkov
 * Partly based on vc9.c (c) 2005 Anonymous, Alex Beregszaszi, Michael Niedermayer
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * VC-1 and WMV3 block decoding routines
 */

#include "avcodec.h"
#include "mpegutils.h"
#include "mpegvideo.h"
#include "msmpeg4data.h"
#include "unary.h"
#include "vc1.h"
#include "vc1_pred.h"
#include "vc1acdata.h"
#include "vc1data.h"

int vc1_decode_ac_coeff(VC1Context *v, int *last, int *skip,
                                int *value, int codingset);

int vc1_decode_p_block(VC1Context *v, int16_t block[64], int n,
                              int mquant, int ttmb, int first_block,
                              uint8_t *dst, int linesize, int skip_block,
                              int *ttmb_out);

int vc1_decode_p_block(VC1Context *v, int16_t block[64], int n,
                              int mquant, int ttmb, int first_block,
                              uint8_t *dst, int linesize, int skip_block,
                              int *ttmb_out)
{
    MpegEncContext *s = &v->s;
    GetBitContext *gb = &s->gb;
    int i, j;
    int subblkpat = 0;
    int scale, off, idx, last, skip, value;
    int ttblk = ttmb & 7;
    int pat = 0;
    int quant = FFABS(mquant);

    s->bdsp.clear_block(block);

    if (ttmb == -1) {
        ttblk = ff_vc1_ttblk_to_tt[v->tt_index][get_vlc2(gb, ff_vc1_ttblk_vlc[v->tt_index].table, VC1_TTBLK_VLC_BITS, 1)];
    }
    if (ttblk == TT_4X4) {
        subblkpat = ~(get_vlc2(gb, ff_vc1_subblkpat_vlc[v->tt_index].table, VC1_SUBBLKPAT_VLC_BITS, 1) + 1);
    }
    if ((ttblk != TT_8X8 && ttblk != TT_4X4)
        && ((v->ttmbf || (ttmb != -1 && (ttmb & 8) && !first_block))
            || (!v->res_rtm_flag && !first_block))) {
        subblkpat = decode012(gb);
        if (subblkpat)
            subblkpat ^= 3; // swap decoded pattern bits
        if (ttblk == TT_8X4_TOP || ttblk == TT_8X4_BOTTOM)
            ttblk = TT_8X4;
        if (ttblk == TT_4X8_RIGHT || ttblk == TT_4X8_LEFT)
            ttblk = TT_4X8;
    }
    scale = quant * 2 + ((mquant < 0) ? 0 : v->halfpq);

    // convert transforms like 8X4_TOP to generic TT and SUBBLKPAT
    if (ttblk == TT_8X4_TOP || ttblk == TT_8X4_BOTTOM) {
        subblkpat = 2 - (ttblk == TT_8X4_TOP);
        ttblk     = TT_8X4;
    }
    if (ttblk == TT_4X8_RIGHT || ttblk == TT_4X8_LEFT) {
        subblkpat = 2 - (ttblk == TT_4X8_LEFT);
        ttblk     = TT_4X8;
    }
    switch (ttblk) {
    case TT_8X8:
        pat  = 0xF;
        i    = 0;
        last = 0;
        while (!last) {
            int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
            if (ret < 0)
                return ret;
            i += skip;
            if (i > 63)
                break;
            if (!v->fcm)
                idx = v->zz_8x8[0][i++];
            else
                idx = v->zzi_8x8[i++];
            block[idx] = value * scale;
            if (!v->pquantizer)
                block[idx] += (block[idx] < 0) ? -quant : quant;
        }
        if (!skip_block) {
            if (i == 1)
                v->vc1dsp.vc1_inv_trans_8x8_dc(dst, linesize, block);
            else {
                v->vc1dsp.vc1_inv_trans_8x8(block);
                s->idsp.add_pixels_clamped(block, dst, linesize);
            }
        }
        break;
    case TT_4X4:
        pat = ~subblkpat & 0xF;
        for (j = 0; j < 4; j++) {
            last = subblkpat & (1 << (3 - j));
            i    = 0;
            off  = (j & 1) * 4 + (j & 2) * 16;
            while (!last) {
                int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
                if (ret < 0)
                    return ret;
                i += skip;
                if (i > 15)
                    break;
                if (!v->fcm)
                    idx = ff_vc1_simple_progressive_4x4_zz[i++];
                else
                    idx = ff_vc1_adv_interlaced_4x4_zz[i++];
                block[idx + off] = value * scale;
                if (!v->pquantizer)
                    block[idx + off] += (block[idx + off] < 0) ? -quant : quant;
            }
            if (!(subblkpat & (1 << (3 - j))) && !skip_block) {
                if (i == 1)
                    v->vc1dsp.vc1_inv_trans_4x4_dc(dst + (j & 1) * 4 + (j & 2) * 2 * linesize, linesize, block + off);
                else
                    v->vc1dsp.vc1_inv_trans_4x4(dst + (j & 1) * 4 + (j & 2) *  2 * linesize, linesize, block + off);
            }
        }
        break;
    case TT_8X4:
        pat = ~((subblkpat & 2) * 6 + (subblkpat & 1) * 3) & 0xF;
        for (j = 0; j < 2; j++) {
            last = subblkpat & (1 << (1 - j));
            i    = 0;
            off  = j * 32;
            while (!last) {
                int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
                if (ret < 0)
                    return ret;
                i += skip;
                if (i > 31)
                    break;
                if (!v->fcm)
                    idx = v->zz_8x4[i++] + off;
                else
                    idx = ff_vc1_adv_interlaced_8x4_zz[i++] + off;
                block[idx] = value * scale;
                if (!v->pquantizer)
                    block[idx] += (block[idx] < 0) ? -quant : quant;
            }
            if (!(subblkpat & (1 << (1 - j))) && !skip_block) {
                if (i == 1)
                    v->vc1dsp.vc1_inv_trans_8x4_dc(dst + j * 4 * linesize, linesize, block + off);
                else
                    v->vc1dsp.vc1_inv_trans_8x4(dst + j * 4 * linesize, linesize, block + off);
            }
        }
        break;
    case TT_4X8:
        pat = ~(subblkpat * 5) & 0xF;
        for (j = 0; j < 2; j++) {
            last = subblkpat & (1 << (1 - j));
            i    = 0;
            off  = j * 4;
            while (!last) {
                int ret = vc1_decode_ac_coeff(v, &last, &skip, &value, v->codingset2);
                if (ret < 0)
                    return ret;
                i += skip;
                if (i > 31)
                    break;
                if (!v->fcm)
                    idx = v->zz_4x8[i++] + off;
                else
                    idx = ff_vc1_adv_interlaced_4x8_zz[i++] + off;
                block[idx] = value * scale;
                if (!v->pquantizer)
                    block[idx] += (block[idx] < 0) ? -quant : quant;
            }
            if (!(subblkpat & (1 << (1 - j))) && !skip_block) {
                if (i == 1)
                    v->vc1dsp.vc1_inv_trans_4x8_dc(dst + j * 4, linesize, block + off);
                else
                    v->vc1dsp.vc1_inv_trans_4x8(dst + j*4, linesize, block + off);
            }
        }
        break;
    }
    if (ttmb_out)
        *ttmb_out |= ttblk << (n * 4);
    return pat;
}
