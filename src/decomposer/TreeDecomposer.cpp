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
#include <cassert>
#include <stack>
#include <htd/main.hpp>

#include "TreeDecomposer.h"
#include "../Instance.h"
#include "../Decomposition.h"
#include "../Application.h"

typedef htd::NamedHypergraph<std::string, std::string> Hypergraph;

namespace {
	enum class FitnessCriterion
	{
		WIDTH,
		MEDIAN_JOIN_NODE_BAG_SIZE,
		AVERAGE_JOIN_NODE_BAG_SIZE,
		NUM_JOIN_NODES,
	};

	class FitnessFunction : public htd::ITreeDecompositionFitnessFunction
	{
		public:
			FitnessFunction(FitnessCriterion criterion) : criterion(criterion) {}

			virtual htd::FitnessEvaluation* fitness(const htd::IMultiHypergraph&, const htd::ITreeDecomposition& decomposition) const override
			{
				switch(criterion) {
					case FitnessCriterion::WIDTH:
						return new htd::FitnessEvaluation(1, -static_cast<double>(decomposition.maximumBagSize()));
					case FitnessCriterion::NUM_JOIN_NODES:
						return new htd::FitnessEvaluation(1, -decomposition.joinNodeCount());
					default:
						break;
				}

				std::vector<htd::vertex_t> joinNodes;
				decomposition.copyJoinNodesTo(joinNodes);


				double medianJoinNodeBagSize = 0.0;
				double averageJoinNodeBagSize = 0.0;

				if(!joinNodes.empty()) {
					std::vector<double> bagSizes;
					bagSizes.reserve(joinNodes.size());

					for(htd::vertex_t joinNode : joinNodes) {
						double bagSize = decomposition.bagSize(joinNode);
						bagSizes.push_back(bagSize);
						averageJoinNodeBagSize += bagSize;
					}

					averageJoinNodeBagSize /= joinNodes.size();
					std::sort(bagSizes.begin(), bagSizes.end());

					if(bagSizes.size() % 2 == 0)
						medianJoinNodeBagSize = (bagSizes[bagSizes.size() / 2 - 1] + bagSizes[bagSizes.size() / 2]) / 2;
					else
						medianJoinNodeBagSize = bagSizes[bagSizes.size() / 2];
				}

				switch(criterion) {
					case FitnessCriterion::MEDIAN_JOIN_NODE_BAG_SIZE:
						return new htd::FitnessEvaluation(1, -medianJoinNodeBagSize);
					case FitnessCriterion::AVERAGE_JOIN_NODE_BAG_SIZE:
						return new htd::FitnessEvaluation(1, -averageJoinNodeBagSize);
					default:
						throw std::logic_error("Unexpected fitness criterion");
				}
			}

			virtual FitnessFunction* clone() const override { return new FitnessFunction(criterion); }

		private:
			FitnessCriterion criterion;
	};

	Hypergraph buildNamedHypergraph(const htd::LibraryInstance& htd, const Instance& instance)
	{
		Hypergraph graph(&htd);

		for(auto fact : instance.getEdgeFacts()) {
			for(const auto& e : fact.second) {
				std::vector<std::string> vs;
				for(auto v : e)
					vs.push_back(*v);
				graph.addEdge(vs);
			}
		}

		return graph;
	}

	DecompositionPtr transformTd(htd::ITreeDecomposition& decomposition, const Hypergraph& graph, const Application& app)
	{
		if(decomposition.root() == htd::Vertex::UNKNOWN)
			return DecompositionPtr{};

		auto htdRootBag = decomposition.bagContent(decomposition.root());
		DecompositionNode::Bag rootBag;
		for(auto v : htdRootBag)
			rootBag.emplace(std::string{graph.vertexName(v)});
		DecompositionPtr result{new Decomposition{rootBag, app.getSolverFactory()}};

		// If root is a join node, maybe add post join node
		DecompositionPtr rootOrPostJoinNode = result;

		// Simulate recursion on htd's generated TD
		std::stack<std::pair<htd::vertex_t, DecompositionPtr>> stack;
		stack.push({decomposition.root(), rootOrPostJoinNode});

		while(stack.empty() == false) {
			htd::vertex_t htdParent = stack.top().first;
			DecompositionPtr parent = stack.top().second;
			stack.pop();
			size_t numChildren = decomposition.childCount(htdParent);
			for(size_t i = 0; i < numChildren; ++i) {
				htd::vertex_t htdChild = decomposition.childAtPosition(htdParent, i);
				const auto htdChildBag = decomposition.bagContent(htdChild);
				DecompositionNode::Bag childBag;
				for(auto v : htdChildBag)
					childBag.emplace(std::string{graph.vertexName(v)});

				// Add post join node if necessary
				Decomposition* parentOrPostJoinNode = parent.get();

				DecompositionPtr child{new Decomposition{childBag, app.getSolverFactory()}};
				parentOrPostJoinNode->addChild(child);
				stack.push({htdChild, child});
			}
		}

		return result;
	}
}

