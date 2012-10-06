#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "TupleNP.h"
#include "Problem.h"

namespace asdp {

TupleNP::TupleNP()
	: currentCost(0), introducedCost(0)
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

bool TupleNP::matches(const ::Tuple& other) const
{
	return assignment == dynamic_cast<const TupleNP&>(other).assignment;
}

TupleNP* TupleNP::join(const ::Tuple& other) const
{
	return new TupleNP(*this);
}

void TupleNP::declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateSuffix) const
{
	out << "childTuple" << predicateSuffix << "(t" << &tupleAndSolution << ")." << std::endl;
	foreach(const Assignment::value_type& a, assignment)
		out << "mapped(t" << &tupleAndSolution << ',' << a.first << ',' << a.second << ")." << std::endl;
}

const TupleNP::Assignment& TupleNP::getAssignment() const
{
	return assignment;
}

unsigned int TupleNP::getCurrentCost() const
{
	return currentCost;
}

unsigned int TupleNP::getIntroducedCost() const
{
	return introducedCost;
}

#ifdef VERBOSE
void TupleNP::print(std::ostream& str) const
{
	str << "Tuple: ";
	foreach(const Assignment::value_type& a, assignment)
		str << a.first << '=' << a.second << ' ';
	str << std::endl;
}
#endif

} // namespace asdp
