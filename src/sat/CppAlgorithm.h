#pragma once

#include "../Algorithm.h"

namespace sat {

class Problem;

class CppAlgorithm : public ::Algorithm
{
public:
	CppAlgorithm(Problem& problem);
	virtual ~CppAlgorithm();

protected:
	virtual sharp::TupleSet* exchangeLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed);
	virtual sharp::TupleSet* exchangeNonLeaf(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleSet& childTuples);
};

} // namespace sat
