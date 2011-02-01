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

class Code0Segment {
public:
	Code0Segment(const DataPair &pair) throw(std::exception);

	void outputHeader(std::ostream &out) throw();
	void outputJumptable(std::ostream &out) throw();
private:
	typedef std::vector<JumpTableEntry> JumpTable;
	JumpTable _jumpTable;

	uint32 _sizeAboveA5;
	uint32 _applicationGlobalsSize;
	uint32 _jumpTableSize;
	uint32 _jumpTableOffset;
};

class Executable {
public:
	Executable(const std::string &filename) throw(std::exception);

	void outputInfo(std::ostream &out) throw();
private:
	ResourceFork _resFork;
	std::auto_ptr<Code0Segment> _code0;
};

#endif