namespace decomposer {

const std::string TreeDecomposer::OPTION_SECTION = "Tree decomposition";

TreeDecomposer::TreeDecomposer(Application& app, bool newDefault)
	: Decomposer(app, "td", "Tree decomposition (bucket elimination)", newDefault)
	, optNormalization("n", "normalization", "Use normal form <normalization> for the tree decomposition")
	, optEliminationOrdering("elimination", "h", "Use heuristic <h> for bucket elimination")
	, optFitnessCriterion("fitness", "criterion", "From several generated TD's choose one with best <criterion>")
	, optNoEmptyRoot("no-empty-root", "Do not add an empty root to the tree decomposition")
	, optNoEmptyLeaves("no-empty-leaves", "Do not add empty leaves to the tree decomposition")
	, optPostJoin("post-join", "To each join node, add a parent with identical bag")
	, optPathDecomposition("path-decomposition", "Generate a path decomposition")
{
	optNormalization.addCondition(selected);
	optNormalization.addChoice("none", "No normalization", true);
	optNormalization.addChoice("weak", "Weak normalization");
	optNormalization.addChoice("semi", "Semi-normalization");
	optNormalization.addChoice("normalized", "Normalization");
	app.getOptionHandler().addOption(optNormalization, OPTION_SECTION);

	optEliminationOrdering.addCondition(selected);
	optEliminationOrdering.addChoice("min-fill", "Minimum fill ordering", true);
	optEliminationOrdering.addChoice("min-degree", "Minimum degree ordering");
	app.getOptionHandler().addOption(optEliminationOrdering, OPTION_SECTION);

	optFitnessCriterion.addCondition(selected);
	optFitnessCriterion.addChoice("width", "Maximum bag size", true);
	optFitnessCriterion.addChoice("join-bag-avg", "Average join node bag size");
	optFitnessCriterion.addChoice("join-bag-median", "Median join node bag size");
	optFitnessCriterion.addChoice("num-joins", "Number of join nodes");
	app.getOptionHandler().addOption(optFitnessCriterion, OPTION_SECTION);

	optNoEmptyRoot.addCondition(selected);
	app.getOptionHandler().addOption(optNoEmptyRoot, OPTION_SECTION);

	optNoEmptyLeaves.addCondition(selected);
	app.getOptionHandler().addOption(optNoEmptyLeaves, OPTION_SECTION);

	optPostJoin.addCondition(selected);
	app.getOptionHandler().addOption(optPostJoin, OPTION_SECTION);

	optPathDecomposition.addCondition(selected);
	app.getOptionHandler().addOption(optPathDecomposition, OPTION_SECTION);
}

DecompositionPtr TreeDecomposer::decompose(const Instance& instance) const
{
	std::unique_ptr<htd::LibraryInstance> htd(htd::createManagementInstance(htd::Id::FIRST));

	// Which algorithm to use?
	if(optEliminationOrdering.getValue() == "min-degree")
		htd->orderingAlgorithmFactory().setConstructionTemplate(new htd::MinDegreeOrderingAlgorithm(htd.get()));
	else {
		assert(optEliminationOrdering.getValue() == "min-fill");
		htd->orderingAlgorithmFactory().setConstructionTemplate(new htd::MinFillOrderingAlgorithm(htd.get()));
	}

	Hypergraph graph = buildNamedHypergraph(*htd, instance);
	std::unique_ptr<htd::TreeDecompositionOptimizationOperation> operation(new htd::TreeDecompositionOptimizationOperation(htd.get()));
	operation->setManagementInstance(htd.get());

	// Add transformation to path decomposition
	if(optPathDecomposition.isUsed())
		operation->addManipulationOperation(new htd::JoinNodeReplacementOperation(htd.get()));

	// Add empty leaves
	if(optNoEmptyLeaves.isUsed() == false)
		operation->addManipulationOperation(new htd::AddEmptyLeavesOperation(htd.get()));

	// Add empty root
	if(optNoEmptyRoot.isUsed() == false)
		operation->addManipulationOperation(new htd::AddEmptyRootOperation(htd.get()));

	// Normalize
	if(optNormalization.getValue() == "semi")
		operation->addManipulationOperation(new htd::SemiNormalizationOperation(htd.get()));
	else if(optNormalization.getValue() == "weak")
		operation->addManipulationOperation(new htd::WeakNormalizationOperation(htd.get()));
	else if(optNormalization.getValue() == "normalized")
		operation->addManipulationOperation(new htd::NormalizationOperation(htd.get()));

	if(optPostJoin.isUsed())
		operation->addManipulationOperation(new htd::AddIdenticalJoinNodeParentOperation(htd.get()));

	// Set up fitness function to find a "good" TD
	FitnessCriterion fitnessCriterion;
	if(optFitnessCriterion.getValue() == "join-bag-avg")
		fitnessCriterion = FitnessCriterion::AVERAGE_JOIN_NODE_BAG_SIZE;
	else if(optFitnessCriterion.getValue() == "join-bag-median")
		fitnessCriterion = FitnessCriterion::MEDIAN_JOIN_NODE_BAG_SIZE;
	else if(optFitnessCriterion.getValue() == "num-joins")
		fitnessCriterion = FitnessCriterion::NUM_JOIN_NODES;
	else {
		assert(optFitnessCriterion.getValue() == "width");
		fitnessCriterion = FitnessCriterion::WIDTH;
	}
	FitnessFunction fitnessFunction(fitnessCriterion);

	std::unique_ptr<htd::ITreeDecompositionAlgorithm> baseAlgorithm(htd->treeDecompositionAlgorithmFactory().createInstance());
	baseAlgorithm->addManipulationOperation(operation.release());
	htd::IterativeImprovementTreeDecompositionAlgorithm algorithm(htd.get(), baseAlgorithm.release(), fitnessFunction);
	algorithm.setIterationCount(100);
	algorithm.setNonImprovementLimit(25);

	// Compute decomposition
	std::unique_ptr<htd::ITreeDecomposition> decomposition{algorithm.computeDecomposition(graph.internalGraph())};

	// Transform htd's tree decomposition into our format
	DecompositionPtr result = transformTd(*decomposition, graph, app);
	result->setRoot();
	return result;
}

} // namespace decomposer
