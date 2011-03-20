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
#include "code.h"

#include <stdexcept>
#include <string>
#include <ostream>
#include <memory>
#include <map>
#include <boost/shared_ptr.hpp>

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

