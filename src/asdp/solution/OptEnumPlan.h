#pragma once

#include <sharp/main>

#include "../../Tuple.h"

namespace asdp { namespace solution {

class OptEnumPlan : public sharp::Plan
{
public:
	static OptEnumPlan* leaf(const Tuple& tuple);
	static OptEnumPlan* extend(const OptEnumPlan* base, const Tuple& extension);
	static OptEnumPlan* unify(const OptEnumPlan* left, const OptEnumPlan* right);
	static OptEnumPlan* join(const OptEnumPlan* left, const OptEnumPlan* right, const Tuple& joined);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	OptEnumPlan(Operation operation, const Tuple::Assignment& assignment, unsigned cost, unsigned currentCost, const OptEnumPlan* left = 0, const OptEnumPlan* right = 0);
	OptEnumPlan(unsigned cost, unsigned currentCost, const OptEnumPlan* left, const OptEnumPlan* right); // Union

	Tuple::Assignment assignment;
	unsigned cost;
	unsigned currentCost;
	const OptEnumPlan* left;
	const OptEnumPlan* right;
};


}} // namespace asdp::solution
