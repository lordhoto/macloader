/* macresview.cpp: Mac resource fork parser
 * Copyright (c) 2010-2011 Matthew Hoops (clone2727)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Partially based on ScummVM's Mac resource fork parser
 */

#ifndef MACRESFORK_H
#define MACRESFORK_H

#include <string>
#include <vector>
#include <cstring>
#include "util.h"

struct ResourceForkID {
	ResourceForkID() { id = 0; }

	uint16 id;
	std::string filename;
	uint32 offset;
};

struct ResourceForkType {
	ResourceForkType() { tag = 0; }
	ResourceForkType(uint32 t) { tag = t; }

	uint32 tag;
	std::vector<ResourceForkID> ids;
};

struct DataPair {
	DataPair(byte *d, uint32 l) { data = d; length = l; }
	DataPair(const DataPair &d) : data(new byte[d.length]), length(d.length) {
		std::memcpy(data, d.data, d.length);
	}
	~DataPair() { delete[] data; }

	byte *data;
	uint32 length;
};

class ResourceFork {
public:
	ResourceFork();
	~ResourceFork();

	bool load(const char *filename);
	void close();
	bool isOpen() const;

	DataPair *getResource(uint32 tag, uint16 id);
	DataPair *getResource(const std::string &filename);
	DataPair *getResource(uint32 tag, const std::string &filename);

	const char *getFilename(uint32 tag, uint16 id);

	std::vector<uint32> getTagArray();
	std::vector<uint16> getIDArray(uint32 tag);

private:
	bool loadFromRawFork(std::string filename);
	bool loadFromMacBaseFilename(std::string filename);
	bool loadFromMacBinary(std::string filename);
	bool loadFromAppleDouble(std::string filename);

	bool loadInternal(uint32 startOffset = 0);

	FILE *_file;
	std::vector<ResourceForkType> _types;
};

#endif
