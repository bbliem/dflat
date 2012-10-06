#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "OptEnumPlan.h"
#include "OptEnumSolution.h"

namespace asdp { namespace solution {

OptEnumPlan::OptEnumPlan(Operation operation, const Tuple::Assignment& extension, unsigned cost, unsigned currentCost, const OptEnumPlan* left, const OptEnumPlan* right)
	: Plan(operation), extension(extension), cost(cost), currentCost(currentCost), left(left), right(right)
{
}

OptEnumPlan::OptEnumPlan(Operation operation, unsigned cost, unsigned currentCost, const OptEnumPlan* left, const OptEnumPlan* right)
	: Plan(operation), cost(cost), currentCost(currentCost), left(left), right(right)
{
}

OptEnumPlan* OptEnumPlan::leaf(const Tuple& tuple)
{
	assert(tuple.getCurrentCost() == tuple.getIntroducedCost());
	return new OptEnumPlan(LEAF, tuple.getAssignment(), tuple.getCurrentCost(), tuple.getCurrentCost());
}

OptEnumPlan* OptEnumPlan::extend(const OptEnumPlan* base, const Tuple& extension)
{
	return new OptEnumPlan(EXTENSION, extension.getAssignment(), base->cost + extension.getIntroducedCost(), extension.getCurrentCost(), base);
}

OptEnumPlan* OptEnumPlan::unify(const OptEnumPlan* left, const OptEnumPlan* right)
{
	assert(left->currentCost == right->currentCost);
	// If either left or right is more expensive than the other, we can dispense with it
	if(left->cost < right->cost)
		return new OptEnumPlan(*left);
	else if(right->cost < left->cost)
		return new OptEnumPlan(*right);
	else // left->cost == right->cost
		return new OptEnumPlan(UNION, left->cost, left->currentCost, left, right);
}

OptEnumPlan* OptEnumPlan::join(const OptEnumPlan* left, const OptEnumPlan* right)
{
	assert(left->currentCost == right->currentCost);
	// currentCost is contained in both left->cost and right->cost, so subtract it once
	return new OptEnumPlan(JOIN, left->cost + right->cost - left->currentCost, left->currentCost, left, right);
}

sharp::Solution* OptEnumPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return OptEnumSolution::leaf(extension, cost);
}

sharp::Solution* OptEnumPlan::materializeExtension() const
{
	assert(operation == EXTENSION && left);
	OptEnumSolution* baseSolution = dynamic_cast<OptEnumSolution*>(left->materialize());
	return OptEnumSolution::extend(baseSolution, extension, cost);
}

sharp::Solution* OptEnumPlan::materializeUnion() const
{
	assert(operation == UNION && left && right);
	OptEnumSolution* baseSolution = dynamic_cast<OptEnumSolution*>(left->materialize());
	OptEnumSolution* otherSolution = dynamic_cast<OptEnumSolution*>(right->materialize());
	return OptEnumSolution::unify(baseSolution, otherSolution, cost);
}

sharp::Solution* OptEnumPlan::materializeJoin() const
{
	assert(operation == JOIN && left && right);
	OptEnumSolution* leftSolution = dynamic_cast<OptEnumSolution*>(left->materialize());
	OptEnumSolution* rightSolution = dynamic_cast<OptEnumSolution*>(right->materialize());
	return OptEnumSolution::join(leftSolution, rightSolution, cost);
}

}} // namespace asdp::solution
