/*{{{
Copyright 2012-2015, Bernhard Bliem
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
#include <iostream>

#include "CountRows.h"
#include "../Decomposition.h"
#include "../Application.h"

namespace {

	size_t countNodes(const ItemTree& tree)
	{
		size_t nodes = 1;

		for(const auto& child : tree.getChildren())
			nodes += countNodes(*child);

		return nodes;
	}

} // anonymous namespace

namespace printer {

CountRows::CountRows(Application& app, bool newDefault)
	: Printer(app, "count-rows", "Number of rows per decomposition node", newDefault)
{
}

void CountRows::solverInvocationResult(const Decomposition& decompositionNode, const ItemTree* result)
{
	std::cout << "Number of rows at decomposition node " << decompositionNode.getNode().getGlobalId() << ": ";
	if(result)
		std::cout << countNodes(*result);
	else
		std::cout << '0';
	std::cout << std::endl;
}

} // namespace printer
