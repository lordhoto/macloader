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
	auto a5Base, address;

	a5Base = LocByName("sys_a5Base");

	address = 0;

	while (1) {
		// Move on to the next opcode
		address = FindCode(address, SEARCH_DOWN);
		if (address == BADADDR)
			break;

		// We only handle jsr opcodes, we might consider handling plain jump
		// opcodes in the future too.
		if (GetMnem(address) != "jsr")
			continue;

		// Check wether the target is base + index + displacement parameter
		if (GetOpType(address, 0) != 4)
			continue;

		// Decode the instruction
		auto instruction = DecodeInstruction(address);

		// Check whether a5 is used
		if (instruction.Op0.reg != 13)
			continue;

		// Get offset
		auto offset = instruction.Op0.addr;

		// Get the destination offset
		auto destOffset = Dword(a5Base + offset + 2);

		// Add a reference to the destination function
		AddCodeXref(address, destOffset, fl_CF | XREF_USER);

		// Add a comment where the instruction jumps to
		MakeComm(address, sprintf("call address $%X", destOffset));

		// Convert the operand
		OpOffEx(address, 0, REF_OFF32, -1, destOffset, instruction.Op0.addr);
	}
}

