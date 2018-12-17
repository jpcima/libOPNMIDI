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

#include "2608intf.h"

DEFINE_DEVICE_TYPE(YM2608, ym2608_device, "ym2608", "YM2608 OPNA")

ym2608_device::ym2608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
    : ay8910_device(mconfig, YM2608, tag, owner, clock, PSG_TYPE_YM, 1, 2)
{
}

const ssg_callbacks ym2608_device::psgintf =
{
	&ym2608_device::psg_set_clock,
	&ym2608_device::psg_write,
	&ym2608_device::psg_read,
	&ym2608_device::psg_reset
};
