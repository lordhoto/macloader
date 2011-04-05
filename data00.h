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

#ifndef DATA00_H
#define DATA00_H

#include "staticdata.h"

/**
 * DATA00 segment loader.
 */
class Data00Loader : public StaticDataLoader {
public:
	/**
	 * Initializes a DATA00 segment loader.
	 */
	Data00Loader(Executable &exe) throw(std::exception);

	/**
	 * Destructor of a DATA00 segment loader.
	 */
	~Data00Loader();

	/**
	 * Reset the loader.
	 *
	 * This can be used to reset it's internal state, in case the loader requires
	 * a specific state before it can operate.
	 */
	virtual void reset() throw(std::exception);

	/**
	 * Query the name of the loader.
	 */
	virtual std::string getName() const { return "DATA00 loader"; }

	/**
	 * Check whether the segment is supported.
	 *
	 * @param name   Name of the segment.
	 * @param offset Offset of the segment.
	 * @param size   Size of the segment.
	 * @return true in case it is supported, false otherwise.
	 */
	virtual bool isSupported(const std::string &name, const uint32 offset, const uint32 size) throw();

	/**
	 * Load the static data.
	 *
	 * @param offset Offset of the segment which usually takes care of the loading.
	 * @param size   Size of the segment.
	 * @param out    Where to output additional loading information.
	 */
	virtual void load(const uint32 offset, const uint32 size, std::ostream &out) throw(std::exception);
private:
	/**
	 * Do the initial data uncompression.
	 */
	const byte *uncompress(const uint32 offset, std::ostream &out) throw(std::exception);

	/**
	 * The resource fork data.
	 */
	ResourceFork &_resFork;

	/**
	 * The DATA00 segment data.
	 */
	DataPair *_data00;
};

#endif

