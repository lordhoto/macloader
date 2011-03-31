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

#include "idc.h"

#include <fstream>
#include <boost/format.hpp>

namespace IDC {

void writeJumpMarkTableScript(const Executable &exe, const std::string &baseFilename) throw(std::exception) {
	std::ofstream out((baseFilename + "_jt.idc").c_str());
	if (!out)
		throw std::runtime_error("Could not open file \"" + baseFilename + "_jt.idc\" for writing");

	const Code0Segment &code0 = exe.getCode0Segment();

	out << "#include <idc.idc>\n"
	       "\n"
	       "static main() {\n"
	       "\tauto num = " << code0.getJumpTableEntryCount() << ";\n"
	    << boost::format("\tauto offset = 0x%1$08X;\n") % code0.getJumpTableOffset()
	    << boost::format("\tauto a5offset = 0x%1$08X;\n") % code0.getApplicationGlobalsSize()
	    << "\t\n"
	       "\tauto i;\n"
	       "\tfor (i = 0; i < num; ++i) {\n"
	       "\t\t// Calculate the jumptable entry offset\n"
	       "\t\tauto entryOff = offset + i * 8;\n"
	       "\n"
	       "\t\t// Mark offset entry as dword\n"
	       "\t\tMakeDword(entryOff + 4);\n"
	       "\t\t// Read the function offset\n"
	       "\t\tauto funcOff = Dword(entryOff + 4);\n"
	       "\n"
	       "\t\t// Mark the function as code\n"
	       "\t\tAutoMark(funcOff, AU_CODE);\n"
	       "\t\t// Finally mark the function as procedure. Doing this after marking it\n"
	       "\t\t// as code, should allow IDA to mark more functions successfully.\n"
	       "\t\tAutoMark(funcOff, AU_PROC);\n"
	       "\t}\n"
	       "}\n";

	out.flush();
}

} // End of namespace IDC

