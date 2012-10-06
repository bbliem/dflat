#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "Tuple.h"
#include "Problem.h"

namespace cyclic_ordering {

bool Tuple::operator<(const sharp::Tuple& rhs) const
{
	return ordering < dynamic_cast<const Tuple&>(rhs).ordering;
}

bool Tuple::operator==(const sharp::Tuple& rhs) const
{
	return ordering == dynamic_cast<const Tuple&>(rhs).ordering;
}

bool Tuple::matches(const ::Tuple& other) const
{
	return ordering == dynamic_cast<const Tuple&>(other).ordering;
}

Tuple* Tuple::join(const ::Tuple& other) const
{
	return new Tuple(*this);
}

void Tuple::declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const sharp::VertexSet& currentVertices) const
{
	assert(ordering.size() > 2);
	std::vector<sharp::Vertex>::const_iterator it = ordering.begin();
	// Find first bag element in ordering
	while(currentVertices.find(*it) == currentVertices.end()) {
		++it;
		if(it == ordering.end())
			return;
	}
	out << "childTuple(t" << &tupleAndSolution << ")." << std::endl;
	// Find next bag element in ordering
	for(sharp::Vertex predecessor = *it; it != ordering.end(); ++it) {
		// Only declare vertices that are in the current bag
		if(currentVertices.find(*it) == currentVertices.end())
			continue;

		out << "succ(t" << &tupleAndSolution << ",v" << predecessor << ",v" << *it << ")." << std::endl;
		predecessor = *it;
	}
}

bool Tuple::isValid(const sharp::Problem& problem, const sharp::ExtendedHypertree& root) const
{
	// We only generate valid tuples in the first place
	return true;	
}

#ifdef VERBOSE
void Tuple::print(std::ostream& str, ::Problem& problem) const
{
	str << "Tuple: ";
	foreach(sharp::Vertex v, ordering)
		str << problem.getVertexName(v) << '[' << v << "] ";
	str << std::endl;
}
#endif

} // namespace cyclic_ordering
