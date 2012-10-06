#pragma once

#include <set>
#include <sharp/main>

#include "../Tuple.h"

namespace threeCol {

struct Tuple : public ::Tuple
{
	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;

	virtual bool matches(const ::Tuple& other) const;
	virtual Tuple* join(const ::Tuple& other) const;
	virtual void declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const sharp::VertexSet& currentVertices) const;
	virtual bool isValid(const sharp::Problem&, const sharp::ExtendedHypertree& root) const;

#ifdef VERBOSE
	virtual void print(std::ostream&, const sharp::Problem&) const;
#endif	

	typedef std::set<sharp::Vertex> VertexSet;
	VertexSet red;
	VertexSet green;
	VertexSet blue;
};

} // namespace threeCol
