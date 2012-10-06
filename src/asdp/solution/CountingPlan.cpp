#include <cassert>

#include "CountingPlan.h"

namespace asdp { namespace solution {

CountingPlan::CountingPlan(const CountingSolution::CountType& count)
	: Plan(LEAF), count(count)
{
}

CountingPlan* CountingPlan::leaf(const sharp::Tuple&)
{
	return new CountingPlan(1);
}

CountingPlan* CountingPlan::extend(const CountingPlan* base, const sharp::Tuple&)
{
	return new CountingPlan(base->count);
}

CountingPlan* CountingPlan::unify(const CountingPlan* left, const CountingPlan* right)
{
	return new CountingPlan(left->count + right->count);
}

CountingPlan* CountingPlan::join(const CountingPlan* left, const CountingPlan* right)
{
	return new CountingPlan(left->count * right->count);
}

sharp::Solution* CountingPlan::materializeLeaf() const
{
	assert(operation == LEAF);
	return new CountingSolution(count);
}

sharp::Solution* CountingPlan::materializeExtension() const
{
	assert(false);
	return 0;
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

}} // namespace asdp::solution
