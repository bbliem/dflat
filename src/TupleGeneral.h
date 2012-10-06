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

#include <sharp/main>
#include <boost/container/map.hpp>

#include "Tuple.h"

class TupleGeneral : public Tuple
{
public:
	TupleGeneral();

	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;
	virtual void unify(const sharp::Tuple& old);

	virtual bool matches(const Tuple& other) const;
	virtual TupleGeneral* join(const Tuple& other) const;
	virtual void declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateSuffix = "") const;
	virtual const Assignment& getAssignment() const;
	virtual unsigned int getCurrentCost() const;
	virtual unsigned int getCost() const;

#ifdef VERBOSE
	virtual void print(std::ostream&) const;
#endif

	// Each assignment has a set of subordinate assignments
	struct Tree
	{
		typedef boost::container::map<Assignment, Tree> Children; // The node data is stored as the keys
		Children children;
		// std::set won't work because Tree is an incomplete type at this time.
		// Cf. http://stackoverflow.com/questions/6527917/how-can-i-emulate-a-recursive-type-definition-in-c
		// Cf. http://www.boost.org/doc/libs/1_48_0/doc/html/container/containers_of_incomplete_types.html

		bool operator==(const Tree& rhs) const;
		bool operator<(const Tree& rhs) const;

		//! Adds the given path of assignments as descendants to this tree
		template <class Iterator> void addPath(Iterator begin, Iterator end)
		{
			if(begin != end) {
				Tree& child = children[*begin];
				child.addPath(++begin, end);
			}
		}
	} tree; // It is your responsibility that "tree" only has one child (viz. the actual root)

	// TODO: We might distinguish tuples with cost information from those without, but OTOH the memory consumption should not be that critical
	unsigned int currentCost;
	unsigned int cost;
};
