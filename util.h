/* util.h: Miscellaneous utilty definitions/functions
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

#ifndef UTIL_H
#define UTIL_H

#include <cstdio>

// Standard types
// If they're not this size on you're system, then whatever :P
typedef unsigned char byte;
typedef signed short int16;
typedef unsigned short uint16;
typedef unsigned int uint32;

uint16 READ_UINT16_BE(byte *data);
uint32 READ_UINT32_BE(byte *data);

byte readByte(FILE *file);
uint16 readUint16LE(FILE *file);
uint32 readUint32LE(FILE *file);
uint16 readUint16BE(FILE *file);
uint32 readUint32BE(FILE *file);
void writeByte(FILE *file, byte b);
void writeUint16LE(FILE *file, uint16 x);
void writeUint32LE(FILE *file, uint32 x);
void writeUint16BE(FILE *file, uint16 x);
void writeUint32BE(FILE *file, uint32 x);

uint32 getFileSize(FILE *file);

#endif
