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
#include <memory>
#include <vector>

#include "DirectedAcyclicGraph.h"
#include "DecompositionNode.h"
#include "Solver.h"

class SolverFactory;
typedef std::shared_ptr<Decomposition> DecompositionPtr;

// A decomposition is a (rooted) DAG where each node corresponds to a part of the instance.
// An instance of this class contains the root node and pointers to its children.
// Additionally, it contains a solver that is responsible for producing the local item tree of the root.
class Decomposition : public DirectedAcyclicGraph<DecompositionNode, std::vector<DecompositionPtr>>
{
public:
	Decomposition(Node&& leaf, const SolverFactory& solverFactory);

	Solver& getSolver();

	// Returns true if the node is a join node, i.e., has at least two children and all children have equal bags
	bool isJoinNode() const;

	// Mark this node to be the root.
	// Only do this if this is not a child of any node.
	void setRoot(bool root = true);

	// Returns true if setRoot(true) has been called.
	bool isRoot() const;

	// Mark this node to be a node added for post-processing join nodes.
	void setPostJoinNode(bool postJoinNode = true);

	// Returns true if setPostJoinNode(true) has been called.
	bool isPostJoinNode() const;

	// Traverses the decomposition and returns its width
	int getWidth() const;

	// Print the decomposition in GraphML format to the given stream
	void printGraphMl(std::ostream& out) const;

protected:
	void printGraphMlElements(std::ostream& out) const;

	const SolverFactory& solverFactory;
	std::unique_ptr<Solver> solver;
	bool root;
	bool postJoinNode;
};
