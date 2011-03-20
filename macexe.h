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
#include "jumptable.h"
#include "code0.h"

#include <stdexcept>
#include <string>
#include <ostream>
#include <memory>
#include <map>
#include <boost/shared_ptr.hpp>

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
	 * Query the segment name.
	 */
	const std::string &getName() const { return _name; }

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
	 * @param out Where to output misc loading information.
	 * @throws std::exception Errors on dumping.
	 */
	void writeMemoryDump(const std::string &filename, std::ostream &out) throw(std::exception);
private:
	/**
	 * Load the executable into memory.
	 *
	 * @param out Where to output misc loading information.
	 */
	void loadIntoMemory(std::ostream &out) throw(std::exception);

	/**
	 * Uncompresses the a5 world from the %A5Init segment.
	 *
	 * @param offset The offset where the %A5Init segment starts.
	 * @param out Where to output misc loading information.
	 */
	void uncompressA5World(uint32 offset, std::ostream &out) throw(std::exception);

	/**
	 * Do the real world uncompression.
	 *
	 * @param dst Where to store the data.
	 * @param src Where the compressed data lies.
	 */
	void uncompressA5World(uint8 *dst, const uint8 *src) throw();

	/**
	 * Get the run length from the given address.
	 *
	 * @param src Where to read from.
	 * @param special Special repeat counter.
	 * @return The decoded run length.
	 */
	uint32 getRunLength(const uint8 *&src, uint32 &special) throw();

	/**
	 * Relocate the world data.
	 *
	 * @param a5 A5 base offset.
	 * @param dst Destination start.
	 * @param src Where the relocation data lies.
	 * @param out Where to output misc loading information.
	 */
	void relocateWorld(const uint32 a5, uint8 *dst, const uint8 *src, std::ostream &out) throw();

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

