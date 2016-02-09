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
#include "Dummy.h"
#include "../Decomposition.h"
#include "../Application.h"
#include "../Printer.h"

namespace decomposer {

Dummy::Dummy(Application& app, bool newDefault)
	: Decomposer(app, "dummy", "Do not decompose", newDefault)
{
}

DecompositionPtr Dummy::decompose(const Instance& instance) const
{
	// Create set of all vertices
	DecompositionNode::Bag vertices;
	for(const auto edgeFacts : instance.getEdgeFacts())
		for(const auto& arguments : edgeFacts.second)
			vertices.insert(arguments.begin(), arguments.end());

	DecompositionPtr result(new Decomposition(vertices, app.getSolverFactory()));
	result->setRoot();
	return result;
}

} // namespace decomposer
