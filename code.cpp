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

#include "code.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

CodeSegment::CodeSegment(const Code0Segment &code0, const uint id, const std::string &name, const DataPair &data) throw(std::exception)
    : _id(id), _name(name), _jumpTableOffset(0), _jumpTableEntries(0), _data(data), _segmentSize(0), _is32BitSegment(false) {
	// A valid code segment must at least contain the header data
	if (data.length < 4)
		throw std::runtime_error("CODE segment contains only " + boost::lexical_cast<std::string>(data.length) + " bytes");

	// Read the header
	_jumpTableOffset = READ_UINT16_BE(data.data + 0);
	_jumpTableEntries = READ_UINT16_BE(data.data + 2);

	// Check whether it's a special 32bit segment
	_is32BitSegment = (_jumpTableOffset == 0xFFFF && _jumpTableEntries == 0x0000);

	// Validate the data
	if (_is32BitSegment) {
		// Validate the first jump table hunk
		const uint32 jumpTableOffset1  = READ_UINT32_BE(data.data +  4);
		const uint32 jumpTableEntries1 = READ_UINT32_BE(data.data +  8);

		if (jumpTableOffset1 % 8 != 0)
			throw std::runtime_error("CODE32 segment has invalid first jump table offset " + boost::lexical_cast<std::string>(jumpTableOffset1));
		if ((uint32)(jumpTableOffset1 + jumpTableEntries1 * 8) > code0.getJumpTableSize())
			throw std::runtime_error("CODE32 segment specifies " + boost::lexical_cast<std::string>(jumpTableEntries1) + " entries in the first hunk but the CODE0 jump table only contains " + boost::lexical_cast<std::string>((code0.getJumpTableSize() - jumpTableOffset1) / 8) + " entries after the jump table entry offset");

		// Validate the second jump table hunk
		const uint32 jumpTableOffset2  = READ_UINT32_BE(data.data + 12);
		const uint32 jumpTableEntries2 = READ_UINT32_BE(data.data + 16);

		if (jumpTableOffset2 % 8 != 0)
			throw std::runtime_error("CODE32 segment has invalid second jump table offset " + boost::lexical_cast<std::string>(jumpTableOffset1));
		if ((uint32)(jumpTableOffset2 + jumpTableEntries2 * 8) > code0.getJumpTableSize())
			throw std::runtime_error("CODE32 segment specifies " + boost::lexical_cast<std::string>(jumpTableEntries2) + " entries in the second hunk but the CODE0 jump table only contains " + boost::lexical_cast<std::string>((code0.getJumpTableSize() - jumpTableOffset2) / 8) + " entries after the jump table entry offset");

		// Validate the global relocation data
		const uint32 relocationDataOffset1 = READ_UINT32_BE(data.data + 20);
		const uint32 relocationOffset1     = READ_UINT32_BE(data.data + 24);

		if (relocationDataOffset1 != 0 && relocationDataOffset1 + 2 > data.length)
			throw std::runtime_error("CODE32 segment has invalid global relocation data offset " + boost::lexical_cast<std::string>(relocationDataOffset1));
		if (relocationOffset1 != 0)
			throw std::runtime_error("CODE32 segment has invalid global relocation offset " + boost::lexical_cast<std::string>(relocationOffset1));

		// Validate segment relocation data
		const uint32 relocationDataOffset2 = READ_UINT32_BE(data.data + 28);
		const uint32 relocationOffset2     = READ_UINT32_BE(data.data + 32);

		if (relocationDataOffset2 != 0 && relocationDataOffset2 + 2 > data.length)
			throw std::runtime_error("CODE32 segment has invalid segment relocation data offset " + boost::lexical_cast<std::string>(relocationDataOffset2));
		if (relocationOffset2 != 0)
			throw std::runtime_error("CODE32 segment has invalid segment relocation offset " + boost::lexical_cast<std::string>(relocationOffset2));
	} else {
		if (_jumpTableOffset % 8 != 0)
			throw std::runtime_error("CODE segment has invalid jump table offset " + boost::lexical_cast<std::string>(_jumpTableOffset));
		if (_jumpTableOffset >= code0.getJumpTableSize())
			throw std::runtime_error("CODE segment specifies offset " + boost::lexical_cast<std::string>(_jumpTableOffset) + " into jump table, but the CODE0 jump table only has size " + boost::lexical_cast<std::string>(code0.getJumpTableSize()));
		if ((uint32)(_jumpTableOffset + _jumpTableEntries * 8) > code0.getJumpTableSize())
			throw std::runtime_error("CODE segment specifies " + boost::lexical_cast<std::string>(_jumpTableEntries) + " entries but the CODE0 jump table only contains " + boost::lexical_cast<std::string>((code0.getJumpTableSize() - _jumpTableOffset) / 8) + " entries after the jump table entry offset");
	}

	// Fix segment size in case it's odd
	_segmentSize = _data.length + (_data.length & 1);
}

void CodeSegment::outputHeader(std::ostream &out) const throw() {
	out << "CODE" << _id << " \"" << _name << "\" header\n"
	    << "Real segment size: " << _data.length << "\n"
	    << "Loaded segment size: " << _segmentSize << "\n"
	    << "===========\n"
	    << "Is 32bit segment: " << (_is32BitSegment ? "yes" : "no") << "\n"
	    << "Offset to first entry in jump table: " << _jumpTableOffset << "\n"
	    << "Number of exported functions: " << _jumpTableEntries << "\n" << std::endl;
}

