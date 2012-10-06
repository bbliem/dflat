#include <cassert>

#include "Tuple.h"
#include "Problem.h"

Tuple::~Tuple()
{
}


bool Tuple::operator<(const sharp::Tuple& rhs_) const
{
	const Tuple& rhs = dynamic_cast<const Tuple&>(rhs_);
	if(atoms < rhs.atoms)
		return true;
	if(atoms == rhs.atoms) // XXX: Why compare twice?
		return rules < rhs.rules;
	return false;
}

bool Tuple::operator==(const sharp::Tuple& rhs_) const
{
	const Tuple& rhs = dynamic_cast<const Tuple&>(rhs_);
	return atoms == rhs.atoms && rules == rhs.rules;
}

int Tuple::hash() const
{
	assert(false);
	return 0; // TODO
}

#ifdef VERBOSE
void Tuple::print(std::ostream& str, Problem& problem) const
{
	str << "Tuple: ";
	foreach(sharp::Vertex v, atoms)
		str << problem.getVertexName(v) << '[' << v << "] ";
	foreach(sharp::Vertex v, rules)
		str << v << ' ';
	str << std::endl;
}
#endif
