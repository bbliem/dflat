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

#include "EnumerationSolution.h"

namespace solution {

EnumerationSolution::EnumerationSolution()
{
}

EnumerationSolution* EnumerationSolution::leaf(const Tuple::Assignment& leafSolution)
{
	EnumerationSolution* s = new EnumerationSolution;
	s->assignments.insert(leafSolution);
	return s;
}

EnumerationSolution* EnumerationSolution::extend(EnumerationSolution* base, const Tuple::Assignment& extension)
{
	EnumerationSolution* s = new EnumerationSolution;
	foreach(const Tuple::Assignment& baseAssignment, base->assignments) {
		Tuple::Assignment newAssignment = extension;
		newAssignment.insert(baseAssignment.begin(), baseAssignment.end());
		s->assignments.insert(newAssignment);
	}
	delete base;
	return s;
}

EnumerationSolution* EnumerationSolution::unify(EnumerationSolution* left, EnumerationSolution* right)
{
	left->assignments.insert(right->assignments.begin(), right->assignments.end());
	delete right;
	return left;
}

EnumerationSolution* EnumerationSolution::join(EnumerationSolution* left, EnumerationSolution* right)
{
	EnumerationSolution* s = new EnumerationSolution;
	foreach(const Tuple::Assignment& l, left->assignments) {
		foreach(const Tuple::Assignment& r, right->assignments) {
			Tuple::Assignment j = l;
			j.insert(r.begin(), r.end());
			s->assignments.insert(j);
		}
	}
	delete left;
	delete right;
	return s;
}

const std::set<Tuple::Assignment>& EnumerationSolution::getSolutions() const
{
	return assignments;
}

} // namespace solution
