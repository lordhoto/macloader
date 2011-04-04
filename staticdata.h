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

#ifndef STATICDATA_H
#define STATICDATA_H

#include "macexe.h"
#include "util.h"

#include <ostream>
#include <string>
#include <list>

/**
 * A static data loader.
 *
 * This interface is used to load static data required by the executable, like
 * the global variables or parts of the jump table in case the CODE 0 segment
 * does not contain a fully initialized jump table.
 */
class StaticDataLoader {
public:
	/**
	 * Create a static data loader.
	 *
	 * @param exe The executable to load.
	 */
	StaticDataLoader(Executable &exe) throw(std::exception) : _executable(exe) {}

	/**
	 * Destructor of the static data loader object.
	 */
	virtual ~StaticDataLoader() {}

	/**
	 * Query the name of the loader.
	 */
	virtual std::string getName() const = 0;

	/**
	 * Reset the loader.
	 *
	 * This can be used to reset it's internal state, in case the loader requires
	 * a specific state before it can operate.
	 */
	virtual void reset() throw(std::exception) {}

	/**
	 * Check whether the segment is supported.
	 *
	 * @param name   Name of the segment.
	 * @param offset Offset of the segment.
	 * @param size   Size of the segment.
	 * @return true in case it is supported, false otherwise.
	 */
	virtual bool isSupported(const std::string &name, const uint32 offset, const uint32 size) throw() = 0;

	/**
	 * Load the static data.
	 *
	 * @param offset Offset of the segment which usually takes care of the loading.
	 * @param size   Size of the segment.
	 * @param out    Where to output additional loading information.
	 */
	virtual void load(const uint32 offset, const uint32 size, std::ostream &out) throw(std::exception) = 0;

protected:
	/**
	 * The executable to load.
	 */
	Executable &_executable;
};

/**
 * A static data loader handler.
 */
class StaticDataLoaderManager {
public:
	/**
	 * Initializes the static data loader manager.
	 *
	 * @param exe The executable to load.
	 */
	StaticDataLoaderManager(Executable &exe) throw(std::exception);

	/**
	 * Destructor of the static data loader manager.
	 */
	~StaticDataLoaderManager();

	/**
	 * Try to load from a specific segment.
	 *
	 * @param name   Name of the segment.
	 * @param offset Offset of the segment.
	 * @param size   Size of the segment.
	 * @param out    Where to output additional loading information.
	 * @return true in case some loading happened, false otherwise.
	 */
	bool loadFromSegment(const std::string &name, const uint32 offset, const uint32 size, std::ostream &out) throw(std::exception);
private:
	typedef std::list<StaticDataLoader *> StaticDataLoaderContainer;

	/**
	 * All the available loaders.
	 */
	StaticDataLoaderContainer _loaders;
};

#endif

