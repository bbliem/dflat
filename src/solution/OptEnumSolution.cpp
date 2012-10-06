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

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "OptEnumSolution.h"

namespace solution {

OptEnumSolution::OptEnumSolution()
{
}

OptEnumSolution* OptEnumSolution::leaf(const Tuple::Assignment& leafSolution, unsigned cost)
{
	OptEnumSolution* s = new OptEnumSolution;
	s->cost = cost;
	s->assignments.insert(leafSolution);
	return s;
}

OptEnumSolution* OptEnumSolution::extend(OptEnumSolution* base, const Tuple::Assignment& extension, unsigned cost)
{
	OptEnumSolution* s = new OptEnumSolution;
	foreach(const Tuple::Assignment& baseAssignment, base->assignments) {
		Tuple::Assignment newAssignment = extension;
		newAssignment.insert(baseAssignment.begin(), baseAssignment.end());
		s->assignments.insert(newAssignment);
	}
	s->cost = cost;
	delete base;
	return s;
}

OptEnumSolution* OptEnumSolution::unify(OptEnumSolution* left, OptEnumSolution* right, unsigned cost)
{
	left->assignments.insert(right->assignments.begin(), right->assignments.end());
	left->cost = cost;
	delete right;
	return left;
}

OptEnumSolution* OptEnumSolution::join(OptEnumSolution* left, OptEnumSolution* right, unsigned cost)
{
	OptEnumSolution* s = new OptEnumSolution;
	foreach(const Tuple::Assignment& l, left->assignments) {
		foreach(const Tuple::Assignment& r, right->assignments) {
			Tuple::Assignment j = l;
			j.insert(r.begin(), r.end());
			s->assignments.insert(j);
		}
	}
	s->cost = cost;
	delete left;
	delete right;
	return s;
}

unsigned OptEnumSolution::getCost() const
{
	return cost;
}

const std::set<Tuple::Assignment>& OptEnumSolution::getSolutions() const
{
	return assignments;
}

} // namespace solution
