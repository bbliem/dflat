#pragma once

#include <sharp/main>

#include "../../Tuple.h"

namespace asdp { namespace solution {

class OptValuePlan : public sharp::Plan
{
public:
	static OptValuePlan* leaf(const Tuple& tuple);
	static OptValuePlan* extend(const OptValuePlan* base, const Tuple& extension);
	static OptValuePlan* unify(const OptValuePlan* left, const OptValuePlan* right);
	static OptValuePlan* join(const OptValuePlan* left, const OptValuePlan* right, const Tuple& joined);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	OptValuePlan(unsigned cost, unsigned currentCost);
	unsigned cost;
	unsigned currentCost;
};

}} // namespace asdp::plan
