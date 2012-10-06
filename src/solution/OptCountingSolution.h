#pragma once

#include <gmpxx.h>

#include <sharp/main>

namespace solution {

class OptCountingSolution : public sharp::Solution
{
public:
	typedef mpz_class CountType;
	OptCountingSolution(const CountType& count, unsigned cost) : count(count), cost(cost) {}
	const CountType& getCount() const { return count; }
	unsigned getCost() const { return cost; }

private:
	CountType count;
	unsigned cost;
};

} // namespace solution
