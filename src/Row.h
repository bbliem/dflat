/*
Copyright 2012, Bernhard Bliem
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

#include <cassert>
#include <gmpxx.h>
#include <sharp/main>
#include <boost/container/map.hpp>

class Row : public sharp::Row
{
public:
	typedef std::set<std::string> Items; // Maybe a vector would be more efficient, but we need the sortedness for, e.g., the default join.
	typedef std::vector<const Row*> ExtensionPointerTuple;
	typedef std::vector<ExtensionPointerTuple> ExtensionPointers;

	Row(const Items& topLevelItems);

	//! Must be asymmetric
	virtual bool operator<(const sharp::Row&) const;
	virtual bool operator==(const sharp::Row&) const;

	virtual void unify(sharp::Row& other);

	//! Only called by the default join algorithm
	//! @return true iff this row is a join partner of the other
	bool matches(const Row& other) const;

	//! Only called by the default join algorithm
	//! @return a new row resulting from this joining with the other
	Row* join(const Row& other) const;

	//! Declares this row in ASP. Among other things, childRow/2 will declare this row's name and the number of the child table containing it.
	//! @param childNumber To which child node this row belongs; second argument of childRow/2
	void declare(std::ostream& out, unsigned childNumber) const;

	/**
	 * Each row consists of a set of items (often assignments of values to
	 * vertex names), and it might have additional information (e.g.
	 * "certificates" that can be revelant to whether a row is valid or not).
	 *
	 * @return this row's items (without any additional information on levels
	 * higher than 0 like, e.g., certificates)
	 */
	const Items& getItems() const { 
		assert(tree.children.size() == 1);
		return tree.children.begin()->first;
	}

	//! Adds the given path of subsidiary item sets
	template <class Iterator> void addSubItemsPath(Iterator begin, Iterator end)
	{
		assert(tree.children.empty() || *begin == getItems());
		tree.addPath(begin, end);
	}

	// TODO: Let the user count if desired
	const mpz_class& getCount() const { return count; }
	void setCount(const mpz_class& c) { count = c; }

	/**
	 * If using the default join implementation, this method is used to calculate the total cost of joining two child rows.
	 * @return cost of the row considering only the current vertices
	 */
	unsigned int getCurrentCost() const { return currentCost; }
	void setCurrentCost(unsigned int c) { currentCost = c; }

	//! @return cost of the (partial) solution of this row, considering the current and all forgotten vertices
	unsigned int getCost() const { return cost; }
	void setCost(unsigned int c) { cost = c; }

	const ExtensionPointers& getExtensionPointers() const { return extensionPointers; }
	//! Adds the given tuple to the extension pointers and adds the product of the extended rows' counts to this row's solution count
	void addExtensionPointerTuple(const ExtensionPointerTuple&);

#ifdef PRINT_COMPUTED_ROWS
	void setIndex(unsigned int i) { index = i; }
	unsigned int getIndex() const { return index; }
	void print(std::ostream&) const;
#endif

private:
	// Each item set has a set of subordinate item sets
	struct Tree
	{
		typedef boost::container::map<Items, Tree> Children; // The node data is stored as the keys
		Children children;
		// std::map won't work because Tree is an incomplete type at this time.
		// Cf. http://stackoverflow.com/questions/6527917/how-can-i-emulate-a-recursive-type-definition-in-c
		// Cf. http://www.boost.org/doc/libs/1_48_0/doc/html/container/containers_of_incomplete_types.html

		bool operator==(const Tree& rhs) const;
		bool operator<(const Tree& rhs) const;

		//! Adds the given path of rows as descendants to this tree
		template <class Iterator> void addPath(Iterator begin, Iterator end)
		{
			if(begin != end) {
				Tree& child = children[*begin];
				child.addPath(++begin, end);
			}
		}

		void declare(std::ostream& out, const std::string& parent) const;
#ifdef PRINT_COMPUTED_ROWS
		void print(std::ostream& out, const std::string& indent = "") const;
#endif
	} tree; // It is your responsibility that "tree" only has one child (viz. the actual root)

	// TODO: We might distinguish rows with some additional information from those without, but OTOH the memory consumption should not be that critical
	mpz_class count;
	unsigned int currentCost;
	unsigned int cost;
	ExtensionPointers extensionPointers;

#ifdef PRINT_COMPUTED_ROWS
	unsigned int index; // Index in the table
#endif 
};
