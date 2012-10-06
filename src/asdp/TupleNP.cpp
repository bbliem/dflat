#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "TupleNP.h"
#include "Problem.h"

namespace asdp {

bool TupleNP::operator<(const sharp::Tuple& rhs) const
{
	return assignments < dynamic_cast<const TupleNP&>(rhs).assignments;
}

bool TupleNP::operator==(const sharp::Tuple& rhs) const
{
	return assignments == dynamic_cast<const TupleNP&>(rhs).assignments;
}

bool TupleNP::matches(const ::Tuple& other) const
{
	return assignments == dynamic_cast<const TupleNP&>(other).assignments;
}

TupleNP* TupleNP::join(const ::Tuple& other) const
{
	return new TupleNP(*this);
}

void TupleNP::declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const char* predicateSuffix) const
{
	out << "childTuple" << predicateSuffix << "(t" << &tupleAndSolution << ")." << std::endl;
	foreach(const Assignments::value_type& a, assignments)
		out << "mapped(t" << &tupleAndSolution << ',' << a.first << ',' << a.second << ")." << std::endl;
}

#ifdef VERBOSE
void TupleNP::print(std::ostream& str, const sharp::Problem& problem) const
{
	str << "Tuple: ";
	foreach(const Assignments::value_type& a, assignments)
		str << a.first << '=' << a.second << ' ';
	str << std::endl;
}
#endif

} // namespace asdp
