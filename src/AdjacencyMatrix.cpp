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
{
}

AdjacencyMatrix::AdjacencyMatrix(unsigned int size)
	: m(size)
{
	for(auto& row : m)
		row.resize(size, false);
}

//AdjacencyMatrix::AdjacencyMatrix(const std::vector<String>& vertexNames)
//    : m(vertexNames.size())
//    , vertexNames(vertexNames)
//{
//    vertexIndices.reserve(vertexNames.size());
//
//    for(Index i = 0; i < vertexNames.size(); ++i) {
//        m[i].resize(vertexNames.size(), false);
//        vertexIndices[vertexNames[i]] = i;
//    }
//}

AdjacencyMatrix AdjacencyMatrix::remove(const AdjacencyMatrix& from, Index remove)
{
	assert(from.isSymmetric());
	AdjacencyMatrix newMatrix;
	const auto size = from.m.size();
	const auto newSize = size - 1;
	//newMatrix.vertexNames.reserve(newSize);
	//newMatrix.vertexNames.insert(newMatrix.vertexNames.end(), from.vertexNames.begin(), from.vertexNames.begin() + remove);
	//newMatrix.vertexNames.insert(newMatrix.vertexNames.end(), from.vertexNames.begin() + remove + 1, from.vertexNames.end());
	//newMatrix.buildVertexIndices();
	newMatrix.m.reserve(newSize);

	for(Index i = 0; i < size; ++i) {
		if(i == remove)
			continue;

		const auto& oldRow = from.m[i];
		assert(oldRow.size() == size);
		std::vector<bool> newRow;
		newRow.reserve(newSize);

		for(Index j = 0; j < size; ++j) {
			if(j != remove)
				newRow.push_back(oldRow[j]);
		}
		newMatrix.m.push_back(std::move(newRow));
	}
	assert(newMatrix.isSymmetric());

	return newMatrix;
}

//AdjacencyMatrix AdjacencyMatrix::introduce(const AdjacencyMatrix& to, String name)
AdjacencyMatrix AdjacencyMatrix::introduce(const AdjacencyMatrix& to, Index at)
{
	assert(to.isSymmetric());
	AdjacencyMatrix newMatrix;
	const auto size = to.m.size();
	const auto newSize = size + 1;
	//newMatrix.vertexNames.reserve(newSize);
	//newMatrix.vertexNames.insert(newMatrix.vertexNames.end(), to.vertexNames.begin(), to.vertexNames.end());
	//newMatrix.vertexNames.push_back(name);
	//newMatrix.buildVertexIndices();
	newMatrix.m.reserve(newSize);

	for(Index i = 0; i < at; ++i) {
		const auto& oldRow = to.m[i];
		assert(oldRow.size() == size);
		std::vector<bool> newRow;
		newRow.reserve(newSize);
		newRow.insert(newRow.end(), oldRow.begin(), oldRow.begin() + at);
		newRow.emplace_back();
		newRow.insert(newRow.end(), oldRow.begin() + at, oldRow.end());
		newMatrix.m.push_back(std::move(newRow));
	}
	newMatrix.m.emplace_back(newSize);
	for(Index i = at; i < size; ++i) {
		const auto& oldRow = to.m[i];
		assert(oldRow.size() == size);
		std::vector<bool> newRow;
		newRow.reserve(newSize);
		//newRow.insert(newRow.end(), oldRow.begin(), oldRow.end());
		//newRow.emplace_back();
		newRow.insert(newRow.end(), oldRow.begin(), oldRow.begin() + at);
		newRow.emplace_back();
		newRow.insert(newRow.end(), oldRow.begin() + at, oldRow.end());
		newMatrix.m.push_back(std::move(newRow));
	}
	assert(newMatrix.isSymmetric());

	return newMatrix;
}

void AdjacencyMatrix::reset()
{
	for(auto& row : m)
		std::fill(row.begin(), row.end(), false);
}

void AdjacencyMatrix::set(unsigned int i, unsigned int j, bool value)
{
	assert(i < m.size() && j < m[i].size());
	// XXX use triangle instead
	assert(m[i][j] == m[j][i]);
	assert(isSymmetric());
	m[i][j] = value;
	m[j][i] = value;
}

const std::vector<bool>& AdjacencyMatrix::getRow(Index i) const
{
	assert(i < m.size());
	return m[i];
}

bool AdjacencyMatrix::operator()(unsigned int i, unsigned int j) const
{
	assert(i < m.size() && j < m[i].size());
	// XXX use triangle instead?
	return m[i][j];
}

void AdjacencyMatrix::makeTransitive()
{
	// Walshall's algorithm
	assert(isSymmetric());
	const unsigned n = m.size();
	for(unsigned i = 0; i < n; ++i) {
		for(unsigned j = 0; j < n; ++j) {
			if(m[j][i]) { // XXX avoid conditional?
				for(unsigned k = 0; k < n; ++k) {
					m[j][k] = m[j][k] || m[i][k]; // XXX Use specialized implementations and fast bitwise operations if n <= sizeof(int)?
				}
			}
		}
	}
	assert(isSymmetric());
}

void AdjacencyMatrix::bitwiseOr(const AdjacencyMatrix& other)
{
	const auto size = m.size();
	assert(other.m.size() == size);
	for(unsigned i = 0; i < size; ++i) {
		for(unsigned j = 0; j < size; ++j) {
			set(i, j, m[i][j] || other.m[i][j]);
		}
	}
}

void AdjacencyMatrix::printWithNames(std::ostream& os, const std::vector<unsigned>& names) const
{
	std::string sep;
	const auto size = m.size();
	assert(names.empty() || size == 0 || names.size() == size);
	for(unsigned int i = 0; i < size; ++i) {
		assert(m[i].size() == size);
		for(unsigned int j = 0; j < i; ++j) {
			if(m[i][j]) {
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
	const unsigned n = m.size();
	for(unsigned i = 0; i < n; ++i) {
		if(m[i].size() != n)
			return false;
		for(unsigned j = 0; j < n; ++j) {
			if(m[i][j] != m[j][i])
				return false;
		}
	}
	return true;
}

bool operator==(const AdjacencyMatrix& a, const AdjacencyMatrix& b)
{
	return a.m == b.m;
}

bool operator<(const AdjacencyMatrix& a, const AdjacencyMatrix& b)
{
	return a.m < b.m;
}

std::ostream& operator<<(std::ostream& os, const AdjacencyMatrix& m)
{
	m.printWithNames(os, {});

	return os;
}
