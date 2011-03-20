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

#ifndef JUMPTABLE_H
#define JUMPTABLE_H

#include "util.h"

/**
 * A jump table entry.
 */
struct JumpTableEntry {
	/**
	 * The raw data of the entry.
	 */
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

#endif

