#include "EnumerationPlan.h"
#include "EnumerationSolution.h"

namespace asdp { namespace solution {

EnumerationPlan::EnumerationPlan(Operation operation, const Tuple::Assignment& assignment, const EnumerationPlan* left, const EnumerationPlan* right)
	: Plan(operation), assignment(assignment), left(left), right(right)
{
}

EnumerationPlan::EnumerationPlan(const EnumerationPlan* left, const EnumerationPlan* right)
	: Plan(UNION), left(left), right(right)
{
}

EnumerationPlan* EnumerationPlan::leaf(const Tuple& tuple)
{
	return new EnumerationPlan(LEAF, tuple.getAssignment());
}

EnumerationPlan* EnumerationPlan::extend(const EnumerationPlan* base, const Tuple& extension)
{
	return new EnumerationPlan(EXTENSION, extension.getAssignment(), base);
}

EnumerationPlan* EnumerationPlan::unify(const EnumerationPlan* left, const EnumerationPlan* right)
{
	return new EnumerationPlan(left, right);
}

EnumerationPlan* EnumerationPlan::join(const EnumerationPlan* left, const EnumerationPlan* right, const Tuple& joined)
{
	return new EnumerationPlan(JOIN, joined.getAssignment(), left, right);
}

sharp::Solution* EnumerationPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return EnumerationSolution::leaf(assignment);
}

sharp::Solution* EnumerationPlan::materializeExtension() const
{
	assert(operation == EXTENSION && left);
	EnumerationSolution* baseSolution = dynamic_cast<EnumerationSolution*>(left->materialize());
	return EnumerationSolution::extend(baseSolution, assignment);
}

sharp::Solution* EnumerationPlan::materializeUnion() const
{
	assert(operation == UNION && left && right);
	EnumerationSolution* baseSolution = dynamic_cast<EnumerationSolution*>(left->materialize());
	EnumerationSolution* otherSolution = dynamic_cast<EnumerationSolution*>(right->materialize());
	return EnumerationSolution::unify(baseSolution, otherSolution);
}

sharp::Solution* EnumerationPlan::materializeJoin() const
{
	assert(operation == JOIN && left && right);
	EnumerationSolution* leftSolution = dynamic_cast<EnumerationSolution*>(left->materialize());
	EnumerationSolution* rightSolution = dynamic_cast<EnumerationSolution*>(right->materialize());
	return EnumerationSolution::join(leftSolution, rightSolution);
}

}} // namespace asdp::solution
