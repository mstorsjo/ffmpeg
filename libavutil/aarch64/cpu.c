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

#include "libavutil/cpu.h"
#include "libavutil/cpu_internal.h"
#include "config.h"

#if (defined(__linux__) || defined(__ANDROID__)) && HAVE_GETAUXVAL && HAVE_ASM_HWCAP_H
#include <stdint.h>
#include <asm/hwcap.h>
#include <sys/auxv.h>

#define get_cpu_feature_reg(reg, val) \
        __asm__("mrs %0, " #reg : "=r" (val))

static int detect_flags(void)
{
    unsigned long ret = getauxval(AT_HWCAP);
    int flags = 0;
#if defined(HWCAP_CPUID)
    uint64_t tmp;
    if (!(ret & HWCAP_CPUID))
        return flags;
    get_cpu_feature_reg(ID_AA64ISAR0_EL1, tmp);
    if (((tmp >> 44) & 0xf) == 0x1)
        flags |= AV_CPU_FLAG_DOTPROD;
    get_cpu_feature_reg(ID_AA64ISAR1_EL1, tmp);
    if (((tmp >> 52) & 0xf) == 0x1)
        flags |= AV_CPU_FLAG_I8MM;
#endif

    return flags;
}

#elif defined(__APPLE__) && HAVE_SYSCTLBYNAME
#include <sys/sysctl.h>

static int detect_flags(void)
{
    uint32_t value = 0;
    size_t size;
    int flags = 0;

    size = sizeof(value);
    if (!sysctlbyname("hw.optional.arm.FEAT_DotProd", &value, &size, NULL, 0)) {
        if (value)
            flags |= AV_CPU_FLAG_DOTPROD;
    }
    size = sizeof(value);
    if (!sysctlbyname("hw.optional.arm.FEAT_I8MM", &value, &size, NULL, 0)) {
        if (value)
            flags |= AV_CPU_FLAG_I8MM;
    }
    return flags;
}

#else

static int detect_flags(void)
{
    return 0;
}

#endif

int ff_get_cpu_flags_aarch64(void)
{
    int flags = AV_CPU_FLAG_ARMV8 * HAVE_ARMV8 |
                AV_CPU_FLAG_NEON  * HAVE_NEON  |
                AV_CPU_FLAG_VFP   * HAVE_VFP;

#ifdef __ARM_FEATURE_DOTPROD
    flags |= AV_CPU_FLAG_DOTPROD;
#endif
#ifdef __ARM_FEATURE_I8MM
    flags |= AV_CPU_FLAG_I8MM;
#endif

    flags |= detect_flags();

    return flags;
}

size_t ff_get_cpu_max_align_aarch64(void)
{
    int flags = av_get_cpu_flags();

    if (flags & AV_CPU_FLAG_NEON)
        return 16;

    return 8;
}
