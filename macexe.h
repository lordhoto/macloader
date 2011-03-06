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

#ifndef MACEXE_H
#define MACEXE_H

#include "macresfork.h"

#include <stdexcept>
#include <string>
#include <ostream>
#include <vector>
#include <memory>
#include <map>
#include <boost/shared_ptr.hpp>

struct JumpTableEntry {
	byte rawData[8];

	/**
	 * Check whether the referenced segment is load or not.
	 */
	bool isLoaded() const { return READ_UINT16_BE(rawData + 6) != 0xA9F0; }

	/**
	 * Query the referenced segment ID of the entry.
	 *
	 * This only returns a correct segment ID, when the entry is not yet load.
	 */
	uint16 getSegmentID() const { return READ_UINT16_BE(rawData + 4); }

	/**
	 * Adjust the entry on segment load.
	 *
	 * @param offset The base offset of the segment.
	 */
	void load(uint32 offset);
};

/**
 * The Code 0 segment of a Macintosh m68k executable.
 */
class Code0Segment {
public:
	/**
	 * Load a Code 0 segment.
	 *
	 * @param pair The resource data to load from.
	 * @throws std::exception Errors on loading.
	 */
	Code0Segment(const DataPair &pair) throw(std::exception);

	/**
	 * Output information about the segment header.
	 *
	 * @param out The stream to output to.
	 */
	void outputHeader(std::ostream &out) const throw();

	/**
	 * Output information about the jump table.
	 *
	 * @param out The stream to output to.
	 */
	void outputJumptable(std::ostream &out) const throw();

	/**
	 * Write the segment into memory.
	 *
	 * This will first make zeroed space for the global variables, then make
	 * space for the application parameters and last but not least write the
	 * jump table.
	 *
	 * @param memory Where to write to.
	 * @param size Size of the memory.
	 */
	void loadIntoMemory(uint8 *memory, uint32 size) const throw(std::exception);

	/**
	 * Query the size of the jump table.
	 */
	uint32 getJumpTableSize() const { return _jumpTableSize; }

	/**
	 * Query the size of the globals.
	 */
	uint32 getApplicationGlobalsSize() const { return _applicationGlobalsSize; }

	/**
	 * Query the application parameter size.
	 */
	uint32 getApplicationParametersSize() const { return _jumpTableOffset; }

	/**
	 * Query the size of the whole segment.
	 */
	uint32 getSegmentSize() const {
		return _jumpTableSize + _applicationGlobalsSize + _jumpTableOffset;
	}

	/**
	 * Query a jump table entry.
	 */
	JumpTableEntry &getJumpTableEntry(int entry) throw(std::exception) {
		return _jumpTable.at(entry);
	}
private:
	/**
	 * The jump table vector.
	 */
	typedef std::vector<JumpTableEntry> JumpTable;

	/**
	 * The jump table of the segment.
	 */
	JumpTable _jumpTable;

	/**
	 * Size above the A5 offset in bytes.
	 * This should equal _jumpTableSize + _jumpTableOffset.
	 * @see _jumpTableSize
	 * @see _jumpTableOffset
	 */
	uint32 _sizeAboveA5;

	/**
	 * The size of the executable's globale variables.
	 */
	uint32 _applicationGlobalsSize;

	/**
	 * The raw size of the jump table.
	 */
	uint32 _jumpTableSize;

	/**
	 * The offset from a5 of the jump table.
	 */
	uint32 _jumpTableOffset;
};

/**
 * Any code segement different to CODE 0
 */
class CodeSegment {
public:
	/**
	 * Load a Code segment.
	 *
	 * @param code0 The code 0 segement.
	 * @param id The id of the code segment.
	 * @param name The name of the code segment.
	 * @param pair The resource data to load from.
	 * @throws std::exception Errors on loading.
	 */
	CodeSegment(const Code0Segment &code0, const uint id, const std::string &name, const DataPair &pair) throw(std::exception);

	/**
	 * Output information about the segment header.
	 *
	 * @param out The stream to output to.
	 */
	void outputHeader(std::ostream &out) const throw();

	/**
	 * Query the size of the whole segment.
	 */
	uint32 getSegmentSize() const { return _data.length; }

	/**
	 * Write the segment into memory.
	 *
	 * @param code0 CODE0 Segement containing the jump table.
	 * @param memory Where to write to.
	 * @param offset The offset into the memory.
	 * @param size Size of the memory.
	 */
	void loadIntoMemory(Code0Segment &code0, uint8 *memory, uint32 offset, uint32 size) const throw(std::exception);
private:
	/**
	 * The id of the segment.
	 */
	const uint _id;

	/**
	 * The name of the segment.
	 */
	const std::string _name;

	/**
	 * Offset into the jump table.
	 */
	uint16 _jumpTableOffset;

	/**
	 * Number of exported functions in the jump table.
	 */
	uint16 _jumpTableEntries;

	/**
	 * The segment data.
	 */
	DataPair _data;
};

/**
 * Object representing a Macintosh m68k executable.
 */
class Executable {
public:
	/**
	 * Initial load of an executable from a file.
	 *
	 * @param filename The file where to load from.
	 * @throws std::exception Errors on loading.
	 */
	Executable(const std::string &filename) throw(std::exception);

	/**
	 * Destructor of the Executable object.
	 */
	~Executable();

	/**
	 * Output information about the loaded file.
	 *
	 * @param out The stream where to output to.
	 */
	void outputInfo(std::ostream &out) const throw();

	/**
	 * Output a memory dump of the executable to the given file.
	 *
	 * @param filename The file to save the dump to.
	 * @throws std::exception Errors on dumping.
	 */
	void writeMemoryDump(const std::string &filename) throw(std::exception);
private:
	/**
	 * Load the executable into memory.
	 */
	void loadIntoMemory() throw(std::exception);

	/**
	 * The resource fork data.
	 */
	ResourceFork _resFork;

	/**
	 * The Code 0 segment.
	 */
	std::auto_ptr<Code0Segment> _code0;

	/**
	 * The segment container.
	 */
	typedef std::map<uint16, boost::shared_ptr<CodeSegment> > CodeSegmentMap;

	/**
	 * ALl the other code segments.
	 */
	CodeSegmentMap _codeSegments;

	/**
	 * The size of all code segments.
	 */
	uint32 _codeSegmentsSize;

	/**
	 * The memory dump.
	 */
	uint8 *_memory;

	/**
	 * Size of the memory dump.
	 */
	uint32 _memorySize;
};

#endif

