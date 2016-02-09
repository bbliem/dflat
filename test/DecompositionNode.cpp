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
#include <gtest/gtest.h>

#include "DecompositionNode.h"

TEST(DecompositionNode, ReturnsBag)
{
	Hypergraph::Vertices bag = {{"a"}, {"b"}};
	EXPECT_EQ(bag, DecompositionNode(bag).getBag());
}

TEST(DecompositionNode, HasUniqueGlobalId)
{
	DecompositionNode n1 = {{}};
	DecompositionNode n2 = {{}};

	EXPECT_TRUE(n1.getGlobalId() != n2.getGlobalId())
		<< "Global IDs of n1 and n2 are not different: " << n1.getGlobalId() << " and " << n2.getGlobalId();
}
