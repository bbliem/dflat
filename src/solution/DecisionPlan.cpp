#include <cassert>

#include "DecisionPlan.h"
#include "DecisionSolution.h"


namespace solution {

DecisionPlan::DecisionPlan()
	: sharp::Plan(LEAF)
{
}

DecisionPlan* DecisionPlan::leaf(const sharp::Tuple&)
{
	return new DecisionPlan;
}

DecisionPlan* DecisionPlan::extend(const DecisionPlan*, const sharp::Tuple&)
{
	return new DecisionPlan;
}

DecisionPlan* DecisionPlan::unify(const DecisionPlan*, const DecisionPlan*)
{
	return new DecisionPlan;
}

DecisionPlan* DecisionPlan::join(const DecisionPlan*, const DecisionPlan*, const sharp::Tuple&)
{
	return new DecisionPlan;
}

sharp::Solution* DecisionPlan::materializeLeaf() const
{
	return new DecisionSolution;
}

sharp::Solution* DecisionPlan::materializeExtension() const
{
	assert(false);
	return 0;
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
