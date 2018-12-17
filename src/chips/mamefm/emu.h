/*
 * libOPNMIDI is a free MIDI to WAV conversion library with OPN2 (YM2612) emulation
 *
 * MIDI parser and player (Original code from ADLMIDI): Copyright (c) 2010-2014 Joel Yliluoma <bisqwit@iki.fi>
 * OPNMIDI Library and YM2612 support:   Copyright (c) 2017-2018 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * Library is based on the ADLMIDI, a MIDI player for Linux and Windows with OPL3 emulation:
 * http://iki.fi/bisqwit/source/adlmidi.html
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPNMIDI_MAMEFM_EMU_H
#define OPNMIDI_MAMEFM_EMU_H

#include <new>
#include <vector>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <assert.h>

#if !defined(M_PI)
#   define M_PI 3.14159265358979323846
#endif

typedef uint32_t offs_t;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

template <class T, class U> T downcast(U *p)
{
    return static_cast<T>(p);
}

#define osd_printf_verbose logerror

inline void vlogerror(const char *fmt, va_list ap)
{
#if defined(OPNMIDI_MAMEFM_EMU_VERBOSE)
    vfprintf(stderr, fmt, ap);
#else
    (void)fmt;
    (void)ap;
#endif
}

inline void logerror(const char *fmt, ...)
{
#if defined(OPNMIDI_MAMEFM_EMU_VERBOSE)
    va_list ap;
    va_start(ap, fmt);
    vlogerror(fmt, ap);
    va_end(ap);
#else
    (void)fmt;
#endif
}

inline void fatalerror(const char *fmt, ...)
{
#if defined(OPNMIDI_MAMEFM_EMU_VERBOSE)
    va_list ap;
    va_start(ap, fmt);
    vlogerror(fmt, ap);
    va_end(ap);
#endif
    std::terminate();
}

struct machine_config
{
};

struct running_machine
{
    const char *describe_context() const { return "machine"; }
};

struct device_type
{
    device_type(const char *shortname, const char *fullname)
        : m_shortname(shortname), m_fullname(fullname) {}

    const char *shortname() const { return m_shortname; }
    const char *fullname() const { return m_fullname; }

    bool operator==(device_type const &that) const { return &that == this; }
    bool operator!=(device_type const &that) const { return &that != this; }

private:
    const char *m_shortname, *m_fullname;
};

#define DECLARE_DEVICE_TYPE(id, ty) extern device_type id;
#define DEFINE_DEVICE_TYPE(id, ty, shortname, fullname) device_type id(shortname, fullname);

struct device_t
{
    device_t(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
        : m_type(type), m_clock(clock), m_machine(NULL)
    {
        (void)mconfig;
        (void)tag;
        (void)owner;
    }
    virtual ~device_t() {}
    const device_type &type() const { return m_type; }
    const char *name() const { return m_type.fullname(); }
    uint32_t clock() const { return m_clock; }
    running_machine &machine() const { return *m_machine; }
    void set_machine(running_machine &machine) { m_machine = &machine; }
    void logerror(const char *fmt, ...);
    void vlogerror(const char *fmt, va_list ap);

    virtual void device_start() {}
    virtual void device_reset() {}
    virtual void device_clock_changed() {}

private:
    device_type m_type;
    uint32_t m_clock;
    running_machine *m_machine;
};

inline void device_t::logerror(const char *fmt, ...)
{
#if defined(OPNMIDI_MAMEFM_EMU_VERBOSE)
    va_list ap;
    va_start(ap, fmt);
    vlogerror(fmt, ap);
    va_end(ap);
#else
    (void)fmt;
#endif
}

inline void device_t::vlogerror(const char *fmt, va_list ap)
{
#if defined(OPNMIDI_MAMEFM_EMU_VERBOSE)
    ::vlogerror(fmt, ap);
#else
    (void)fmt;
    (void)ap;
#endif
}

typedef int32_t stream_sample_t;

struct sound_stream
{
};

struct device_sound_interface
{
    device_sound_interface(const machine_config &mconfig, device_t &device) {}
    virtual ~device_sound_interface() {}
    virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) {}
};

#endif
