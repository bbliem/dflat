#include "EnumerationPlan.h"
#include "EnumerationSolution.h"

namespace asdp { namespace solution {

EnumerationPlan::EnumerationPlan(Operation operation, const Tuple::Assignment& extension, const EnumerationPlan* left, const EnumerationPlan* right)
	: Plan(operation), extension(extension), left(left), right(right)
{
}

EnumerationPlan::EnumerationPlan(Operation operation, const EnumerationPlan* left, const EnumerationPlan* right)
	: Plan(operation), left(left), right(right)
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
	return new EnumerationPlan(UNION, left, right);
}

EnumerationPlan* EnumerationPlan::join(const EnumerationPlan* left, const EnumerationPlan* right)
{
	return new EnumerationPlan(JOIN, left, right);
}

sharp::Solution* EnumerationPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return EnumerationSolution::leaf(extension);
}

sharp::Solution* EnumerationPlan::materializeExtension() const
{
	assert(operation == EXTENSION && left);
	EnumerationSolution* baseSolution = dynamic_cast<EnumerationSolution*>(left->materialize());
	return EnumerationSolution::extend(baseSolution, extension);
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
