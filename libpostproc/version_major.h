/*
 * Version macros.
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

#ifndef POSTPROC_VERSION_MAJOR_H
#define POSTPROC_VERSION_MAJOR_H

#include "libavutil/avconfig.h"

/**
 * @file
 * Libpostproc version macros
 */

#if AV_USE_NEXT_ABI
#define LIBPOSTPROC_VERSION_MAJOR  58
#else
#define LIBPOSTPROC_VERSION_MAJOR  57
#endif

#endif /* POSTPROC_VERSION_MAJOR_H */
