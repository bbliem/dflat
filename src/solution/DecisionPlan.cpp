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

#include "DecisionPlan.h"
#include "DecisionSolution.h"

namespace solution {

DecisionPlan::DecisionPlan(const Row& row)
	: sharp::Plan(LEAF), cost(row.getCost()), currentCost(row.getCurrentCost())
{
}

DecisionPlan* DecisionPlan::leaf(const Row& row)
{
	assert(!row.getCost() || row.getCurrentCost() == row.getCost());
	return new DecisionPlan(row);
}

DecisionPlan* DecisionPlan::unify(const DecisionPlan* left, const DecisionPlan* right)
{
	assert(left->currentCost == right->currentCost);
	// If either left or right is more expensive than the other, we can dispense with it
	if(left->cost < right->cost)
		return new DecisionPlan(*left);
	else // left->cost >= right->cost
		return new DecisionPlan(*right);
}

DecisionPlan* DecisionPlan::join(const Row& joined, const DecisionPlan* left, const DecisionPlan* right)
{
	return new DecisionPlan(joined);
}

sharp::Solution* DecisionPlan::materializeLeaf() const
{
	return new DecisionSolution(cost);
}

sharp::Solution* DecisionPlan::materializeUnion() const
{
	assert(false);
	return 0;
}

sharp::Solution* DecisionPlan::materializeJoin() const
{
	assert(false);
	return 0;
}

} // namespace solution
