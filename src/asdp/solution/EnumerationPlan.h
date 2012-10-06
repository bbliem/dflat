#pragma once

#include <sharp/main>

#include "../../Tuple.h"

namespace asdp { namespace solution {

class EnumerationPlan : public sharp::Plan
{
public:
	static EnumerationPlan* leaf(const Tuple& tuple);
	static EnumerationPlan* extend(const EnumerationPlan* base, const Tuple& extension);
	static EnumerationPlan* unify(const EnumerationPlan* left, const EnumerationPlan* right);
	static EnumerationPlan* join(const EnumerationPlan* left, const EnumerationPlan* right, const Tuple& joined);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	EnumerationPlan(Operation operation, const Tuple::Assignment& assignment, const EnumerationPlan* left = 0, const EnumerationPlan* right = 0);
	EnumerationPlan(const EnumerationPlan* left, const EnumerationPlan* right); // Union

	Tuple::Assignment assignment;
	const EnumerationPlan* left;
	const EnumerationPlan* right;
};

}} // namespace asdp::solution
