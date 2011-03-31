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

static fixOperand(address, n, op, a5Base) {
	// Check whether the operand is base + index + displacement parameter
	if (op.type == 4) {
		// Check whether it uses a5 as base
		if (op.reg == 13) {
			// Mark the offset properly
			OpOff(address, n, a5Base);
		}
	}
}

static main() {
	auto a5Base, address;

	a5Base = LocByName("sys_a5Base");

	address = 0;

	while (1) {
		// Move on to the next opcode
		address = FindCode(address, SEARCH_DOWN);
		if (address == BADADDR)
			break;

		// Ignore all jsr opcodes, since those are handled by a different script
		if (GetMnem(address) == "jsr")
			continue;

		// Decode the instruction
		auto instruction = DecodeInstruction(address);
		if (!instruction)
			continue;

		// Fix up all operands if required
		if (instruction.n >= 1)
			fixOperand(address, 0, instruction.Op0, a5Base);
		if (instruction.n >= 2)
			fixOperand(address, 1, instruction.Op1, a5Base);
		if (instruction.n >= 3)
			fixOperand(address, 2, instruction.Op2, a5Base);
		if (instruction.n >= 4)
			fixOperand(address, 3, instruction.Op3, a5Base);
		if (instruction.n >= 5)
			fixOperand(address, 4, instruction.Op4, a5Base);
		if (instruction.n >= 6)
			fixOperand(address, 5, instruction.Op5, a5Base);
	}
}
