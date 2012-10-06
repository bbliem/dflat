#pragma once

#include <sharp/main>

#include "../Tuple.h"

namespace solution {

class EnumerationSolution : public sharp::Solution
{
public:
	static EnumerationSolution* leaf(const Tuple::Assignment& leafSolution);
	static EnumerationSolution* extend(EnumerationSolution* base, const Tuple::Assignment& extension);
	static EnumerationSolution* unify(EnumerationSolution* left, EnumerationSolution* right);
	static EnumerationSolution* join(EnumerationSolution* left, EnumerationSolution* right);

	const std::set<Tuple::Assignment>& getSolutions() const;

private:
	EnumerationSolution();
	std::set<Tuple::Assignment> assignments;
};

} // namespace solution
