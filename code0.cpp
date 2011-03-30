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

#include "code0.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

Code0Segment::Code0Segment(const DataPair &data) throw(std::exception)
    : _jumpTable(), _sizeAboveA5(0), _applicationGlobalsSize(0), _jumpTableSize(0), _jumpTableOffset(0) {
	// A valid Code 0 segment must have at least the header + 1 jump table entry
	if (data.length < 24)
		throw std::runtime_error("CODE 0 segment contains only " + boost::lexical_cast<std::string>(data.length) + " bytes");

	// Read the header
	_sizeAboveA5 = READ_UINT32_BE(data.data + 0);
	_applicationGlobalsSize = READ_UINT32_BE(data.data + 4);
	_jumpTableSize = READ_UINT32_BE(data.data + 8);
	_jumpTableOffset = READ_UINT32_BE(data.data + 12);

	// Validate the header fields
	if (_jumpTableSize % 8 != 0)
		throw std::runtime_error("CODE 0 segment has invalid jump table size " + boost::lexical_cast<std::string>(_jumpTableSize));
	if (_sizeAboveA5 != _jumpTableSize + _jumpTableOffset)
		throw std::runtime_error("CODE 0 segment has invalid above a5 size " + boost::lexical_cast<std::string>(_sizeAboveA5) + " != " + boost::lexical_cast<std::string>(_jumpTableSize) + " + " + boost::lexical_cast<std::string>(_jumpTableOffset));
	if (getSegmentSize() % 2 != 0)
		throw std::runtime_error("CODE 0 segment has odd size " + boost::lexical_cast<std::string>(getSegmentSize()));

	// Load the jump table
	_jumpTable.resize(_jumpTableSize / 8);
	for (uint offset = 0, i = 0; offset < _jumpTableSize; offset += 8, ++i) {
		JumpTableEntry entry;
		std::memcpy(entry.rawData, data.data + 16 + offset, 8);
		_jumpTable[i] = entry;
	}
}

void Code0Segment::outputHeader(std::ostream &out) const throw() {
	out << "CODE0 header\n"
	    << "============\n"
	    << "Size above A5: " << _sizeAboveA5 << "\n"
	    << "Global data size: " << _applicationGlobalsSize << "\n"
	    << "Jump table size: " << _jumpTableSize << "\n"
	    << "Jump table offset: " << _jumpTableOffset << "\n" << std::endl;
}

void Code0Segment::outputJumptable(std::ostream &out) const throw() {
	out << "Jump table information\n"
	    << "======================\n"
	    << "Entries: " << _jumpTable.size() << "\n";

	for (uint i = 0, offset = 16, size = _jumpTable.size(); i < size; ++i, offset += 8) {
		const JumpTableEntry &entry = _jumpTable[i];

		out << "Entry " << i << ": Raw: " << boost::format("%1$02X%2$02X%3$02X%4$02X%5$02X%6$02X%7$02X%8$02X") % (uint)entry.rawData[0] % (uint)entry.rawData[1] %
		    (uint)entry.rawData[2] % (uint)entry.rawData[3] % (uint)entry.rawData[4] % (uint)entry.rawData[5] % (uint)entry.rawData[6] % (uint)entry.rawData[7] << "\n";
	}

	out << "\n" << std::flush;
}

void Code0Segment::loadIntoMemory(uint8 *memory, uint32 size) const throw(std::exception) {
	if (size < getSegmentSize())
		throw std::runtime_error("CODE segment has size " + boost::lexical_cast<std::string>(getSegmentSize()) + ", but the memory only has a size of " + boost::lexical_cast<std::string>(size));

	// Zero space for the application parameters.
	std::memset(memory + _applicationGlobalsSize, 0, _jumpTableOffset);

	// Move to jump table start
	memory += _applicationGlobalsSize + _jumpTableOffset;

	// Write the jump table to the memory dump
	BOOST_FOREACH(const JumpTableEntry &entry, _jumpTable) {
		std::memcpy(memory, entry.rawData, 8);
		memory += 8;
	}
}

