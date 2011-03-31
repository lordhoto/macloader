//
// Copyright (c) 2011 Johannes Schickel (LordHoto)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//

#include <idc.idc>

static main() {
	auto address;

	// Iterate through all functions
	for (address = NextFunction(0); address != -1; address = NextFunction(address)) {
		// Determine end of function
		auto functionEnd = FindFuncEnd(address);

		// Peek at next function
		auto nextFunc = NextFunction(address);

		// Check whether the addresses match
		if (nextFunc == functionEnd)
			continue;

		// If not we look for a debug function name

		auto byte = Byte(functionEnd);

		// If the high bit is not set, there is no debug function name
		if (!(byte & 0x80))
			continue;

		auto size;

		// Check whether the size byte is included in the first byte or not
		if (byte == 0x80) {
			// Size is the next byte
			size = Byte(functionEnd + 1);
			// Skip the size byte
			functionEnd = functionEnd + 1;
		} else {
			// Size is the lower 7 bits
			size = byte & 0x7F;
		}

		auto name = "";

		// Move to start of the name
		functionEnd = functionEnd + 1;

		// Read the name
		auto i;
		for (i = 0; i < size; ++i)
			name = sprintf("%s%c", name, Byte(functionEnd + i));

		// Set the function name
		MakeNameEx(address, name, SN_CHECK | SN_PUBLIC);
	}
}
