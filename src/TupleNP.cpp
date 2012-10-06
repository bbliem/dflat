#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "TupleNP.h"
#include "Problem.h"

TupleNP::TupleNP()
	: currentCost(0), cost(0)
{
}

bool TupleNP::operator<(const sharp::Tuple& rhs) const
{
	return assignment < dynamic_cast<const TupleNP&>(rhs).assignment;
}

bool TupleNP::operator==(const sharp::Tuple& rhs) const
{
	return assignment == dynamic_cast<const TupleNP&>(rhs).assignment;
}

void TupleNP::unify(const sharp::Tuple& old)
{
	assert(currentCost == dynamic_cast<const TupleNP&>(old).currentCost);
	cost = std::min(cost, dynamic_cast<const TupleNP&>(old).cost);
}

bool TupleNP::matches(const Tuple& other) const
{
	return assignment == dynamic_cast<const TupleNP&>(other).assignment;
}

TupleNP* TupleNP::join(const Tuple& other) const
{
	// Since according to matches() the assignments must coincide, we suppose equal currentCost
	assert(currentCost == dynamic_cast<const TupleNP&>(other).currentCost);
	assert(cost >= currentCost);
	assert(dynamic_cast<const TupleNP&>(other).cost >= dynamic_cast<const TupleNP&>(other).currentCost);

	TupleNP* t = new TupleNP(*this);
	// currentCost is contained in both left->cost and right->cost, so subtract it once
	t->cost = (this->cost - currentCost) + dynamic_cast<const TupleNP&>(other).cost;
	return t;
}

void TupleNP::declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateSuffix) const
{
	out << "childTuple" << predicateSuffix << "(t" << &tupleAndSolution << ")." << std::endl;
	foreach(const Assignment::value_type& a, assignment)
		out << "mapped(t" << &tupleAndSolution << ',' << a.first << ',' << a.second << ")." << std::endl;
	out << "childCost(t" << &tupleAndSolution << ',' << cost << ")." << std::endl;
}

const TupleNP::Assignment& TupleNP::getAssignment() const
{
	return assignment;
}

unsigned int TupleNP::getCurrentCost() const
{
	return currentCost;
}

unsigned int TupleNP::getCost() const
{
	return cost;
}

#ifdef VERBOSE
void TupleNP::print(std::ostream& str) const
{
	str << "Tuple: ";
	foreach(const Assignment::value_type& a, assignment)
		str << a.first << '=' << a.second << ' ';
	str << "(cost " << cost << ")" << std::endl;
}
#endif
