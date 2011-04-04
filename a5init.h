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

#ifndef A5INIT_H
#define A5INIT_H

#include "staticdata.h"

/**
 * A loader for the %A5Init segment.
 */
class A5InitLoader : public StaticDataLoader {
public:
	/**
	 * Initialize the A5Init segment loader.
	 */
	A5InitLoader(Executable &exe) : StaticDataLoader(exe) {}

	/**
	 * Query the name of the loader.
	 */
	virtual std::string getName() const { return "%A5Init loader"; }

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
	 * Do the real world uncompression.
	 *
	 * @param dst Where to store the data.
	 * @param src Where the compressed data lies.
	 */
	void uncompressA5World(uint8 *dst, const uint8 *src) const throw();

	/**
	 * Get the run length from the given address.
	 *
	 * @param src Where to read from.
	 * @param special Special repeat counter.
	 * @return The decoded run length.
	 */
	uint32 getRunLength(const uint8 *&src, uint32 &special) const throw();

	/**
	 * Relocate the world data.
	 *
	 * @param a5 A5 base offset.
	 * @param dst Destination start.
	 * @param src Where the relocation data lies.
	 * @param out Where to output misc loading information.
	 */
	void relocateWorld(const uint32 a5, uint8 *dst, const uint8 *src, std::ostream &out) const throw();
};

#endif

