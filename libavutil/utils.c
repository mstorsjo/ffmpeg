/*
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

#include "config.h"
#include "avutil.h"
#include "avassert.h"

/**
 * @file
 * various utility functions
 */

const char *av_get_media_type_string(enum AVMediaType media_type)
{
    switch (media_type) {
    case AVMEDIA_TYPE_VIDEO:      return "video";
    case AVMEDIA_TYPE_AUDIO:      return "audio";
    case AVMEDIA_TYPE_DATA:       return "data";
    case AVMEDIA_TYPE_SUBTITLE:   return "subtitle";
    case AVMEDIA_TYPE_ATTACHMENT: return "attachment";
    default:                      return NULL;
    }
}

char av_get_picture_type_char(enum AVPictureType pict_type)
{
    switch (pict_type) {
    case AV_PICTURE_TYPE_I:  return 'I';
    case AV_PICTURE_TYPE_P:  return 'P';
    case AV_PICTURE_TYPE_B:  return 'B';
    case AV_PICTURE_TYPE_S:  return 'S';
    case AV_PICTURE_TYPE_SI: return 'i';
    case AV_PICTURE_TYPE_SP: return 'p';
    case AV_PICTURE_TYPE_BI: return 'b';
    default:                 return '?';
    }
}

#if FF_API_OPT_INT_LIST
unsigned av_int_list_length_for_size(unsigned elsize,
                                     const void *list, uint64_t term)
{
    unsigned i;

    if (!list)
        return 0;
#define LIST_LENGTH(type) \
    { type t = term, *l = (type *)list; for (i = 0; l[i] != t; i++); }
    switch (elsize) {
    case 1: LIST_LENGTH(uint8_t);  break;
    case 2: LIST_LENGTH(uint16_t); break;
    case 4: LIST_LENGTH(uint32_t); break;
    case 8: LIST_LENGTH(uint64_t); break;
    default: av_assert0(!"valid element size");
    }
    return i;
}
#endif

char *av_fourcc_make_string(char *buf, uint32_t fourcc)
{
    int i;
    char *orig_buf = buf;
    size_t buf_size = AV_FOURCC_MAX_STRING_SIZE;

    for (i = 0; i < 4; i++) {
        const int c = fourcc & 0xff;
        const int print_chr = (c >= '0' && c <= '9') ||
                              (c >= 'a' && c <= 'z') ||
                              (c >= 'A' && c <= 'Z') ||
                              (c && strchr(". -_", c));
        const int len = snprintf(buf, buf_size, print_chr ? "%c" : "[%d]", c);
        if (len < 0)
            break;
        buf += len;
        buf_size = buf_size > len ? buf_size - len : 0;
        fourcc >>= 8;
    }

    return orig_buf;
}

AVRational av_get_time_base_q(void)
{
    return (AVRational){1, AV_TIME_BASE};
}

void av_assert0_fpu(void) {
#if HAVE_MMX_INLINE
    uint16_t state[14];
     __asm__ volatile (
        "fstenv %0 \n\t"
        : "+m" (state)
        :
        : "memory"
    );
    av_assert0((state[4] & 3) == 3);
#endif
}

