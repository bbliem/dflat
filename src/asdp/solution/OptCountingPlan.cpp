#include <cassert>

#include "OptCountingPlan.h"

namespace asdp { namespace solution {

OptCountingPlan::OptCountingPlan(const OptCountingSolution::CountType& count, unsigned cost, unsigned currentCost)
	: Plan(LEAF), count(count), cost(cost), currentCost(currentCost)
{
}

OptCountingPlan* OptCountingPlan::leaf(const Tuple& tuple)
{
	assert(tuple.getCurrentCost() == tuple.getIntroducedCost());
	return new OptCountingPlan(1, tuple.getCurrentCost(), tuple.getCurrentCost());
}

OptCountingPlan* OptCountingPlan::extend(const OptCountingPlan* base, const Tuple& extension)
{
	return new OptCountingPlan(base->count, base->cost + extension.getIntroducedCost(), extension.getCurrentCost());
}

OptCountingPlan* OptCountingPlan::unify(const OptCountingPlan* left, const OptCountingPlan* right)
{
	assert(left->currentCost == right->currentCost);
	// If either left or right is more expensive than the other, we can dispense with it
	if(left->cost < right->cost)
		return new OptCountingPlan(*left);
	else if(right->cost < left->cost)
		return new OptCountingPlan(*right);
	else // left->cost == right->cost
		return new OptCountingPlan(left->count + right->count, left->cost, left->currentCost);
}

OptCountingPlan* OptCountingPlan::join(const OptCountingPlan* left, const OptCountingPlan* right)
{
	assert(left->currentCost == right->currentCost);
	// currentCost is contained in both left->cost and right->cost, so subtract it once
	return new OptCountingPlan(left->count * right->count, left->cost + right->cost - left->currentCost, left->currentCost);
}

sharp::Solution* OptCountingPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return new OptCountingSolution(count, cost);
}

sharp::Solution* OptCountingPlan::materializeExtension() const
{
	assert(false);
	return 0;
}

sharp::Solution* OptCountingPlan::materializeUnion() const
{
	assert(false);
	return 0;
}

sharp::Solution* OptCountingPlan::materializeJoin() const
{
	assert(false);
	return 0;
}

}} // namespace asdp::solution
