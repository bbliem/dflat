#pragma once

#include <sharp/main>

#include "../Tuple.h"

namespace solution {

class OptEnumSolution : public sharp::Solution
{
public:
	static OptEnumSolution* leaf(const Tuple::Assignment& leafSolution, unsigned cost);
	static OptEnumSolution* extend(OptEnumSolution* base, const Tuple::Assignment& extension, unsigned cost);
	static OptEnumSolution* unify(OptEnumSolution* left, OptEnumSolution* right, unsigned cost);
	static OptEnumSolution* join(OptEnumSolution* left, OptEnumSolution* right, unsigned cost);

	unsigned getCost() const;
	const std::set<Tuple::Assignment>& getSolutions() const;

private:
	OptEnumSolution();
	unsigned cost;
	std::set<Tuple::Assignment> assignments; // Contains (partial) solutions that have this cost
};

} // namespace solution
