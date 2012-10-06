#include <cassert>

#include "OptValuePlan.h"
#include "OptValueSolution.h"

namespace solution {

OptValuePlan::OptValuePlan(unsigned cost, unsigned currentCost)
	: sharp::Plan(LEAF), cost(cost), currentCost(currentCost)
{
}

OptValuePlan* OptValuePlan::leaf(const Tuple& tuple)
{
	assert(!tuple.getCost() || tuple.getCurrentCost() == tuple.getCost());
	return new OptValuePlan(tuple.getCost(), tuple.getCurrentCost());
}

OptValuePlan* OptValuePlan::extend(const OptValuePlan* base, const Tuple& extension)
{
	return new OptValuePlan(extension.getCost(), extension.getCurrentCost());
}

OptValuePlan* OptValuePlan::unify(const OptValuePlan* left, const OptValuePlan* right)
{
	assert(left->currentCost == right->currentCost);
	// If either left or right is more expensive than the other, we can dispense with it
	if(left->cost < right->cost)
		return new OptValuePlan(*left);
	else // left->cost >= right->cost
		return new OptValuePlan(*right);
}

OptValuePlan* OptValuePlan::join(const OptValuePlan* left, const OptValuePlan* right, const Tuple& joined)
{
	return new OptValuePlan(joined.getCost(), joined.getCurrentCost());
}

sharp::Solution* OptValuePlan::materializeLeaf() const
{
	return new OptValueSolution(cost);
}

sharp::Solution* OptValuePlan::materializeExtension() const
{
	assert(false);
	return 0;
}

sharp::Solution* OptValuePlan::materializeUnion() const
{
	assert(false);
	return 0;
}

sharp::Solution* OptValuePlan::materializeJoin() const
{
	assert(false);
	return 0;
}

} // namespace solution
