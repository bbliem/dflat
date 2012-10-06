#pragma once

#include <sharp/main>

#include "CountingSolution.h"

namespace asdp { namespace solution {

class CountingPlan : public sharp::Plan
{
public:
	static CountingPlan* leaf(const sharp::Tuple&);
	static CountingPlan* extend(const CountingPlan* base, const sharp::Tuple&);
	static CountingPlan* unify(const CountingPlan* left, const CountingPlan* right);
	static CountingPlan* join(const CountingPlan* left, const CountingPlan* right, const sharp::Tuple&);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	CountingPlan(const CountingSolution::CountType& count);
	CountingSolution::CountType count;
};

}} // namespace asdp::solution
