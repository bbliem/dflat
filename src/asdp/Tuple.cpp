#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "Tuple.h"
#include "Problem.h"

namespace asdp {

bool Tuple::operator<(const sharp::Tuple& rhs) const
{
	return assignments < dynamic_cast<const Tuple&>(rhs).assignments;
}

bool Tuple::operator==(const sharp::Tuple& rhs) const
{
	return assignments == dynamic_cast<const Tuple&>(rhs).assignments;
}

bool Tuple::matches(const ::Tuple& other) const
{
	return assignments == dynamic_cast<const Tuple&>(other).assignments;
}

Tuple* Tuple::join(const ::Tuple& other) const
{
	return new Tuple(*this);
}

void Tuple::declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const sharp::VertexSet& currentVertices) const
{
	out << "childTuple(t" << &tupleAndSolution << ")." << std::endl;
	foreach(const Assignments::value_type& a, assignments)
		out << "mapped(t" << &tupleAndSolution << ',' << a.first << ',' << a.second << ")." << std::endl;
}

bool Tuple::isValid(const sharp::Problem& problem, const sharp::ExtendedHypertree& root) const
{
	// TODO
	return true;
}

#ifdef VERBOSE
void Tuple::print(std::ostream& str, const sharp::Problem& problem) const
{
	str << "Tuple: ";
	foreach(const Assignments::value_type& a, assignments)
		str << a.first << '=' << a.second << ' ';
	str << std::endl;
}
#endif

} // namespace asdp