void ff_dump_simd_s8(int line, const char *msg, int first, int last, const int8_t *ptr);
void ff_dump_simd_s8(int line, const char *msg, int first, int last, const int8_t *ptr) {
    fprintf(stderr, "%d: %s:\n", line, msg);
#ifdef __aarch64__
    for (int v = first; v <= last; v++) {
        const int8_t *vec = &ptr[16*v];
        fprintf(stderr, "v%02d = {", v);
        for (int i = 0; i < 16; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#elif defined(__arm__)
    for (int v = first; v <= last; v++) {
        const int8_t *vec = &ptr[8*v];
        fprintf(stderr, "d%02d = {", v);
        for (int i = 0; i < 8; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#endif
    fflush(stderr);
}

void ff_dump_simd_u8(int line, const char *msg, int first, int last, const uint8_t *ptr);
void ff_dump_simd_u8(int line, const char *msg, int first, int last, const uint8_t *ptr) {
    fprintf(stderr, "%d: %s:\n", line, msg);
#ifdef __aarch64__
    for (int v = first; v <= last; v++) {
        const uint8_t *vec = &ptr[16*v];
        fprintf(stderr, "v%02d = {", v);
        for (int i = 0; i < 16; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#elif defined(__arm__)
    for (int v = first; v <= last; v++) {
        const uint8_t *vec = &ptr[8*v];
        fprintf(stderr, "d%02d = {", v);
        for (int i = 0; i < 8; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#endif
    fflush(stderr);
}

void ff_dump_simd_s16(int line, const char *msg, int first, int last, const int16_t *ptr);
void ff_dump_simd_s16(int line, const char *msg, int first, int last, const int16_t *ptr) {
    fprintf(stderr, "%d: %s:\n", line, msg);
#ifdef __aarch64__
    for (int v = first; v <= last; v++) {
        const int16_t *vec = &ptr[8*v];
        fprintf(stderr, "v%02d = {", v);
        for (int i = 0; i < 8; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#elif defined(__arm__)
    for (int v = first; v <= last; v++) {
        const int16_t *vec = &ptr[4*v];
        fprintf(stderr, "d%02d = {", v);
        for (int i = 0; i < 4; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#endif
    fflush(stderr);
}

void ff_dump_simd_u16(int line, const char *msg, int first, int last, const uint16_t *ptr);
void ff_dump_simd_u16(int line, const char *msg, int first, int last, const uint16_t *ptr) {
    fprintf(stderr, "%d: %s:\n", line, msg);
#ifdef __aarch64__
    for (int v = first; v <= last; v++) {
        const uint16_t *vec = &ptr[8*v];
        fprintf(stderr, "v%02d = {", v);
        for (int i = 0; i < 8; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#elif defined(__arm__)
    for (int v = first; v <= last; v++) {
        const uint16_t *vec = &ptr[4*v];
        fprintf(stderr, "d%02d = {", v);
        for (int i = 0; i < 4; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#endif
    fflush(stderr);
}

void ff_dump_simd_s32(int line, const char *msg, int first, int last, const int32_t *ptr);
void ff_dump_simd_s32(int line, const char *msg, int first, int last, const int32_t *ptr) {
    fprintf(stderr, "%d: %s:\n", line, msg);
#ifdef __aarch64__
    for (int v = first; v <= last; v++) {
        const int32_t *vec = &ptr[4*v];
        fprintf(stderr, "v%02d = {", v);
        for (int i = 0; i < 4; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#elif defined(__arm__)
    for (int v = first; v <= last; v++) {
        const int32_t *vec = &ptr[2*v];
        fprintf(stderr, "d%02d = {", v);
        for (int i = 0; i < 2; i++)
            fprintf(stderr, "%s %d", i > 0 ? "," : "", vec[i]);
        fprintf(stderr, " }\n");
    }
#endif
    fflush(stderr);
}

void ff_dump_gpr(int line, const char *msg, int first, int last, const size_t *ptr);
void ff_dump_gpr(int line, const char *msg, int first, int last, const size_t *ptr) {
    fprintf(stderr, "%d: %s:\n", line, msg);
#ifdef __aarch64__
    for (int r = first; r <= last; r++)
        fprintf(stderr, "x%02d = %"PRIx64"\n", r, (uint64_t) ptr[r]);
#elif defined(__arm__)
    for (int r = first; r <= last; r++)
        if (r < 13)
            fprintf(stderr, "r%02d = %"PRIx32"\n", r, (uint32_t) ptr[r]);
        else
            fprintf(stderr, "%s  = %"PRIx32"\n", r == 13 ? "sp" : "lr", (uint32_t) ptr[r]);
#endif
    fflush(stderr);
}
