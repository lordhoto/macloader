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

// Forward from staticdata.h
class StaticDataLoaderManager;

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

	/**
	 * Query the resource fork.
	 */
	ResourceFork &getResourceFork() { return _resFork; }

	/**
	 * Query the CODE0 segment.
	 */
	Code0Segment &getCode0Segment() { return *_code0; }

	/**
	 * Query the CODE0 segment.
	 */
	const Code0Segment &getCode0Segment() const { return *_code0; }

	/**
	 * Query the memory dump.
	 */
	byte *getMemory() { return _memory; }

	/**
	 * Query the memory dump size.
	 */
	uint32 getMemorySize() const { return _memorySize; }
private:
	/**
	 * Load the executable into memory.
	 *
	 * @param out Where to output misc loading information.
	 */
	void loadIntoMemory(std::ostream &out) throw(std::exception);

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

	/**
	 * The static data loader manager.
	 */
	StaticDataLoaderManager *_loaderManager;
};

#endif

