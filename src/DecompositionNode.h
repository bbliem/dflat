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
#include <set>

#include "String.h"

class DecompositionNode
{
public:
	typedef String BagElement;
	typedef std::set<BagElement> Bag;

	DecompositionNode(const Bag& bag);

	const Bag& getBag() const;

	// Each DecompositionNode object that is created gets assigned a unique number starting from 1. This can, for instance, be used for printing when nodes should have unique names.
	unsigned int getGlobalId() const;

	// Print this node (no newlines)
	friend std::ostream& operator<<(std::ostream& os, const DecompositionNode& node);

private:
	unsigned int globalId;
	Bag bag;
};
