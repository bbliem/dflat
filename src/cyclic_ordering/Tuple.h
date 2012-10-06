#pragma once

#include <sharp/main>

#include "../Tuple.h"

namespace cyclic_ordering {

struct Tuple : public ::Tuple
{
	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;

	virtual bool matches(const ::Tuple& other) const;
	virtual Tuple* join(const ::Tuple& other) const;
	//! Let the ordering be (x1,...,xn). Prints "succ(xi,xj)." for i < j s.t. xi and xj are in currentVertices and there is no k with i < k < j and xk is in currentVertices.
	virtual void declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const sharp::VertexSet& currentVertices) const;
	virtual bool isValid(const sharp::Problem&, const sharp::ExtendedHypertree& root) const;

#ifdef VERBOSE
	virtual void print(std::ostream&, class ::Problem&) const; // prints names instead of vertex numbers for atoms
#endif	

	std::vector<sharp::Vertex> ordering;
};

} // namespace cyclic_ordering
