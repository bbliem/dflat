/*
Copyright 2012-2013, Bernhard Bliem
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

#include <set>
#include <string>
#include <memory>

class ItemTree
{
public:
	typedef std::set<std::string> Items; // Maybe a vector would be more efficient, but we need the sortedness for, e.g., the default join.
//	typedef std::vector<const Row*> ExtensionPointerTuple;
//	typedef std::vector<ExtensionPointerTuple> ExtensionPointers;

	// typedef std::set<ItemTree> Children;
	// ItemTree is an incomplete type, so the latter might pose problems. Using incomplete types in STL containers is forbidden.
	// See also the solutions on:
	// http://stackoverflow.com/questions/9860503/templated-recursive-data-types
	// See also:
	// http://stackoverflow.com/questions/8452566/why-cant-i-replace-stdmap-with-stdunordered-map
	// http://stackoverflow.com/questions/11438969/c-struct-that-contains-a-map-of-itself
	// In particular, look at bdonlan's answer in http://stackoverflow.com/questions/6527917/how-can-i-emulate-a-recursive-type-definition-in-c
	typedef std::unique_ptr<ItemTree> TreePtr;
	// The set of children is sorted ascendingly according to the following criterion:
	// A TreePtr is smaller than another if
	// (a) its item set is (lexicographically) smaller, or
	// (b) its item set is equal to the other's and its set of children is (lexicographically) smaller.
	struct TreePtrComparator { bool operator()(const TreePtr& lhs, const TreePtr& rhs); };
	typedef std::set<TreePtr, TreePtrComparator> Children;

	ItemTree(Items&& items);
	ItemTree(Items&& items, Children&& children);

	friend std::ostream& operator<<(std::ostream& os, const ItemTree& tree);

private:
	void printNode(std::ostream& os, bool root, bool last = false, std::string indent = "") const;

	Items items;
	Children children;
};
