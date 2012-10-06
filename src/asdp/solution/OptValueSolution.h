#pragma once

#include <sharp/main>

namespace asdp { namespace solution {

class OptValueSolution : public sharp::Solution
{
	// If there is a OptValueSolution, the problem instance is positive
public:
	OptValueSolution(unsigned cost) : cost(cost) {};
	unsigned getCost() const { return cost; }

private:
	unsigned cost;
};

}} // namespace asdp::solution
