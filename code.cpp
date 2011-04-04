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
    : _id(id), _name(name), _jumpTableOffset(0), _jumpTableEntries(0), _data(data), _segmentSize(0) {
	// A valid code segment must at least contain the header data
	if (data.length < 4)
		throw std::runtime_error("CODE segment contains only " + boost::lexical_cast<std::string>(data.length) + " bytes");

	// Read the header
	_jumpTableOffset = READ_UINT16_BE(data.data + 0);
	_jumpTableEntries = READ_UINT16_BE(data.data + 2);

	// Validate the data
	if (_jumpTableOffset % 8 != 0)
		throw std::runtime_error("CODE segment has invalid jump table offset " + boost::lexical_cast<std::string>(_jumpTableOffset));
	if (_jumpTableOffset >= code0.getJumpTableSize())
		throw std::runtime_error("CODE segment specifies offset " + boost::lexical_cast<std::string>(_jumpTableOffset) + " into jump table, but the CODE0 jump table only has size " + boost::lexical_cast<std::string>(code0.getJumpTableSize()));
	if ((uint32)(_jumpTableOffset + _jumpTableEntries * 8) > code0.getJumpTableSize())
		throw std::runtime_error("CODE segment specifies " + boost::lexical_cast<std::string>(_jumpTableEntries) + " entries but the CODE0 jump table only contains " + boost::lexical_cast<std::string>((code0.getJumpTableSize() - _jumpTableOffset) / 8) + " entries after the jump table entry offset");

	// Fix segment size in case it's odd
	_segmentSize = _data.length + (_data.length & 1);
}

void CodeSegment::outputHeader(std::ostream &out) const throw() {
	out << "CODE" << _id << " \"" << _name << "\" header\n"
	    << "Real segment size: " << _data.length << "\n"
	    << "Loaded segment size: " << _segmentSize << "\n"
	    << "===========\n"
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

