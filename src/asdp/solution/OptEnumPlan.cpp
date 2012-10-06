#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "OptEnumPlan.h"
#include "OptEnumSolution.h"

namespace asdp { namespace solution {

OptEnumPlan::OptEnumPlan(Operation operation, const Tuple::Assignment& assignment, unsigned cost, unsigned currentCost, const OptEnumPlan* left, const OptEnumPlan* right)
	: Plan(operation), assignment(assignment), cost(cost), currentCost(currentCost), left(left), right(right)
{
}

OptEnumPlan::OptEnumPlan(unsigned cost, unsigned currentCost, const OptEnumPlan* left, const OptEnumPlan* right)
	: Plan(UNION), cost(cost), currentCost(currentCost), left(left), right(right)
{
}

OptEnumPlan* OptEnumPlan::leaf(const Tuple& tuple)
{
	assert(!tuple.getCost() || tuple.getCurrentCost() == tuple.getCost());
	return new OptEnumPlan(LEAF, tuple.getAssignment(), tuple.getCost(), tuple.getCurrentCost());
}

OptEnumPlan* OptEnumPlan::extend(const OptEnumPlan* base, const Tuple& extension)
{
	return new OptEnumPlan(EXTENSION, extension.getAssignment(), extension.getCost(), extension.getCurrentCost(), base);
}

OptEnumPlan* OptEnumPlan::unify(const OptEnumPlan* left, const OptEnumPlan* right)
{
	assert((left->currentCost && right->currentCost) || (!left->currentCost && !right->currentCost));
	assert(!left->currentCost || left->currentCost == right->currentCost);
	// If either left or right is more expensive than the other, we can dispense with it
	if(left->cost < right->cost)
		return new OptEnumPlan(*left);
	else if(right->cost < left->cost)
		return new OptEnumPlan(*right);
	else // left->cost == right->cost
		return new OptEnumPlan(left->cost, left->currentCost, left, right);
}

OptEnumPlan* OptEnumPlan::join(const OptEnumPlan* left, const OptEnumPlan* right, const Tuple& joined)
{
	return new OptEnumPlan(JOIN, joined.getAssignment(), joined.getCost(), joined.getCurrentCost(), left, right);
}

sharp::Solution* OptEnumPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return OptEnumSolution::leaf(assignment, cost);
}

sharp::Solution* OptEnumPlan::materializeExtension() const
{
	assert(operation == EXTENSION && left);
	OptEnumSolution* baseSolution = dynamic_cast<OptEnumSolution*>(left->materialize());
	return OptEnumSolution::extend(baseSolution, assignment, cost);
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
