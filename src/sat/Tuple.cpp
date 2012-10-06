#include <cassert>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "Tuple.h"
#include "Problem.h"

namespace sat {

bool Tuple::operator<(const sharp::Tuple& rhs) const
{
	if(atoms < dynamic_cast<const Tuple&>(rhs).atoms)
		return true;
	if(atoms == dynamic_cast<const Tuple&>(rhs).atoms) // XXX: Why compare twice?
		return clauses < dynamic_cast<const Tuple&>(rhs).clauses;
	return false;
}

bool Tuple::operator==(const sharp::Tuple& rhs) const
{
	return atoms == dynamic_cast<const Tuple&>(rhs).atoms && clauses == dynamic_cast<const Tuple&>(rhs).clauses;
}

bool Tuple::matches(const ::Tuple& other) const
{
	return atoms == dynamic_cast<const Tuple&>(other).atoms;
}

Tuple* Tuple::join(const ::Tuple& other) const
{
	Tuple* t = new Tuple(*this);
	t->clauses.insert(dynamic_cast<const Tuple&>(other).clauses.begin(), dynamic_cast<const Tuple&>(other).clauses.end());
	return t;
}

void Tuple::declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const sharp::VertexSet& currentVertices) const
{
	out << "childTuple(m" << &tupleAndSolution << ")." << std::endl;
	foreach(sharp::Vertex v, atoms)
		out << "oldMAtom(m" << &tupleAndSolution << ",v" << v << ")." << std::endl;
	foreach(sharp::Vertex v, clauses)
		out << "oldMClause(m" << &tupleAndSolution << ",v" << v << ")." << std::endl;
}

bool Tuple::isValid(const sharp::Problem& problem, const sharp::ExtendedHypertree& root) const
{
	// FIXME: This does not have to be done for each tuple but only once
//	unsigned int numClausesInRoot = 0;
//	foreach(sharp::Vertex v, root.getVertices())
//		if(dynamic_cast<const Problem&>(problem).vertexIsClause(v))
//			++numClausesInRoot;
//
//	// Suppose if the size is the same, the clauses are the same
//	return clauses.size() == numClausesInRoot;

	// TODO
	assert(false);
	return false;
}

#ifdef VERBOSE
void Tuple::print(std::ostream& str, const sharp::Problem& problem) const
{
	str << "Tuple: ";
	foreach(sharp::Vertex v, atoms)
		str << const_cast<sharp::Problem&>(problem).getVertexName(v) << '[' << v << "] ";
	foreach(sharp::Vertex v, clauses)
		str << v << ' ';
	str << std::endl;
}
#endif

} // namespace sat
