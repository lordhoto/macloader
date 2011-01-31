/* util.cpp: Miscellaneous utilty definitions/functions
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
 */

#include "util.h"

// Helper functions for reading integers from the stream (maintaining endianness)
byte readByte(FILE *file) {
	byte b = 0;
	fread(&b, 1, 1, file);
	return b;
}

uint16 readUint16LE(FILE *file) {
	uint16 x = readByte(file);
	return x | readByte(file) << 8;
}

uint32 readUint32LE(FILE *file) {
	uint16 x = readUint16LE(file);
	return x | readUint16LE(file) << 16;
}

uint16 readUint16BE(FILE *file) {
	uint16 x = readByte(file) << 8;
	return x | readByte(file);
}

uint32 readUint32BE(FILE *file) {
	uint32 x = readUint16BE(file) << 16;
	return x | readUint16BE(file); 
}

void writeByte(FILE *file, byte b) {
	fwrite(&b, 1, 1, file);
}

void writeUint16LE(FILE *file, uint16 x) {
	writeByte(file, x & 0xff);
	writeByte(file, x >> 8);
}

void writeUint32LE(FILE *file, uint32 x) {
	writeUint16LE(file, x & 0xffff);
	writeUint16LE(file, x >> 16);
}

void writeUint16BE(FILE *file, uint16 x) {
	writeByte(file, x >> 8);
	writeByte(file, x & 0xff);
}

void writeUint32BE(FILE *file, uint32 x) {
	writeUint16BE(file, x >> 16);
	writeUint16BE(file, x & 0xffff);
}

uint32 getFileSize(FILE *file) {
	if (!file)
		return 0;

	uint32 pos = ftell(file);
	fseek(file, 0, SEEK_END);
	uint32 size = ftell(file);
	fseek(file, pos, SEEK_SET);
	return size;
}

uint16 READ_UINT16_BE(byte *data) {
	return (*data << 8) | *(data + 1);
}

uint32 READ_UINT32_BE(byte *data) {
	return (READ_UINT16_BE(data) << 16) | READ_UINT16_BE(data + 2);
}
