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
#include "staticdata.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>

const uint32 kCodeTag = 0x434F4445;

Executable::Executable(const std::string &filename) throw(std::exception)
    : _resFork(), _code0(), _codeSegments(), _memory(nullptr), _memorySize(0), _loaderManager(nullptr) {
	_loaderManager = new StaticDataLoaderManager(*this);

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
	destroy(_loaderManager);
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

void Executable::writeMemoryDump(const std::string &filename, std::ostream &outInfo) throw(std::exception) {
	// Load the executable
	loadIntoMemory(outInfo);

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

void Executable::loadIntoMemory(std::ostream &out) throw(std::exception) {
	// Allocate enough memory for the executable
	delete[] _memory;
	_memorySize = _code0->getSegmentSize() + _codeSegmentsSize;
	_memory = new uint8[_memorySize];
	std::memset(_memory, 0, _memorySize);

	// The current offset in the memory dump
	uint32 offset = _code0->getSegmentSize();

	// Output the a5 base address
	out << boost::format("A5 base is at 0x%1$08X\n") % _code0->getApplicationGlobalsSize()
	    << boost::format("Jump table starts at 0x%1$08X\n") % _code0->getJumpTableOffset()
	    << boost::format("Number of jump table entries %1$d\n") % _code0->getJumpTableEntryCount();

	// Load all the segments
	BOOST_FOREACH(const CodeSegmentMap::value_type &i, _codeSegments) {
		// Load the segment
		i.second->loadIntoMemory(*_code0, _memory, offset, _memorySize);

		// Output information about the segment
		out << boost::format("Segment %1$d \"%2$s\" starts at offset 0x%3$08X\n") % i.first % i.second->getName() % offset;

		// Try to load static data from the segment
		_loaderManager->loadFromSegment(*i.second, offset, i.second->getSegmentSize(), out);

		// Adjust offset for the next entry
		offset += i.second->getSegmentSize();
	}

	// Finally load the CODE0 segment
	_code0->loadIntoMemory(_memory, _memorySize);
}

