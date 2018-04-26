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

#pragma once
//}}}
#include <ostream>
#include <vector>
#include <boost/dynamic_bitset.hpp>

// TODO we just have undirected graphs, so use a triangle and save half the space
// TODO write move constructor?
class AdjacencyMatrix
{
public:
	typedef unsigned int Index;

	AdjacencyMatrix();
	AdjacencyMatrix(unsigned int numRows);

	// create a new matrix from a given matrix where the given row and column is removed
	static AdjacencyMatrix remove(const AdjacencyMatrix& from, Index remove);

	// create a new matrix from a given matrix where a new row and column is added to the given index
	static AdjacencyMatrix introduce(const AdjacencyMatrix& to, Index at);

	unsigned int getNumRows() const { return numRows; }

	void reset();

	void set(Index i, Index j, bool value = true);

	bool operator()(Index i, Index j) const;

	// Compute transitive closure
	void makeTransitive();

	void bitwiseOr(const AdjacencyMatrix& other);

	// Print this adjacency matrix (no newlines)
	// If names is empty, prints indices.
	void printWithNames(std::ostream& os, const std::vector<unsigned>& names) const;

	bool isSymmetric() const;

	friend bool operator==(const AdjacencyMatrix& a, const AdjacencyMatrix& b);
	friend bool operator<(const AdjacencyMatrix& a, const AdjacencyMatrix& b);

	// Print this adjacency matrix (no newlines) with indices
	friend std::ostream& operator<<(std::ostream& os, const AdjacencyMatrix& m);

private:
	unsigned int numRows;
	unsigned int size; // numRows*numRows
	typedef boost::dynamic_bitset<> bitset;
	bitset matrix; // we use mpz_class as a big bit vector
};
