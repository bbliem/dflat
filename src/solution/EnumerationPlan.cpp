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

#include "EnumerationPlan.h"
#include "EnumerationIterator.h"

namespace solution {

EnumerationPlan::EnumerationPlan(Operation operation, const Tuple& tuple, const EnumerationPlan* left, const EnumerationPlan* right)
	: Plan(operation), cost(tuple.getCost()), assignment(tuple.getAssignment()), left(left), right(right)
{
}

EnumerationPlan::EnumerationPlan(const EnumerationPlan* left, const EnumerationPlan* right)
	: Plan(UNION), cost(left->getCost()), left(left), right(right)
{
	assert(left && right && left->getCost() == right->getCost());
}

EnumerationPlan* EnumerationPlan::leaf(const Tuple& tuple)
{
	return new EnumerationPlan(LEAF, tuple);
}

EnumerationPlan* EnumerationPlan::unify(const EnumerationPlan* left, const EnumerationPlan* right)
{
	assert(left && right);
	// If either left or right is more expensive than the other, we can dispense with it
	if(left->cost < right->cost)
		return new EnumerationPlan(*left);
	else if(right->cost < left->cost)
		return new EnumerationPlan(*right);
	else
		return new EnumerationPlan(left, right);
}

EnumerationPlan* EnumerationPlan::join(const Tuple& extension, const EnumerationPlan* left, const EnumerationPlan* right)
{
	return new EnumerationPlan(JOIN, extension, left, right);
}

sharp::Solution* EnumerationPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return new EnumerationIterator(*this);
}

sharp::Solution* EnumerationPlan::materializeUnion() const
{
	assert(operation == UNION && left && right);
	return new EnumerationIterator(*this);
}

sharp::Solution* EnumerationPlan::materializeJoin() const
{
	assert(operation == JOIN && left);
	return new EnumerationIterator(*this);
}

} // namespace solution
