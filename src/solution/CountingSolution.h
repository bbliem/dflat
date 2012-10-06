#pragma once

#include <gmpxx.h>

#include <sharp/main>

namespace solution {

class CountingSolution : public sharp::Solution
{
public:
	typedef mpz_class CountType;
	CountingSolution(const CountType& count) : count(count) {}
	const CountType& getCount() const { return count; }

private:
	CountType count;
};

} // namespace solution
