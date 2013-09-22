/*
Copyright 2012-2013, Bernhard Bliem
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

#include <memory>
#include <vector>

#include "Node.h"

// A decomposition is a (rooted) DAG where each node corresponds to a part of the instance.
// An instance of this class contains the root node and pointers to its children.
// TODO Maybe make this abstract and derive TreeDecomposition?
class Decomposition
{
public:
	Decomposition(const Node& leaf);

	const Node& getRoot() const;

	// Adds the root of "child" to the list of children. Takes ownership of the whole subgraph rooted at "child".
	void addChild(Decomposition* child);

private:
	Node root;

	typedef std::shared_ptr<Decomposition> DecompositionPtr;
	typedef std::vector<DecompositionPtr> Children;
	Children children;
};
