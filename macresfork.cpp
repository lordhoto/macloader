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

#include <cstdio>

#include "macresfork.h"

ResourceFork::ResourceFork() {
	_file = 0;
}

ResourceFork::~ResourceFork() {
	close();
}

bool ResourceFork::load(const char *filename) {
	if (loadFromRawFork(filename))
		return true;
	
	if (loadFromMacBaseFilename(filename))
		return true;

	if (loadFromMacBinary(filename))
		return true;

	if (loadFromAppleDouble(filename))
		return true;

	return false;
}

bool ResourceFork::loadFromRawFork(std::string filename) {
	_file = fopen(filename.c_str(), "rb");

	if (!_file)
		return false;

	return loadInternal();
}

bool ResourceFork::loadFromMacBaseFilename(std::string filename) {
#ifdef __APPLE__
	// On Mac OS X, try to access the resource fork directly
	return loadFromRawFork(filename + "/..namedfork/rsrc");
#else
	return false;
#endif
}

#define MBI_INFOHDR 128
#define MBI_ZERO1 0
#define MBI_NAMELEN 1
#define MBI_ZERO2 74
#define MBI_ZERO3 82
#define MBI_DFLEN 83
#define MBI_RFLEN 87
#define MAXNAMELEN 63

bool ResourceFork::loadFromMacBinary(std::string filename) {
	_file = fopen(filename.c_str(), "rb");

	if (!_file)
		return false;

	byte infoHeader[MBI_INFOHDR];
	fread(infoHeader, 1, MBI_INFOHDR, _file);

	// Try to parse the MacBinary header
	if (infoHeader[MBI_ZERO1] == 0 && infoHeader[MBI_ZERO2] == 0 &&
		infoHeader[MBI_ZERO3] == 0 && infoHeader[MBI_NAMELEN] <= MAXNAMELEN) {

		// Pull out the resource fork length
		uint32 dataSize = READ_UINT32_BE(infoHeader + MBI_DFLEN);
		uint32 rsrcSize = READ_UINT32_BE(infoHeader + MBI_RFLEN);

		uint32 dataSizePad = (((dataSize + 127) >> 7) << 7);
		uint32 rsrcSizePad = (((rsrcSize + 127) >> 7) << 7);

		// Length check
		if (MBI_INFOHDR + dataSizePad + rsrcSizePad == getFileSize(_file))
			return loadInternal(MBI_INFOHDR + dataSizePad);
	}

	close();
	return false;
}

bool ResourceFork::loadFromAppleDouble(std::string filename) {
	_file = fopen(filename.c_str(), "rb");

	if (!_file)
		return false;

	if (readUint32BE(_file) != 0x00051607) { // tag
		close();
		return false;
	}

	fseek(_file, 20, SEEK_CUR); // version + home file system

	uint16 entryCount = readUint16BE(_file);

	for (uint16 i = 0; i < entryCount; i++) {
		uint32 id = readUint32BE(_file);
		uint32 offset = readUint32BE(_file);
		/* uint32 length = */ readUint32BE(_file);

		if (id == 2) // Found the resource fork!
			return loadInternal(offset);
	}

	close();
	return false;
}

bool ResourceFork::loadInternal(uint32 startOffset) {
	fseek(_file, startOffset, SEEK_SET);

	uint32 fileSize = getFileSize(_file);
	uint32 dataOffset = readUint32BE(_file) + startOffset;
	uint32 mapOffset = readUint32BE(_file) + startOffset;

	if (feof(_file) || dataOffset == 0 || mapOffset == 0 || dataOffset >= fileSize || mapOffset >= fileSize) {
		close();
		return false;
	}

	fseek(_file, mapOffset + 24, SEEK_SET);

	uint16 typeOffset = readUint16BE(_file);
	uint16 nameOffset = readUint16BE(_file);
	uint16 typeCount = readUint16BE(_file) + 1;

	if (feof(_file) || typeOffset == 0 || typeOffset >= fileSize) {
		close();
		return false;
	}

	_types.resize(typeCount);

	for (uint16 i = 0; i < typeCount; i++) {
		_types[i].tag = readUint32BE(_file);
		uint16 idCount = readUint16BE(_file) + 1;
		uint16 idOffset = readUint16BE(_file);

		uint32 lastTypePos = ftell(_file);

		fseek(_file, idOffset + mapOffset + typeOffset, SEEK_SET);

		for (uint16 j = 0; j < idCount; j++) {
			ResourceForkID id;

			id.id = readUint16BE(_file);
			uint16 idNameOffset = readUint16BE(_file);
			id.offset = (readUint32BE(_file) & 0xffffff) + dataOffset;
			readUint32BE(_file);

			if (nameOffset != 0xffff) {
				uint32 lastIDPos = ftell(_file);

				fseek(_file, nameOffset + mapOffset + idNameOffset, SEEK_SET);

				byte stringLength = readByte(_file);
				char *subFilename = new char[stringLength + 1];
				subFilename[stringLength] = 0;
				fread(subFilename, 1, stringLength, _file);
	
				id.filename = subFilename;

				if (j != idCount - 1)
					fseek(_file, lastIDPos, SEEK_SET);

				printf("%c%c%c%c %04x - %s\n", _types[i].tag >> 24, (_types[i].tag >> 16) & 0xff, (_types[i].tag >> 8) & 0xff, _types[i].tag & 0xff, id.id, id.filename.c_str());
			} else
				printf("%c%c%c%c %04x\n", _types[i].tag >> 24, (_types[i].tag >> 16) & 0xff, (_types[i].tag >> 8) & 0xff, _types[i].tag & 0xff, id.id); 

			_types[i].ids.push_back(id);
		}

		if (i != typeCount - 1)
			fseek(_file, lastTypePos, SEEK_SET);
	}
	
	printf("\n\n\n\n");

	return true;
}

