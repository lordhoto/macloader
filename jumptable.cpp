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

#include "jumptable.h"

bool JumpTableEntry::isDummy() const {
	return rawData[0] == 0
	    && rawData[1] == 0
	    && rawData[2] == 0
	    && rawData[3] == 0
	    && rawData[4] == 0
	    && rawData[5] == 0
	    && rawData[6] == 0
	    && rawData[7] == 0;
}

void JumpTableEntry::load(uint32 offset) {
	// In case the segment is load already we ignore the request
	if (isLoaded())
		return;

	// Read the function offset
	const uint16 functionOffset = READ_UINT16_BE(rawData + 0);

	// Caclulate the real offset
	offset += functionOffset;

	// Write the JMP instruction
	WRITE_UINT16_BE(rawData + 2, 0x4EF9);

	// Write the offset
	WRITE_UINT32_BE(rawData + 4, offset);
}

