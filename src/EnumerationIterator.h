
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

#pragma once

#include <sharp/main>

#include "Row.h"

// TODO: Maybe make it a "proper" iterator? (E.g., use Boost Iterator?)
class EnumerationIterator : public sharp::Solution
{
public:
	explicit EnumerationIterator(const Row&);
	virtual ~EnumerationIterator();

	//! @return true iff this iterator can be dereferenced
	bool isValid() const { return valid; }

	const Row::Items& operator*() const; // dereference
	EnumerationIterator& operator++();

private:
	// Reset to initial state after construction
	void reset();
	void resetExtensionPointers();

	// Materialize what will be returned upon dereferencing
	void materializeItems();

	void incrementExtensionIterator(unsigned int i);

	Row::Items items;
	const Row& row;
	bool valid;
	Row::ExtensionPointers::const_iterator curExtension; // current ExtensionPointerTuple in row
	std::vector<EnumerationIterator*> extensionIts; // for each extended row in *eptIt one iterator
};
