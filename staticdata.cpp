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

#include "staticdata.h"

#include <boost/foreach.hpp>

StaticDataLoaderManager::StaticDataLoaderManager(Executable &exe) throw(std::exception)
    : _loaders() {
}

StaticDataLoaderManager::~StaticDataLoaderManager() {
	BOOST_FOREACH(StaticDataLoader *&loader, _loaders) {
		destroy(loader);
	}

	_loaders.clear();
}

bool StaticDataLoaderManager::loadFromSegment(const std::string &name, const uint32 offset, const uint32 size, std::ostream &out) throw(std::exception) {
	BOOST_FOREACH(StaticDataLoader *loader, _loaders) {
		if (loader->isSupported(name, offset, size)) {
			out << "Loading data from segment \"" << name << "\" with loader: \"" << loader->getName() << "\"\n";
			loader->reset();
			loader->load(offset, size, out);
			return true;
		}
	}

	return false;
}

