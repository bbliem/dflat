#pragma once

#include <sharp/main>

namespace asdp { namespace solution {

struct CountingSolution : public sharp::Solution
{
	CountingSolution(size_t count) : count(count) {}
	size_t count;
};

}} // namespace asdp::solution
