/*{{{
Copyright 2012-2016, Bernhard Bliem
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
#include <memory>
#include <limits>
#include <unordered_map>

#include "Row.h"

class Table;
typedef std::unique_ptr<Table> TablePtr;
class Application;
typedef std::shared_ptr<Row> RowPtr;
struct RowComparator { bool operator()(const RowPtr& lhs, const RowPtr& rhs); };
typedef std::set<RowPtr, RowComparator> Rows; // TODO If we don't use binary search, could we use unordered sets?

class Table
{
public:
	const Rows& getRows() const { return rows; }

	// If there already is a row that is equal to the given one, it is unified
	// with the given one.
	// If there is already an equal row that is more expensive, it is replaced
	// with the given row.
	// If a new row was added, or if merging resulted in a change of costs
	// (i.e., the given row leads to a better partial solution), returns an
	// iterator to this new row; otherwise returns an iterator to
	// this->rows.end().
	Rows::const_iterator add(RowPtr&& row);
	/// XXX was: costChangeAfterAddChildAndMerge

	// Print all extensions of all rows
	// TOOD
	//void printExtensions(std::ostream& os) const;

	// Print this table. If names is empty, prints indices.
	void printWithNames(std::ostream& os, const std::vector<unsigned>& names) const;

	friend std::ostream& operator<<(std::ostream& os, const Table& table)
	{
		table.printWithNames(os, {});
		return os;
	}

private:
	Rows rows;

	// Recursively unify extension pointers of this row with the other one's
	// given that the item sets are all equal.
	void merge(Table&& other);

#ifndef NDEBUG
	void printDebug() const;
#endif
};

// Key: Global ID of child node; value: the child node's table
typedef std::unordered_map<unsigned int, TablePtr> ChildTables;