void CodeSegment::loadIntoMemory(Code0Segment &code0, uint8 *memory, uint32 offset, uint32 size) const throw(std::exception) {
	if (size - offset < getSegmentSize())
		throw std::runtime_error("CODE segment has size " + boost::lexical_cast<std::string>(getSegmentSize()) + ", but the memory only has a size of " + boost::lexical_cast<std::string>(size));

	// Write the segment data to the memory
	std::memcpy(memory + offset, _data.data, _data.length);

	// Add a padding zero in case we have an odd segment size
	assert(_segmentSize >= _data.length);
	assert(_segmentSize <= _data.length + 1);
	if (_segmentSize > _data.length)
		memory[offset + _data.length] = 0;

	if (_is32BitSegment)
		initialize32Bit(code0, memory, offset, size);
	else
		initialize(code0, memory, offset, size);
}

void CodeSegment::initialize(Code0Segment &code0, uint8 *memory, uint32 offset, uint32 size) const throw(std::exception) {
	// Adjust the jump table
	for (uint i = 0; i < _jumpTableEntries; ++i) {
		const uint entryNum = i + _jumpTableOffset / 8;

		try {
			JumpTableEntry &entry = code0.getJumpTableEntry(entryNum);

			// Check whether the entry is loaded already
			if (entry.isLoaded())
				throw std::runtime_error("Jump table entry " + boost::lexical_cast<std::string>(entryNum) + " is loaded already");

			// Check whether we are the segment the entry references
			if (entry.getSegmentID() != _id)
				throw std::runtime_error("Jump table entry " + boost::lexical_cast<std::string>(entryNum) + " references segment " + boost::lexical_cast<std::string>(entry.getSegmentID()) + " and not segment " + boost::lexical_cast<std::string>(_id));

			// Adjust the entry, we add 4 here, since we also copy the CODE segment header into the dump
			entry.load(offset + 4);
		} catch (std::exception &exception) {
			throw std::runtime_error(std::string("CODE segment could not load: ") + exception.what());
		}
	}
}

void CodeSegment::initialize32Bit(Code0Segment &code0, uint8 *memory, uint32 offset, uint32 size) const throw(std::exception) {
	// Adjust the jump table
	initJumpTableBlock32Bit(code0, READ_UINT32_BE(memory + offset +  4), READ_UINT32_BE(memory + offset +  8), offset);
	initJumpTableBlock32Bit(code0, READ_UINT32_BE(memory + offset + 12), READ_UINT32_BE(memory + offset + 16), offset);

	// Do the global relocation
	const int32 relOffset1 = code0.getApplicationGlobalsSize() - (int32)READ_UINT32_BE(memory + offset + 24);
	const uint32 relDataOffset1 = READ_UINT32_BE(memory + offset + 20);

	if (relOffset1 && relDataOffset1)
		relocate32Bit(memory + offset, memory + offset + relDataOffset1, relOffset1);

	// Do the segment relocation
	int32 relOffset2 = READ_UINT32_BE(memory + offset + 32);

	if (relOffset2 == 0)
		relOffset2 = offset + 40;
	else
		relOffset2 = offset - relOffset2;

	const uint32 relDataOffset2 = READ_UINT32_BE(memory + offset + 28);

	if (relOffset2 && relDataOffset2)
		relocate32Bit(memory + offset, memory + offset + relDataOffset2, relOffset2);
}

void CodeSegment::initJumpTableBlock32Bit(Code0Segment &code0, uint32 startOffset, uint32 count, uint32 offset) const throw(std::exception) {
	// Check whether we actually process anything
	if (!count)
		return;

	for (uint i = 0; i < count; ++i) {
		const uint entryNum = i + startOffset / 8;

		try {
			JumpTableEntry &entry = code0.getJumpTableEntry(entryNum);

			// Check whether the entry is loaded already
			if (entry.isLoaded32Bit())
				throw std::runtime_error("Jump table entry " + boost::lexical_cast<std::string>(entryNum) + " is loaded already");

			// Check whether we are the segment the entry references
			if (entry.getSegmentID32Bit() != _id)
				throw std::runtime_error("Jump table entry " + boost::lexical_cast<std::string>(entryNum) + " references segment " + boost::lexical_cast<std::string>(entry.getSegmentID()) + " and not segment " + boost::lexical_cast<std::string>(_id));

			// Adjust the entry
			entry.load32Bit(offset);
		} catch (std::exception &exception) {
			throw std::runtime_error(std::string("CODE0 32bit segment could not load: ") + exception.what());
		}
	}
}

void CodeSegment::relocate32Bit(uint8 *memory, const uint8 *src, int32 offset) const {
	while (true) {
		int32 off = 0;
		off = *src++;

		if (off == 0) {
			if (!*src)
				return;

			off = READ_UINT32_BE(src);
			src += 4;
		} else if (off & 0x80) {
			off &= 0x7F;
			off <<= 8;
			off |= *src++;
		}

		off += off;
		memory += off;
		WRITE_UINT32_BE(memory, (int32)READ_UINT32_BE(memory) + offset);
	}
}

