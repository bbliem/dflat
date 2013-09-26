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

#include "DirectedAcyclicGraph.h"
#include "DecompositionNode.h"

// A decomposition is a (rooted) DAG where each node corresponds to a part of the instance.
// An instance of this class contains the root node and pointers to its children.
class Decomposition : public DirectedAcyclicGraph<DecompositionNode, std::vector<std::shared_ptr<Decomposition>>>
{
public:
	using DirectedAcyclicGraph::DirectedAcyclicGraph;
};
