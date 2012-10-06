#pragma once

#include <sharp/main>

#include "../../Tuple.h"

namespace asdp { namespace solution {

class OptimizationPlan : public sharp::Plan
{
public:
	static OptimizationPlan* leaf(const Tuple& tuple);
	static OptimizationPlan* extend(const OptimizationPlan* base, const Tuple& extension);
	static OptimizationPlan* unify(const OptimizationPlan* left, const OptimizationPlan* right);
	static OptimizationPlan* join(const OptimizationPlan* left, const OptimizationPlan* right);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	OptimizationPlan(Operation operation, const Tuple::Assignment& extension, unsigned cost, unsigned currentCost, const OptimizationPlan* left = 0, const OptimizationPlan* right = 0);
	OptimizationPlan(Operation operation, unsigned cost, unsigned currentCost, const OptimizationPlan* left, const OptimizationPlan* right);

	Tuple::Assignment extension;
	unsigned cost;
	unsigned currentCost;
	const OptimizationPlan* left;
	const OptimizationPlan* right;
};


}} // namespace asdp::solution
