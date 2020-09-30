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

#ifndef AVCODEC_MF_UTILS_H
#define AVCODEC_MF_UTILS_H

#include <windows.h>
#include <initguid.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfobjects.h>
#include <mftransform.h>
#ifdef _MSC_VER
// The official way of including codecapi (via dshow.h) makes the ICodecAPI
// interface unavailable in UWP mode, but including icodecapi.h + codecapi.h
// seems to be equivalent. (These headers conflict with the official way
// of including it though, through strmif.h via dshow.h. And on mingw, the
// mf*.h headers below indirectly include strmif.h.)
#include <icodecapi.h>
#else
#include <dshow.h>
#endif
// Older versions of mingw-w64 need codecapi.h explicitly included, while newer
// ones include it implicitly from dshow.h (via uuids.h).
#include <codecapi.h>

#include "avcodec.h"

// These functions do exist in mfapi.h, but are only available within
// __cplusplus ifdefs.
HRESULT ff_MFGetAttributeSize(IMFAttributes *pattr, REFGUID guid,
                              UINT32 *pw, UINT32 *ph);
HRESULT ff_MFSetAttributeSize(IMFAttributes *pattr, REFGUID guid,
                              UINT32 uw, UINT32 uh);
#define ff_MFSetAttributeRatio ff_MFSetAttributeSize
#define ff_MFGetAttributeRatio ff_MFGetAttributeSize

// MFTEnumEx was missing from mingw-w64's mfplat import library until
// mingw-w64 v6.0.0, thus wrap it and load it using GetProcAddress.
// It's also missing in Windows Vista's mfplat.dll.
HRESULT ff_MFTEnumEx(GUID guidCategory, UINT32 Flags,
                     const MFT_REGISTER_TYPE_INFO *pInputType,
                     const MFT_REGISTER_TYPE_INFO *pOutputType,
                     IMFActivate ***pppMFTActivate, UINT32 *pnumMFTActivate);

DEFINE_GUID(ff_CODECAPI_AVScenarioInfo, 0xb28a6e64,0x3ff9,0x446a,0x8a,0x4b,0x0d,0x7a,0x53,0x41,0x32,0x36);

char *ff_hr_str_buf(char *buf, size_t size, HRESULT hr);
#define ff_hr_str(hr) ff_hr_str_buf((char[80]){0}, 80, hr)

// Possibly compiler-dependent; the MS/MinGW definition for this is just crazy.
#define FF_VARIANT_VALUE(type, contents) &(VARIANT){ .vt = (type), contents }

#define FF_VAL_VT_UI4(v) FF_VARIANT_VALUE(VT_UI4, .ulVal = (v))
#define FF_VAL_VT_BOOL(v) FF_VARIANT_VALUE(VT_BOOL, .boolVal = (v))

IMFSample *ff_create_memory_sample(void *fill_data, size_t size, size_t align);
enum AVSampleFormat ff_media_type_to_sample_fmt(IMFAttributes *type);
enum AVPixelFormat ff_media_type_to_pix_fmt(IMFAttributes *type);
const GUID *ff_pix_fmt_to_guid(enum AVPixelFormat pix_fmt);
int ff_fourcc_from_guid(const GUID *guid, uint32_t *out_fourcc);
char *ff_guid_str_buf(char *buf, size_t buf_size, const GUID *guid);
#define ff_guid_str(guid) ff_guid_str_buf((char[80]){0}, 80, guid)
void ff_attributes_dump(void *log, IMFAttributes *attrs);
void ff_media_type_dump(void *log, IMFMediaType *type);
const CLSID *ff_codec_to_mf_subtype(enum AVCodecID codec);
int ff_instantiate_mf(void *log, GUID category,
                      MFT_REGISTER_TYPE_INFO *in_type,
                      MFT_REGISTER_TYPE_INFO *out_type,
                      int use_hw, IMFTransform **res);
void ff_free_mf(IMFTransform **mft);

#endif
