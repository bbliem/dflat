#pragma once

#include <sharp/main>

namespace solution {

class DecisionPlan : public sharp::Plan
{
public:
	static DecisionPlan* leaf(const sharp::Tuple&);
	static DecisionPlan* extend(const DecisionPlan* base, const sharp::Tuple&);
	static DecisionPlan* unify(const DecisionPlan* left, const DecisionPlan* right);
	static DecisionPlan* join(const DecisionPlan* left, const DecisionPlan* right, const sharp::Tuple&);

protected:
	virtual sharp::Solution* materializeLeaf() const;
	virtual sharp::Solution* materializeExtension() const;
	virtual sharp::Solution* materializeUnion() const;
	virtual sharp::Solution* materializeJoin() const;

private:
	DecisionPlan();
};

} // namespace solution
