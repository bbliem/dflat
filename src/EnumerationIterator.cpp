/*
Copyright 2012-2013, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dynasp/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "EnumerationIterator.h"

EnumerationIterator::EnumerationIterator(const Row& row)
	: row(row), valid(true), curExtension(row.getExtensionPointers().begin())
{
	if(curExtension != row.getExtensionPointers().end()) {
		extensionIts.reserve(curExtension->size());
		foreach(const Row* extendedRow, *curExtension)
			extensionIts.push_back(new EnumerationIterator(*extendedRow));
	}
	materializeItems();
}

EnumerationIterator::~EnumerationIterator()
{
	foreach(EnumerationIterator* it, extensionIts)
		delete it;
}

void EnumerationIterator::reset()
{
	curExtension = row.getExtensionPointers().begin();
	valid = true;
	resetExtensionPointers();
}

inline void EnumerationIterator::resetExtensionPointers()
{
	if(curExtension != row.getExtensionPointers().end()) {
		assert(curExtension->size() == extensionIts.size());

		for(unsigned i = 0; i < curExtension->size(); ++i) {
			delete extensionIts[i];
			extensionIts[i] = new EnumerationIterator(*(*curExtension)[i]);
		}
	}
}

inline void EnumerationIterator::materializeItems()
{
	items.clear();
	items.insert(row.getItems().begin(), row.getItems().end());
	foreach(EnumerationIterator* it, extensionIts) {
		const Row::Items& extension = **it;
		items.insert(extension.begin(), extension.end());
	}
}

const Row::Items& EnumerationIterator::operator*() const
{
	assert(valid);
	return items;
}

EnumerationIterator& EnumerationIterator::operator++()
{
	assert(valid);

	if(curExtension == row.getExtensionPointers().end())
		valid = false;
	else {
		incrementExtensionIterator(0);
		if(valid)
			materializeItems();
	}

	return *this;
}

void EnumerationIterator::incrementExtensionIterator(unsigned int i) {
	if(i == extensionIts.size()) {
		// The last one was the rightmost extension iterator.
		// Use next extension pointer tuple.
		++curExtension;
		if(curExtension == row.getExtensionPointers().end())
			valid = false;
		else
			resetExtensionPointers();
	}
	else {
		++(*extensionIts[i]);
		if(!extensionIts[i]->valid) {
			// Now we need to advance the next iterator left of it ("carry operation")
			extensionIts[i]->reset();
			extensionIts[i]->materializeItems();
			incrementExtensionIterator(i + 1);
		}
	}
}
