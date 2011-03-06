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

#include "macexe.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

const uint32 kCodeTag = 0x434F4445;

Executable::Executable(const std::string &filename) throw(std::exception)
    : _resFork(), _code0(), _codeSegments(), _memory(nullptr), _memorySize(0) {
	// Try to load the resource fork of the given file
	if (!_resFork.load(filename.c_str()))
		throw std::runtime_error("Could not load file " + filename);

	// Initialize the Code 0 segment
	DataPair *data = _resFork.getResource(kCodeTag, 0);
	// In case no Code 0 segment is present it is definitly no valid executable
	if (data == nullptr)
		throw std::runtime_error("File " + filename + " does not contain any CODE 0 segment");

	try {
		_code0 = std::auto_ptr<Code0Segment>(new Code0Segment(*data));
	} catch (std::exception &e) {
		destroy(data);
		throw;
	}

	destroy(data);

	// Load all other segments
	std::vector<uint16> idArray = _resFork.getIDArray(kCodeTag);
	_codeSegmentsSize = 0;

	BOOST_FOREACH(uint16 id, idArray) {
		// Segment 0 is loaded already, thus skip it
		if (id == 0)
			continue;

		DataPair *data = _resFork.getResource(kCodeTag, id);
		if (data == nullptr)
			throw std::runtime_error("Failed to load CODE segment " + boost::lexical_cast<std::string>(id));

		try {
			boost::shared_ptr<CodeSegment> seg = _codeSegments[id] = boost::make_shared<CodeSegment>(*_code0, id, _resFork.getFilename(kCodeTag, id), *data);
			_codeSegmentsSize += seg->getSegmentSize();
		} catch (std::exception &e) {
			destroy(data);
			throw std::runtime_error("CODE segment " + boost::lexical_cast<std::string>(id) + " loading error: " + e.what());
		}

		destroy(data);
	}
}

Executable::~Executable() {
	delete[] _memory;
	_memory = nullptr;
}

void Executable::outputInfo(std::ostream &out) const throw() {
	assert(_code0.get() != nullptr);
	_code0->outputHeader(out);
	_code0->outputJumptable(out);

	BOOST_FOREACH(const CodeSegmentMap::value_type &i, _codeSegments)
		i.second->outputHeader(out);
}

void Executable::writeMemoryDump(const std::string &filename) throw(std::exception) {
	// Load the executable
	loadIntoMemory();

	std::ofstream out(filename.c_str(), std::ios::out | std::ios::binary);
	if (!out)
		throw std::runtime_error("Could not open file " + filename + " for writing");
	out.write(reinterpret_cast<const char *>(_memory), _memorySize);
	out.flush();
	out.close();

	// Free the memory
	delete[] _memory;
	_memory = nullptr;
}

void Executable::loadIntoMemory() throw(std::exception) {
	// Allocate enough memory for the executable
	delete[] _memory;
	_memorySize = _code0->getSegmentSize() + _codeSegmentsSize;
	_memory = new uint8[_memorySize];

	// The current offset in the memory dump
	uint32 offset = _code0->getSegmentSize();

	// Load all the segments
	BOOST_FOREACH(const CodeSegmentMap::value_type &i, _codeSegments) {
		// Load the segment
		i.second->loadIntoMemory(*_code0, _memory, offset, _memorySize);

		// Adjust offset for the next entry
		offset += i.second->getSegmentSize();
	}

	// Finally load the CODE0 segment
	_code0->loadIntoMemory(_memory, _memorySize);
}

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
		throw std::runtime_error("CODE 0 segment has invalid above a5 size " + boost::lexical_cast<std::string>(_sizeAboveA5));

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

	// Zero space for the application globals and the application parameters.
	std::memset(memory, 0, _applicationGlobalsSize);
	std::memset(memory + _applicationGlobalsSize, 0, _jumpTableOffset);

	// Move to jump table start
	memory += _applicationGlobalsSize + _jumpTableOffset;

	// Write the jump table to the memory dump
	BOOST_FOREACH(const JumpTableEntry &entry, _jumpTable) {
		std::memcpy(memory, entry.rawData, 8);
		memory += 8;
	}
}

CodeSegment::CodeSegment(const Code0Segment &code0, const uint id, const std::string &name, const DataPair &data) throw(std::exception)
    : _id(id), _name(name), _jumpTableOffset(0), _jumpTableEntries(0), _data(data) {
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
}

void CodeSegment::outputHeader(std::ostream &out) const throw() {
	out << "CODE" << _id << " \"" << _name << "\" header\n"
	    << "Segment size: " << _data.length << "\n"
	    << "===========\n"
	    << "Offset to first entry in jump table: " << _jumpTableOffset << "\n"
	    << "Number of exported functions: " << _jumpTableEntries << "\n" << std::endl;
}

void CodeSegment::loadIntoMemory(Code0Segment &code0, uint8 *memory, uint32 offset, uint32 size) const throw(std::exception) {
	if (size - offset < getSegmentSize())
		throw std::runtime_error("CODE segment has size " + boost::lexical_cast<std::string>(getSegmentSize()) + ", but the memory only has a size of " + boost::lexical_cast<std::string>(size));

	// Write the segment data to the memory
	std::memcpy(memory + offset, _data.data, _data.length);

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

void JumpTableEntry::load(uint32 offset) {
	// In case the segment is load already we ignore the request
	if (isLoaded())
		return;

	// Read the function offset
	const uint16 functionOffset = READ_UINT16_BE(rawData + 0);

	// Caclulate the real offset
	offset += functionOffset;

	// Write the JMP instruction
	WRITE_UINT16_BE(rawData + 2, 0x4EF9);

	// Write the offset
	WRITE_UINT32_BE(rawData + 4, offset);
}

