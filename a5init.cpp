/**
 * Copyright (c) 2011 Johannes Schickel (LordHoto)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "a5init.h"

#include <boost/format.hpp>

bool A5InitLoader::isSupported(const std::string &name, const uint32 offset, const uint32 size) throw() {
	// Check whether the name matches
	if (name != "%A5Init")
		return false;

	const byte *memory = _executable.getMemory();
	const uint32 memorySize = _executable.getMemorySize();

	// Check whether it only exports one function
	if (READ_UINT16_BE(memory + offset + 2) != 0x0001)
		return false;

	const uint32 infoOffset = READ_UINT16_BE(memory + offset + 10) + 10;

	// Check whether the information area is still inside the memory dump
	if (offset + infoOffset + 16 >= memorySize)
		return false;

	const uint32 dataOffset = READ_UINT32_BE(memory + offset + infoOffset + 8);
	const uint32 relocationDataOffset = READ_UINT32_BE(memory + offset + infoOffset + 12);

	// Check whether the compressed data is still in the memory dump
	if (offset + dataOffset >= memorySize)
		return false;
	// Check whether the relocation data is still in the memory dump
	if (offset + relocationDataOffset >= memorySize)
		return false;

	// Looks like it is an %A5Init segment
	return true;
}

void A5InitLoader::load(const uint32 offset, const uint32 size, std::ostream &out) throw(std::exception) {
	byte *memory = _executable.getMemory();

	const uint32 infoOffset = READ_UINT16_BE(memory + offset + 10) + 10;
	const uint32 dataSize = READ_UINT32_BE(memory + offset + infoOffset + 0);
	const uint16 needLoadBit = READ_UINT16_BE(memory + offset + infoOffset + 4);
	const uint32 dataOffset = READ_UINT32_BE(memory + offset + infoOffset + 8);
	const uint32 relocationDataOffset = READ_UINT32_BE(memory + offset + infoOffset + 12);

	// Output various information about the %A5Init segment
	out << "%A5Init info data:\n"
	       "\tData size: " << dataSize << "\n"
	       "\tNeed to load: " << needLoadBit << "\n"
	       "\tData offset: " << dataOffset << "\n"
	       "\tRelocation offset: " << relocationDataOffset << std::endl;

	// Check whether we actually have to do some work
	if (needLoadBit != 1) {
		out << "A5 data does not need any initialization" << std::endl;
		return;
	}

	const Code0Segment &code0 = _executable.getCode0Segment();
	uint8 *dst = memory + code0.getApplicationGlobalsSize() - dataSize;

	// uncompress the world
	uncompressA5World(dst, memory + offset + infoOffset + dataOffset);

	// relocate the world
	relocateWorld(code0.getApplicationGlobalsSize(), dst, memory + offset + infoOffset + relocationDataOffset, out);

	// Mark segment as initialized
	WRITE_UINT16_BE(memory + offset + infoOffset + 4, 0);
}

void A5InitLoader::uncompressA5World(uint8 *dst, const uint8 *src) const throw() {
	assert(dst != nullptr);
	assert(src != nullptr);

	while (true) {
		uint32 loops = 1;
		uint32 size = *src++;
		uint32 offset = size;

		size &= 0x0F;
		if (!size) {
			size = getRunLength(src, loops);

			if (!size)
				return;
		} else {
			size += size;
		}

		offset &= 0xF0;
		if (!offset)
			offset = getRunLength(src, loops);
		else
			offset >>= 3;

		do {
			dst += offset;
			std::memcpy(dst, src, size);
			dst += size;
			src += size;
		} while (--loops);
	}
}

uint32 A5InitLoader::getRunLength(const uint8 *&src, uint32 &special) const throw() {
	assert(src != nullptr);

	uint32 rl = *src++;

	if (!(rl & 0x80)) {
		return rl;
	} else if (!(rl & 0x40)) {
		rl &= 0x3F;
		rl <<= 8;
		rl |= *src++;
		return rl;
	} else if (!(rl & 0x20)) {
		rl &= 0x1F;
		rl <<= 8;
		rl |= *src++;
		rl <<= 8;
		rl |= *src++;
		return rl;
	} else if (!(rl & 0x10)) {
		rl = READ_UINT32_BE(src);
		src += 4;
		return rl;
	} else {
		rl = getRunLength(src, special);
		special = getRunLength(src, special);
		return rl;
	}
}

void A5InitLoader::relocateWorld(const uint32 a5, uint8 *dst, const uint8 *src, std::ostream &out) const throw() {
	assert(dst != nullptr);
	assert(src != nullptr);

	uint32 dummy = 0;

	while (true) {
		uint32 loops = 1;
		uint32 offset = *src++;

		if (offset) {
			if (offset & 0x80) {
				offset &= 0x7F;
				offset <<= 8;
				offset |= *src++;
			}
		} else {
			offset = *src++;
			if (!offset)
				return;

			if (offset & 0x80) {
				offset <<= 8;
				offset |= *src++;
				offset <<= 8;
				offset |= *src++;
				offset <<= 8;
				offset |= *src++;
			} else {
				loops = getRunLength(src, dummy);
			}
		}

		offset += offset;

		do {
			dst += offset;
			out << boost::format("Relocation at 0x%1$08X\n") % (dst - _executable.getMemory());
			WRITE_UINT32_BE(dst, READ_UINT32_BE(dst) + a5);
		} while (--loops);
	}
}

