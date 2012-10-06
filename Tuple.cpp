#include <cassert>

#include "Tuple.h"
#include "Problem.h"

Tuple::~Tuple()
{
}


bool Tuple::operator<(const sharp::Tuple& rhs) const
{
	return m < ((Tuple&)rhs).m;
}

bool Tuple::operator==(const sharp::Tuple& rhs) const
{
	return m == ((Tuple&)rhs).m;
}

int Tuple::hash() const
{
	assert(false);
	return 0; // TODO
}

#ifdef VERBOSE
void Tuple::print(std::ostream& str) const
{
	str << "Tuple\tm: ";
	for(VertexSet::const_iterator i = m.begin(); i != m.end(); ++i)
		str << *i << ' ';
	str << std::endl;
}

void Tuple::print(std::ostream& str, Problem& problem) const
{
	str << "Tuple\tm: ";
	foreach(sharp::Vertex v, m) {
		if(problem.vertexIsRule(v))
			str << v << ' ';
		else
			str << problem.getVertexName(v) << ' ';
	}
	str << std::endl;
}
#endif
