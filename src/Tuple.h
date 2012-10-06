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

struct Tuple : public sharp::Tuple
{
	//! Must be asymmetric
	virtual bool operator<(const sharp::Tuple&) const = 0;
	virtual bool operator==(const sharp::Tuple&) const = 0;

	//! Only called by the default join algorithm
	//! @return true iff this tuple is a join partner of the other
	virtual bool matches(const Tuple& other) const = 0;

	//! Only called by the default join algorithm
	//! @return a new tuple resulting from this joining with the other
	virtual Tuple* join(const Tuple& other) const = 0;

	//! Declares this tuple in ASP. Among other things, childTuple/2 will declare this tuple's name and the number of the child table containing it.
	//! @param tupleAndSolution reference to the entry in the TupleTable that contains this tuple
	//! @param childNumber To which child node this tuple belongs; second argument of childTuple/2
	virtual void declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, unsigned childNumber) const = 0;

	//! Declares this tuple in ASP. The number of the child table containing it will not be declared (can be used, e.g., on semi-normalized TDs) but the name of the predicate that should declare this tuple's name can be specified.
	//! @param tupleAndSolution reference to the entry in the TupleTable that contains this tuple
	//! @param predicateName name of the predicate that should be used to declare this tuple's name
	virtual void declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateName = "childTuple") const = 0;

	typedef std::vector<std::string> Row;
	/**
	 * Each tuple consists of a row (a set of elements/items, often assignments
	 * of values to vertex names), and it might have additional information
	 * (e.g.  "certificates" that can be revelant to whether a tuple is valid
	 * or not).
	 *
	 * @return this tuple's row (without any additional information on levels
	 * higher than 0 like, e.g., certificates)
	 */
	virtual const Row& getRow() const = 0;

	/**
	 * If using the default join implementation, this method is used to calculate the total cost of joining two child tuples.
	 * @return cost of the row considering only the current vertices
	 */
	virtual unsigned int getCurrentCost() const { return 0; }

	//! @return cost of the (partial) solution of this tuple, considering the current and all forgotten vertices
	virtual unsigned int getCost() const { return 0; }

#ifdef PRINT_COMPUTED_TUPLES
	virtual void print(std::ostream&) const = 0;
#endif
};
