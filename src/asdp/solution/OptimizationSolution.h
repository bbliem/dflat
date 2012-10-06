#pragma once

#include <sharp/main>

#include "../../Tuple.h"

namespace asdp { namespace solution {

class OptimizationSolution : public sharp::Solution
{
public:
	static OptimizationSolution* leaf(const Tuple::Assignment& leafSolution, unsigned cost);
	static OptimizationSolution* extend(OptimizationSolution* base, const Tuple::Assignment& extension, unsigned cost);
	static OptimizationSolution* unify(OptimizationSolution* left, OptimizationSolution* right, unsigned cost);
	static OptimizationSolution* join(OptimizationSolution* left, OptimizationSolution* right, unsigned cost);

	unsigned getCost() const;
	const std::set<Tuple::Assignment>& getSolutions() const;

private:
	OptimizationSolution();
	unsigned cost;
	std::set<Tuple::Assignment> assignments; // Contains (partial) solutions that have this cost
};

}} // namespace asdp::solution
