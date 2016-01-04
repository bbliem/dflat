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

#include "Application.h"
#include "Decomposition.h"
#include "solver/dummy/SolverFactory.h"
#include "solver/dummy/Solver.h"

struct DecompositionTest : public ::testing::Test
{
	DecompositionTest()
	{
		addDecompositionChild(decomposition, {{{"b"}, {"c"}, {"d"}}});
	}

	Decomposition& addDecompositionChild(Decomposition& parent, DecompositionNode&& child)
	{
		parent.addChild(DecompositionPtr(new Decomposition(std::move(child), solverFactory)));
		return *parent.getChildren().back();
	}

	Application app{"test"};
	solver::dummy::SolverFactory solverFactory{app};
	Decomposition decomposition{DecompositionNode({{"a"}, {"b"}}), solverFactory};
};

TEST_F(DecompositionTest, ReturnsDummySolver)
{
	const Solver& result = decomposition.getSolver();
	EXPECT_EQ(typeid(solver::dummy::Solver), typeid(result))
		<< "decomposition.getSolver() should return an object of type solver::dummy::Solver";
}

TEST_F(DecompositionTest, ReportsCorrectWidth)
{
	EXPECT_EQ(2, decomposition.getWidth());
	addDecompositionChild(decomposition, {{{"e"}, {"f"}, {"g"}, {"h"}}});
	EXPECT_EQ(3, decomposition.getWidth());
}

TEST_F(DecompositionTest, IdentifiesJoinNodes)
{
	EXPECT_FALSE(decomposition.isJoinNode());
	const Hypergraph::Vertices bag = {{"x"}, {"y"}};
	Decomposition& joinNode = addDecompositionChild(decomposition, bag);
	addDecompositionChild(joinNode, bag);
	addDecompositionChild(joinNode, bag);
	EXPECT_EQ(2, joinNode.getChildren().size());

	EXPECT_TRUE(joinNode.isJoinNode());
	EXPECT_FALSE(decomposition.isJoinNode());
	for(const auto& child : joinNode.getChildren())
		EXPECT_FALSE(child->isJoinNode());
}

TEST_F(DecompositionTest, IsNotRootByDefault)
{
	Decomposition d = {DecompositionNode{{}}, solverFactory};
	EXPECT_FALSE(d.isRoot());
	d.setRoot();
	EXPECT_TRUE(d.isRoot());
}