void ResourceFork::close() {
	if (_file) {
		fclose(_file);
		_file = 0;
	}

	_types.clear();
}

bool ResourceFork::isOpen() const { 
	return _file != 0;
}

DataPair *ResourceFork::getResource(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _types.size(); i++) {
		if (_types[i].tag != tag)
			continue;

		for (uint32 j = 0; j < _types[i].ids.size(); j++) {
			if (_types[i].ids[j].id != id)
				continue;

			fseek(_file, _types[i].ids[j].offset, SEEK_SET);
			uint32 length = readUint32BE(_file);
			byte *data = new byte[length];
			fread(data, 1, length, _file);
			return new DataPair(data, length);
		}
	}

	return 0;
}

// A very simple function to compare strings while ignoring case
static int compareStringIgnoreCase(const char *s1, const char *s2) {
    for(; tolower((byte)(*s1)) == tolower((byte)(*s2)); ++s1, ++s2)
        if (*s1 == 0 || *s2 == 0)
            return 0;

	return tolower((byte)(*s1)) - tolower((byte)(*s2));
}

DataPair *ResourceFork::getResource(const std::string &filename) {
	for (uint32 i = 0; i < _types.size(); i++) {
		for (uint32 j = 0; j < _types[i].ids.size(); j++) {
			if (compareStringIgnoreCase(_types[i].ids[j].filename.c_str(), filename.c_str()))
				continue;

			fseek(_file, _types[i].ids[j].offset, SEEK_SET);
			uint32 length = readUint32BE(_file);
			byte *data = new byte[length];
			fread(data, 1, length, _file);
			return new DataPair(data, length);
		}
	}

	return 0;
}

DataPair *ResourceFork::getResource(uint32 tag, const std::string &filename) {
	for (uint32 i = 0; i < _types.size(); i++) {
		if (_types[i].tag != tag)
			continue;

		for (uint32 j = 0; j < _types[i].ids.size(); j++) {
			if (compareStringIgnoreCase(_types[i].ids[j].filename.c_str(), filename.c_str()))
				continue;

			fseek(_file, _types[i].ids[j].offset, SEEK_SET);
			uint32 length = readUint32BE(_file);
			byte *data = new byte[length];
			fread(data, 1, length, _file);
			return new DataPair(data, length);
		}
	}

	return 0;
}

const char *ResourceFork::getFilename(uint32 tag, uint16 id) {
	for (uint32 i = 0; i < _types.size(); i++) {
		if (_types[i].tag != tag)
			continue;

		for (uint32 j = 0; j < _types[i].ids.size(); j++) {
			if (_types[i].ids[j].id != id)
				continue;

			if (!_types[i].ids[j].filename.empty())
				return _types[i].ids[j].filename.c_str();
		}
	}

	static char filename[12];
	sprintf(filename, "%c%c%c%c_%02d.dat", tag >> 24, (tag >> 16) & 0xff, (tag >> 8) & 0xff, tag & 0xff, id);

	return filename;
}

std::vector<uint32> ResourceFork::getTagArray() {
	std::vector<uint32> tagArray;

	if (!isOpen())
		return tagArray;

	tagArray.resize(_types.size());

	for (uint32 i = 0; i < _types.size(); i++)
		tagArray[i] = _types[i].tag;

	return tagArray;
}

std::vector<uint16> ResourceFork::getIDArray(uint32 tag) {
	std::vector<uint16> idArray;

	for (uint32 i = 0; i < _types.size(); i++)
		if (_types[i].tag == tag) {
			idArray.resize(_types[i].ids.size());

			for (uint32 j = 0; j < _types[i].ids.size(); j++)
				idArray[j] = _types[i].ids[j].id;

			return idArray;
		}

	return idArray;
}
