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
		return rules < dynamic_cast<const Tuple&>(rhs).rules;
	return false;
}

bool Tuple::operator==(const sharp::Tuple& rhs) const
{
	return atoms == dynamic_cast<const Tuple&>(rhs).atoms && rules == dynamic_cast<const Tuple&>(rhs).rules;
}

bool Tuple::matches(const ::Tuple& other) const
{
	return atoms == dynamic_cast<const Tuple&>(other).atoms;
}

Tuple* Tuple::join(const ::Tuple& other) const
{
	Tuple* t = new Tuple(*this);
	t->rules.insert(dynamic_cast<const Tuple&>(other).rules.begin(), dynamic_cast<const Tuple&>(other).rules.end());
	return t;
}

void Tuple::declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution) const
{
	out << "childTuple(m" << &tupleAndSolution << ")." << std::endl;
	foreach(sharp::Vertex v, atoms)
		out << "oldMAtom(m" << &tupleAndSolution << ",v" << v << ")." << std::endl;
	foreach(sharp::Vertex v, rules)
		out << "oldMRule(m" << &tupleAndSolution << ",v" << v << ")." << std::endl;
}

bool Tuple::isValid(const sharp::Problem& problem, const sharp::ExtendedHypertree& root) const
{
	// FIXME: This does not have to be done for each tuple but only once
	unsigned int numRulesInRoot = 0;
	foreach(sharp::Vertex v, root.getVertices())
		if(dynamic_cast<const Problem&>(problem).vertexIsRule(v))
			++numRulesInRoot;

	// Suppose if the size is the same, the rules are the same
	return rules.size() == numRulesInRoot;
}

#ifdef VERBOSE
void Tuple::print(std::ostream& str, ::Problem& problem) const
{
	str << "Tuple: ";
	foreach(sharp::Vertex v, atoms)
		str << problem.getVertexName(v) << '[' << v << "] ";
	foreach(sharp::Vertex v, rules)
		str << v << ' ';
	str << std::endl;
}
#endif

} // namespace sat
