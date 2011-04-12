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

#include "data00.h"

#include <boost/lexical_cast.hpp>

Data00Loader::Data00Loader(Executable &exe) throw(std::exception)
    : StaticDataLoader(exe), _resFork(exe.getResourceFork()), _data00(nullptr) {
}

Data00Loader::~Data00Loader() {
	destroy(_data00);
}

void Data00Loader::reset() throw(std::exception) {
	destroy(_data00);
}

bool Data00Loader::isSupported(const CodeSegment &code, const uint32 offset, const uint32 size) throw() {
	const byte *memory = _executable.getMemory();
	const uint32 memorySize = _executable.getMemorySize();

	// TODO: This detection heuristic is probably all wrong...

	// Chechk whether the segment is big enough
	if (memorySize < offset + 0x210)
		return false;

	// Check whether the offset into the jump table is 0
	if (READ_UINT16_BE(memory + offset + 0) != 0)
		return false;

	// Check whether just one function is exported
	if (READ_UINT16_BE(memory + offset + 2) != 1)
		return false;

	// Check whether a "CODE" tag is at 0xA
	if (READ_UINT32_BE(memory + offset + 0x0A) != 0x434F4445)
		return false;

	// Check whether a "DATA" tag is at 0x44
	if (READ_UINT32_BE(memory + offset + 0x44) != 0x44415441)
		return false;

	// Check whether we have an DATA00 resource
	_data00 = _resFork.getResource(0x44415441, 0x0000);
	if (_data00 == nullptr)
		return false;

	return true;
}

void Data00Loader::load(const CodeSegment &code, const uint32 offset, const uint32 size, std::ostream &out) throw(std::exception) {
	assert(_data00->data != nullptr);

	// Uncompress the data
	const byte *src = uncompress(offset, out);
}

const byte *Data00Loader::uncompress(const uint32 offset, std::ostream &out) throw(std::exception) {
	byte * const memory = _executable.getMemory();
	Code0Segment &code0 = _executable.getCode0Segment();
	byte * const a5Base = memory + code0.getApplicationGlobalsSize();
	const byte *src = _data00->data + 4;

	// Whether data was written to the jump table
	bool dataWrittenToJumpTable = false;

	for (uint i = 0; i < 3; ++i) {
		// Read the offset
		const int32 offset = (int32)READ_UINT32_BE(src);
		src += 4;

		uint8 *dst = a5Base + offset;

		// Whether we uncompress data onto the uninitialized part of the jump table
		if (offset >= int32(code0.getApplicationParametersSize() + 8)) {
			out << "\tData write to jump table offset: " << offset << std::endl;
			dataWrittenToJumpTable = true;
		}

		while (true) {
			uint8 code = *src++;

			if (code & 0x80) {
				code &= 0x7F;
				code += 1;

				std::memcpy(dst, src, code);
				src += code;
				dst += code;
			} else if (code & 0x40) {
				code &= 0x3F;
				code += 1;

				std::memset(dst, 0, code);
				dst += code;
			} else if (code & 0x20) {
				code &= 0x1F;
				code += 2;

				const uint8 data = *src++;
				std::memset(dst, data, code);
				dst += code;
			} else if (code & 0x10) {
				code &= 0x0F;
				code += 1;

				std::memset(dst, 0xFF, code);
				dst += code;
			} else {
				if (code == 0) {
					break;
				} else if (code == 1) {
					*dst++ = 0x00;
					*dst++ = 0x00;
					*dst++ = 0x00;
					*dst++ = 0x00;
					*dst++ = 0xFF;
					*dst++ = 0xFF;
					*dst++ = *src++;
					*dst++ = *src++;
				} else if (code == 2) {
					*dst++ = 0x00;
					*dst++ = 0x00;
					*dst++ = 0x00;
					*dst++ = 0x00;
					*dst++ = 0xFF;
					*dst++ = *src++;
					*dst++ = *src++;
					*dst++ = *src++;
				} else if (code == 3) {
					*dst++ = 0xA9;
					*dst++ = 0xF0;
					*dst++ = 0x00;
					*dst++ = 0x00;
					*dst++ = *src++;
					*dst++ = *src++;
					*dst++ = 0x0;
					*dst++ = *src++;
				} else if (code == 4) {
					*dst++ = 0xA9;
					*dst++ = 0xF0;
					*dst++ = 0x00;
					*dst++ = *src++;
					*dst++ = *src++;
					*dst++ = *src++;
					*dst++ = 0x0;
					*dst++ = *src++;
				} else {
					throw std::runtime_error("DATA00 Loader: Invalid code " + boost::lexical_cast<std::string>(int(code)) + " encountered");
				}
			}
		}
	}
	
	// Check whether data was written to the jump table
	if (dataWrittenToJumpTable) {
		// Load the data from entry 1 to the last entry into our jump table structure
		for (uint i = 1, end = code0.getJumpTableEntryCount(); i < end; ++i)
			std::memcpy(code0.getJumpTableEntry(i).rawData, memory + code0.getJumpTableOffset() + i * 8, 8);

		code0.outputJumptable(out);
	}

	return src;
}

