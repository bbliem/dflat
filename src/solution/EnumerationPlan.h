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


#include <boost/foreach.hpp>
#include <sharp/main>

#include "../Tuple.h"

namespace solution {

// TODO: Deleting a plan does not delete recursively. This might be nasty since there are cycles.
class EnumerationPlan : public sharp::Plan
{
public:
	static EnumerationPlan* leaf(const Tuple&);
	static EnumerationPlan* unify(const EnumerationPlan* left, const EnumerationPlan* right);
	static EnumerationPlan* join(const Tuple& extension, const EnumerationPlan* left, const EnumerationPlan* right);

	unsigned getCost() const { return cost; }
	const Tuple::Assignment& getAssignment() const { return assignment; }
	const EnumerationPlan* getLeft() const { return left; }
	const EnumerationPlan* getRight() const { return right; }

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	EnumerationPlan(Operation operation, const Tuple& tuple, const EnumerationPlan* left = 0, const EnumerationPlan* right = 0);
	EnumerationPlan(const EnumerationPlan* left, const EnumerationPlan* right); // Union

	unsigned cost;
	Tuple::Assignment assignment;
	const EnumerationPlan* left;
	const EnumerationPlan* right;
};


} // namespace solution
