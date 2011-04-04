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

#ifndef CODE0_H
#define CODE0_H

#include "macresfork.h"
#include "jumptable.h"

#include <stdexcept>
#include <vector>
#include <ostream>

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
	uint32 getJumpTableSize() const { return _jumpTable.size() * 8; }

	/**
	 * Query the number of jump table entries.
	 */
	uint32 getJumpTableEntryCount() const { return _jumpTable.size(); }

	/**
	 * Query the size of the globals.
	 */
	uint32 getApplicationGlobalsSize() const { return _applicationGlobalsSize; }

	/**
	 * Query the application parameter size.
	 */
	uint32 getApplicationParametersSize() const { return _jumpTableOffset; }

	/**
	 * Query the offset of the jump table in the output dump.
	 */
	uint32 getJumpTableOffset() const { return _applicationGlobalsSize + _jumpTableOffset; }

	/**
	 * Query the size of the whole segment.
	 */
	uint32 getSegmentSize() const {
		return getJumpTableSize() + _applicationGlobalsSize + _jumpTableOffset;
	}

	/**
	 * Query a jump table entry.
	 */
	JumpTableEntry &getJumpTableEntry(int entry) throw(std::exception) {
		return _jumpTable.at(entry);
	}

	/**
	 * Query whether the jump table is partly uninitialized.
	 */
	bool isJumpTableUninitialized() const {
		return _onlyFirstJumpTableEntryInitialized;
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

	/**
	 * Whether the jump table is partly uninitialized.
	 */
	bool _onlyFirstJumpTableEntryInitialized;
};

#endif

