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
#include <sharp/main>

struct Row : public sharp::Row
{
	//! Must be asymmetric
	virtual bool operator<(const sharp::Row&) const = 0;
	virtual bool operator==(const sharp::Row&) const = 0;

	//! Only called by the default join algorithm
	//! @return true iff this row is a join partner of the other
	virtual bool matches(const Row& other) const = 0;

	//! Only called by the default join algorithm
	//! @return a new row resulting from this joining with the other
	virtual Row* join(const Row& other) const = 0;

	//! Declares this row in ASP. Among other things, childRow/2 will declare this row's name and the number of the child table containing it.
	//! @param rowAndSolution reference to the entry in the Table that contains this row
	//! @param childNumber To which child node this row belongs; second argument of childRow/2
	virtual void declare(std::ostream& out, const sharp::Table::value_type& rowAndSolution, unsigned childNumber) const = 0;

	//! Declares this row in ASP. The number of the child table containing it will not be declared (can be used, e.g., on semi-normalized TDs) but the name of the predicate that should declare this row's name can be specified.
	//! @param rowAndSolution reference to the entry in the Table that contains this row
	//! @param predicateName name of the predicate that should be used to declare this row's name
	virtual void declare(std::ostream& out, const sharp::Table::value_type& rowAndSolution, const char* predicateName = "childRow") const = 0;

	typedef std::vector<std::string> Items;
	/**
	 * Each row consists of a set of items (often assignments of values to
	 * vertex names), and it might have additional information (e.g.
	 * "certificates" that can be revelant to whether a row is valid or not).
	 *
	 * @return this row's items (without any additional information on levels
	 * higher than 0 like, e.g., certificates)
	 */
	virtual const Items& getItems() const = 0;

	/**
	 * If using the default join implementation, this method is used to calculate the total cost of joining two child rows.
	 * @return cost of the row considering only the current vertices
	 */
	virtual unsigned int getCurrentCost() const { return 0; }

	//! @return cost of the (partial) solution of this row, considering the current and all forgotten vertices
	virtual unsigned int getCost() const { return 0; }

#ifdef PRINT_COMPUTED_ROWS
	virtual void print(std::ostream&) const = 0;
#endif
};
