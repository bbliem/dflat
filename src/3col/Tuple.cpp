#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "Tuple.h"
#include "Problem.h"

namespace threeCol {

bool Tuple::operator<(const sharp::Tuple& rhs) const
{
	if(red < dynamic_cast<const Tuple&>(rhs).red)
		return true;
	if(red == dynamic_cast<const Tuple&>(rhs).red) { // XXX: Why compare twice?
		if(green < dynamic_cast<const Tuple&>(rhs).green)
			return true;
		if(green == dynamic_cast<const Tuple&>(rhs).green)
			return blue < dynamic_cast<const Tuple&>(rhs).blue;
	}
	return false;
}

bool Tuple::operator==(const sharp::Tuple& rhs) const
{
	return red == dynamic_cast<const Tuple&>(rhs).red && green == dynamic_cast<const Tuple&>(rhs).green && blue == dynamic_cast<const Tuple&>(rhs).blue;
}

bool Tuple::matches(const ::Tuple& other) const
{
	return *this == other;
}

Tuple* Tuple::join(const ::Tuple& other) const
{
	return new Tuple(*this);
}

void Tuple::declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const sharp::VertexSet& currentVertices) const
{
	out << "childTuple(t" << &tupleAndSolution << ")." << std::endl;
	foreach(sharp::Vertex v, red)
		out << "oldR(t" << &tupleAndSolution << ",v" << v << ")." << std::endl;
	foreach(sharp::Vertex v, green)
		out << "oldG(t" << &tupleAndSolution << ",v" << v << ")." << std::endl;
	foreach(sharp::Vertex v, blue)
		out << "oldB(t" << &tupleAndSolution << ",v" << v << ")." << std::endl;
}

bool Tuple::isValid(const sharp::Problem& problem, const sharp::ExtendedHypertree& root) const
{
	// We only generate valid tuples in the first place
	return true;
}

#ifdef VERBOSE
void Tuple::print(std::ostream& str, ::Problem& problem) const
{
	str << "Tuple: R: ";
	foreach(sharp::Vertex v, red)
		str << problem.getVertexName(v);
	std::cout << " G: ";
	foreach(sharp::Vertex v, green)
		str << problem.getVertexName(v);
	std::cout << " B: ";
	foreach(sharp::Vertex v, blue)
		str << problem.getVertexName(v);
	str << std::endl;
}
#endif

} // namespace threeCol
