#pragma once

#include <sharp/main>

#include "../../Tuple.h"
#include "OptCountingSolution.h"

namespace asdp { namespace solution {

class OptCountingPlan : public sharp::Plan
{
public:
	static OptCountingPlan* leaf(const Tuple& tuple);
	static OptCountingPlan* extend(const OptCountingPlan* base, const Tuple& extension);
	static OptCountingPlan* unify(const OptCountingPlan* left, const OptCountingPlan* right);
	static OptCountingPlan* join(const OptCountingPlan* left, const OptCountingPlan* right, const Tuple& joined);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	OptCountingPlan(const OptCountingSolution::CountType& count, unsigned cost, unsigned currentCost);
	OptCountingSolution::CountType count;
	unsigned cost;
	unsigned currentCost;
};

}} // namespace asdp::solution
