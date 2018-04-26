/*{{{
Copyright 2012-2016, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/
//}}}
#include <cassert>

#include "AdjacencyMatrix.h"

AdjacencyMatrix::AdjacencyMatrix()
	: numRows(0)
	, size(0)
{
}

AdjacencyMatrix::AdjacencyMatrix(unsigned int numRows)
	: numRows(numRows)
	, size(numRows*numRows)
	, matrix(size)
{
}

AdjacencyMatrix AdjacencyMatrix::remove(const AdjacencyMatrix& a, Index remove)
{
	assert(a.isSymmetric());
	AdjacencyMatrix b(a.numRows-1);

	Index aIndex = 0;
	Index bIndex = 0;

	for(Index i = 0; i < a.numRows; ++i) {
		if(i == remove) {
			aIndex += a.numRows;
			continue;
		}

		for(Index j = 0; j < a.numRows; ++j) {
			if(j != remove) {
				b.matrix.set(bIndex, a.matrix[aIndex]);
				++bIndex;
			}
			++aIndex;
		}
	}
	assert(b.isSymmetric());
	assert(aIndex == a.matrix.size());
	assert(bIndex == b.matrix.size());

	return b;
}

AdjacencyMatrix AdjacencyMatrix::introduce(const AdjacencyMatrix& a, Index introduce)
{
	assert(a.isSymmetric());
	assert(introduce <= a.numRows);
	AdjacencyMatrix b(a.numRows+1);

	Index aIndex = 0;
	Index bIndex = 0;

	for(Index i = 0; i < introduce; ++i) {
		for(Index j = 0; j < introduce; ++j) {
			b.matrix.set(bIndex, a.matrix[aIndex]);
			++aIndex;
			++bIndex;
		}
		++bIndex;
		for(Index j = introduce; j < a.numRows; ++j) {
			b.matrix.set(bIndex, a.matrix[aIndex]);
			++aIndex;
			++bIndex;
		}
	}
	bIndex += b.numRows;
	for(Index i = introduce; i < a.numRows; ++i) {
		for(Index j = 0; j < introduce; ++j) {
			b.matrix.set(bIndex, a.matrix[aIndex]);
			++aIndex;
			++bIndex;
		}
		++bIndex;
		for(Index j = introduce; j < a.numRows; ++j) {
			b.matrix.set(bIndex, a.matrix[aIndex]);
			++aIndex;
			++bIndex;
		}
	}
	assert(b.isSymmetric());

	return b;
}

void AdjacencyMatrix::reset()
{
	matrix.reset();
}

void AdjacencyMatrix::set(unsigned int i, unsigned int j, bool value)
{
	assert(i < numRows && j < numRows);
	// XXX use triangle instead?
	assert(matrix[i*numRows + j] == matrix[j*numRows + i]);
	matrix[i*numRows+j] = value;
	matrix[j*numRows+i] = value;
	assert(isSymmetric());
}

bool AdjacencyMatrix::operator()(unsigned int i, unsigned int j) const
{
	assert(i < numRows && j < numRows);
	// XXX use triangle instead?
	return matrix[i*numRows+j];
}

void AdjacencyMatrix::makeTransitive()
{
	// Walshall's algorithm
	assert(isSymmetric());
	for(unsigned i = 0; i < numRows; ++i) {
		for(unsigned j = 0; j < numRows; ++j) {
			if(matrix[j*numRows+i]) { // XXX avoid conditional?
				for(unsigned k = 0; k < numRows; ++k) {
					matrix[j*numRows+k] = matrix[j*numRows+k] || matrix[i*numRows+k]; // XXX Use specialized implementations and fast bitwise operations if numRows <= sizeof(int)?
				}
			}
		}
	}
	assert(isSymmetric());
}

void AdjacencyMatrix::bitwiseOr(const AdjacencyMatrix& other)
{
	//const auto size = m.size();
	//assert(other.m.size() == size);
	//for(unsigned i = 0; i < size; ++i) {
	//    for(unsigned j = 0; j < size; ++j) {
	//        set(i, j, m[i][j] || other.m[i][j]);
	//    }
	//}
	matrix |= other.matrix;
}

void AdjacencyMatrix::printWithNames(std::ostream& os, const std::vector<unsigned>& names) const
{
	std::string sep;
	assert(names.empty() || size == 0 || names.size() == size);
	for(unsigned int i = 0; i < numRows; ++i) {
		for(unsigned int j = 0; j < i; ++j) {
			if(matrix[i*numRows+j]) {
				if(names.empty())
					os << sep << '(' << j << ',' << i << ')';
				else
					os << sep << '(' << names[j] << ',' << names[i] << ')';
				sep = ", ";
			}
		}
	}
}

bool AdjacencyMatrix::isSymmetric() const
{
	for(unsigned i = 0; i < numRows; ++i) {
		for(unsigned j = 0; j < numRows; ++j) {
			if(matrix[i*numRows+j] != matrix[j*numRows+i])
				return false;
		}
	}
	return true;
}

bool operator==(const AdjacencyMatrix& a, const AdjacencyMatrix& b)
{
	return a.matrix == b.matrix;
}

bool operator<(const AdjacencyMatrix& a, const AdjacencyMatrix& b)
{
	return a.matrix < b.matrix;
}

std::ostream& operator<<(std::ostream& os, const AdjacencyMatrix& m)
{
	m.printWithNames(os, {});

	return os;
}
