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

#include <ostream>

#include "Hypergraph.h"

// Node of a Decomposition
class Node
{
public:
	Node(const Hypergraph::Vertices& bag);

	const Hypergraph::Vertices& getBag() const;

	// Each Node object that is created gets assigned a unique number starting from 1. This can, for instance, be used for printing when nodes should have unique names.
	int getGlobalId() const;

	// Print node (no EOLs)
	friend std::ostream& operator<<(std::ostream& os, const Node& node);

private:
	Hypergraph::Vertices bag;
	int globalId;
};
