/**
 * Copyright (c) 2011 Johannes Schickel (LordHoto)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

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

struct JumpTableEntry {
	byte rawData[8];
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
	void outputHeader(std::ostream &out) throw();

	/**
	 * Output information about the jump table.
	 *
	 * @param out The stream to output to.
	 */
	void outputJumptable(std::ostream &out) throw();
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
	 * @param pair The resource data to load from.
	 * @throws std::exception Errors on loading.
	 */
	CodeSegment(const Code0Segment &code0, const DataPair &pair) throw(std::exception);

	/**
	 * Output information about the segment header.
	 *
	 * @param out The stream to output to.
	 */
	void outputHeader(std::ostream &out) throw();
private:
	/**
	 * Offset into the jump table.
	 */
	uint16 _jumpTableOffset;

	/**
	 * Number of exported functions in the jump table.
	 */
	uint16 _jumpTableEntries;
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
	 * Output information about the loaded file.
	 *
	 * @param out The stream where to output to.
	 */
	void outputInfo(std::ostream &out) throw();
private:
	/**
	 * The resource fork data.
	 */
	ResourceFork _resFork;

	/**
	 * The Code 0 segment.
	 */
	std::auto_ptr<Code0Segment> _code0;
};

#endif

