/*{{{
Copyright 2012-2016, Bernhard Bliem, Marius Moldovan
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

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
//}}}
#include <cstddef>
#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <gmpxx.h>

#include "String.h"

class ExtensionIterator;

class Row
{
public:
	typedef std::set<String> Items; // We need the sortedness for, e.g., the default join.
	typedef std::shared_ptr<Row> ExtensionPointer;
	typedef std::vector<ExtensionPointer> ExtensionPointerTuple;
	typedef std::vector<ExtensionPointerTuple> ExtensionPointers;

	// extensionPointers may not be empty; if there are no decomposition
	// children, it should contain a 0-tuple.
	// Sets the count to the sum of the products of the counts for each
	// extension pointer tuple.
	// Sets the cost to 0.
	Row(Items&& items = {}, Items&& auxItems = {}, ExtensionPointers&& extensionPointers = {{}});

	// Returns the items of this row (but not the auxiliary items, see below).
	const Items& getItems() const { return items; }

	// Returns the items that have been declared as auxiliary.
	// These items are disregarded in the default join.
	const Items& getAuxItems() const { return auxItems; }

	const ExtensionPointers& getExtensionPointers() const { return extensionPointers; }
	void clearExtensionPointers() { extensionPointers.clear(); }

	const mpz_class& getCount() const { return count; }

	long getCost() const { return cost; }
	void setCost(long cost);

	long getCurrentCost() const { return currentCost; }
	void setCurrentCost(long currentCost);

	// Unify extension pointers of this row with the other one's given that the item sets are equal.
	// "other" will subsequently be thrown away and only "this" will be retained.
	void merge(Row&& other);

	// Returns a negative/positive integer if this is "less"/"greater" than other, without considering costs.
	// Returns zero if this is equal to other, without considering costs.
	int compareCostInsensitive(const Row& other) const;

	// Materialize just one extension
	Items firstExtension() const;

	// Print this row (no newlines)
	friend std::ostream& operator<<(std::ostream& os, const Row& row);

private:
	Items items;
	Items auxItems;
	ExtensionPointers extensionPointers;
	mpz_class count; // number of possible extensions of this row
	long cost = 0;
	long currentCost = 0;
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Row>& row);
