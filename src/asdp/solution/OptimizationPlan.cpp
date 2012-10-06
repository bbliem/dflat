#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "OptimizationPlan.h"
#include "OptimizationSolution.h"

namespace asdp { namespace solution {

OptimizationPlan::OptimizationPlan(Operation operation, const Tuple::Assignment& extension, unsigned cost, unsigned currentCost, const OptimizationPlan* left, const OptimizationPlan* right)
	: Plan(operation), extension(extension), cost(cost), currentCost(currentCost), left(left), right(right)
{
}

OptimizationPlan::OptimizationPlan(Operation operation, unsigned cost, unsigned currentCost, const OptimizationPlan* left, const OptimizationPlan* right)
	: Plan(operation), cost(cost), currentCost(currentCost), left(left), right(right)
{
}

OptimizationPlan* OptimizationPlan::leaf(const Tuple& tuple)
{
	assert(tuple.getCurrentCost() == tuple.getIntroducedCost());
	return new OptimizationPlan(LEAF, tuple.getAssignment(), tuple.getCurrentCost(), tuple.getCurrentCost());
}

OptimizationPlan* OptimizationPlan::extend(const OptimizationPlan* base, const Tuple& extension)
{
	return new OptimizationPlan(EXTENSION, extension.getAssignment(), base->cost + extension.getIntroducedCost(), extension.getCurrentCost(), base);
}

OptimizationPlan* OptimizationPlan::unify(const OptimizationPlan* left, const OptimizationPlan* right)
{
	assert(left->currentCost == right->currentCost);
	// If either left or right is more expensive than the other, we can dispense with it
	if(left->cost < right->cost)
		return new OptimizationPlan(*left);
	else if(right->cost < left->cost)
		return new OptimizationPlan(*right);
	else
		return new OptimizationPlan(UNION, std::min(left->cost, right->cost), left->currentCost, left, right);
}

OptimizationPlan* OptimizationPlan::join(const OptimizationPlan* left, const OptimizationPlan* right)
{
	assert(left->currentCost == right->currentCost);
	// currentCost is contained in both left->cost and right->cost, so subtract it once
	return new OptimizationPlan(JOIN, left->cost + right->cost - left->currentCost, left->currentCost, left, right);
}

sharp::Solution* OptimizationPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return OptimizationSolution::leaf(extension, cost);
}

sharp::Solution* OptimizationPlan::materializeExtension() const
{
	assert(operation == EXTENSION && left);
	OptimizationSolution* baseSolution = dynamic_cast<OptimizationSolution*>(left->materialize());
	return OptimizationSolution::extend(baseSolution, extension, cost);
}

sharp::Solution* OptimizationPlan::materializeUnion() const
{
	assert(operation == UNION && left && right);
	OptimizationSolution* baseSolution = dynamic_cast<OptimizationSolution*>(left->materialize());
	OptimizationSolution* otherSolution = dynamic_cast<OptimizationSolution*>(right->materialize());
	return OptimizationSolution::unify(baseSolution, otherSolution, cost);
}

sharp::Solution* OptimizationPlan::materializeJoin() const
{
	assert(operation == JOIN && left && right);
	OptimizationSolution* leftSolution = dynamic_cast<OptimizationSolution*>(left->materialize());
	OptimizationSolution* rightSolution = dynamic_cast<OptimizationSolution*>(right->materialize());
	return OptimizationSolution::join(leftSolution, rightSolution, cost);
}

}} // namespace asdp::solution
