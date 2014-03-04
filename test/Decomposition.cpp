#include <gtest/gtest.h>

#include "Application.h"
#include "Decomposition.h"
#include "solver/dummy/SolverFactory.h"
#include "solver/dummy/Solver.h"

struct DecompositionTest : public ::testing::Test
{
	DecompositionTest()
	{
		addDecompositionChild(decomposition, {{"b", "c", "d"}});
	}

	Decomposition& addDecompositionChild(Decomposition& parent, DecompositionNode&& child)
	{
		parent.addChild(DecompositionPtr(new Decomposition(std::move(child), solverFactory)));
		return *parent.getChildren().back();
	}

	Application app{"test"};
	solver::dummy::SolverFactory solverFactory{app};
	Decomposition decomposition{DecompositionNode({"a", "b"}), solverFactory};
};

TEST_F(DecompositionTest, ReturnsDummySolver)
{
	EXPECT_EQ(typeid(solver::dummy::Solver), typeid(decomposition.getSolver()))
		<< "decomposition.getSolver() should return an object of type solver::dummy::Solver";
}

TEST_F(DecompositionTest, ReportsCorrectWidth)
{
	EXPECT_EQ(2, decomposition.getWidth());
	addDecompositionChild(decomposition, {{"e", "f", "g", "h"}});
	EXPECT_EQ(3, decomposition.getWidth());
}

TEST_F(DecompositionTest, IdentifiesJoinNodes)
{
	EXPECT_FALSE(decomposition.isJoinNode());
	const Hypergraph::Vertices bag = {"x", "y"};
	Decomposition& joinNode = addDecompositionChild(decomposition, bag);
	addDecompositionChild(joinNode, bag);
	addDecompositionChild(joinNode, bag);
	EXPECT_EQ(2, joinNode.getChildren().size());

	EXPECT_TRUE(joinNode.isJoinNode());
	EXPECT_FALSE(decomposition.isJoinNode());
	for(const auto& child : joinNode.getChildren())
		EXPECT_FALSE(child->isJoinNode());
}
