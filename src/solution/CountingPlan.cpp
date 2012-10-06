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

#include <cassert>

#include "CountingPlan.h"

namespace solution {

CountingPlan::CountingPlan(const CountingSolution::CountType& count, unsigned cost, unsigned currentCost)
	: Plan(LEAF), count(count), cost(cost), currentCost(currentCost)
{
}

CountingPlan* CountingPlan::leaf(const Tuple& tuple)
{
	assert(!tuple.getCost() || tuple.getCurrentCost() == tuple.getCost());
	return new CountingPlan(1, tuple.getCost(), tuple.getCurrentCost());
}

CountingPlan* CountingPlan::unify(const CountingPlan* left, const CountingPlan* right)
{
	assert(left->currentCost == right->currentCost);
	// If either left or right is more expensive than the other, we can dispense with it
	if(left->cost < right->cost)
		return new CountingPlan(*left);
	else if(right->cost < left->cost)
		return new CountingPlan(*right);
	else // left->cost == right->cost
		return new CountingPlan(left->count + right->count, left->cost, left->currentCost);
}

CountingPlan* CountingPlan::join(const Tuple& joined, const CountingPlan* left, const CountingPlan* right)
{
	if(right)
		return new CountingPlan(left->count * right->count, joined.getCost(), joined.getCurrentCost());
	return new CountingPlan(left->count, joined.getCost(), joined.getCurrentCost());
}

sharp::Solution* CountingPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return new CountingSolution(count, cost);
}

sharp::Solution* CountingPlan::materializeUnion() const
{
	assert(false);
	return 0;
}

sharp::Solution* CountingPlan::materializeJoin() const
{
	assert(false);
	return 0;
}

} // namespace solution
