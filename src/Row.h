/*{{{
Copyright 2012-2016, Bernhard Bliem, Marius Moldovan
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
#include <cstddef>
#include <ostream>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <gmpxx.h>

#include "AdjacencyMatrix.h"

class Decomposition;

class Row
{
public:
	typedef std::shared_ptr<Row> ExtensionPointer;
	typedef std::vector<ExtensionPointer> ExtensionPointerTuple;
	typedef std::vector<ExtensionPointerTuple> ExtensionPointers;
	typedef std::vector<bool> VertexList;
	typedef std::set<std::pair<unsigned,unsigned>> EdgeSet;

	// extensionPointers may not be empty; if there are no decomposition
	// children, it should contain a 0-tuple.
	// Sets the count to the sum of the products of the counts for each
	// extension pointer tuple.
	// Sets the cost to 0.
	Row(AdjacencyMatrix&& selectedEdges, VertexList&& selectedVertices, AdjacencyMatrix&& connectedViaSelectedEdges, bool hasForgottenComponent, ExtensionPointers&& extensionPointers = {{}});

	const AdjacencyMatrix& getSelectedEdges() const { return selectedEdges; }
	const VertexList& getSelectedVertices() const { return selectedVertices; }
	const AdjacencyMatrix& getConnectedViaSelectedEdges() const { return connectedViaSelectedEdges; }
	bool getHasForgottenComponent() const { return hasForgottenComponent; }

	const ExtensionPointers& getExtensionPointers() const { return extensionPointers; }
	void clearExtensionPointers() { extensionPointers.clear(); }

	const mpz_class& getCount() const { return count; }

	unsigned long getCost() const { return cost; }
	void setCost(unsigned long cost);

	unsigned long getCurrentCost() const { return currentCost; }
	void setCurrentCost(unsigned long currentCost);

	// Unify extension pointers of this row with the other one's given that thy are equal.
	// "other" will subsequently be thrown away and only "this" will be retained.
	void merge(Row&& other);

	// Materialize just one extension.
	// Returns an adjacency list containing all selected edges.
	EdgeSet firstExtension(const Decomposition& node) const;

	// Print this row (no newlines)
	// If names is empty, prints indices.
	void printWithNames(std::ostream& os, const std::vector<unsigned>& names) const;

	// Print this row (no newlines) with indices
	friend std::ostream& operator<<(std::ostream& os, const Row& row);

private:
	ExtensionPointers extensionPointers;
	mpz_class count; // number of possible extensions of this row
	unsigned long cost = 0;
	unsigned long currentCost = 0;

	AdjacencyMatrix selectedEdges;
	VertexList selectedVertices; // For each bag element with index i, selectedVertices[i] is true iff the bag element is (was) incident to a (previously) selected edge.
	AdjacencyMatrix connectedViaSelectedEdges;
	bool hasForgottenComponent;

	// IMPORTANT: If you add/remove attributes, also update RowComparator!
};

std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Row>& row);
