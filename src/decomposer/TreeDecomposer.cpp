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
#include <htd/AddEmptyLeavesOperation.hpp>
#include <htd/AddEmptyRootOperation.hpp>
#include <htd/CompressionOperation.hpp>
#include <htd/NormalizationOperation.hpp>
#include <htd/SemiNormalizationOperation.hpp>
#include <htd/WeakNormalizationOperation.hpp>
#include <htd/NamedHypergraph.hpp>
#include <htd/PathDecompositionFactory.hpp>
#include <htd/PathDecompositionAlgorithmFactory.hpp>
#include <htd/TreeDecompositionFactory.hpp>
#include <htd/TreeDecompositionAlgorithmFactory.hpp>
#include <htd/MinDegreeOrderingAlgorithm.hpp>
#include <htd/MinFillOrderingAlgorithm.hpp>
#include <htd/OrderingAlgorithmFactory.hpp>

#include "TreeDecomposer.h"
#include "../Instance.h"
#include "../Decomposition.h"
#include "../Application.h"

typedef htd::NamedHypergraph<String, String> Hypergraph;

namespace {
	Hypergraph buildNamedHypergraph(const Instance& instance)
	{
		Hypergraph graph;

		for(auto fact : instance.getEdgeFacts()) {
			for(const auto& e : fact.second) {
				std::vector<String> vs;
				for(auto v : e)
					vs.push_back(v);
				graph.addEdge(vs);
			}
		}

		return graph;
	}

	DecompositionPtr transformTd(htd::IMutableTreeDecomposition& decomposition, const Hypergraph& graph, bool addPostJoinNodes, const Application& app)
	{
		if(decomposition.root() == htd::Vertex::UNKNOWN)
			return DecompositionPtr{};

		auto htdRootBag = decomposition.bagContent(decomposition.root());
		DecompositionNode::Bag rootBag;
		for(auto v : htdRootBag)
			rootBag.insert(graph.vertexName(v));
		DecompositionPtr result{new Decomposition{rootBag, app.getSolverFactory()}};

		// If root is a join node, maybe add post join node
		DecompositionPtr rootOrPostJoinNode = result;

		if(addPostJoinNodes && decomposition.isJoinNode(decomposition.root())) {
			DecompositionPtr postJoin{new Decomposition{rootBag, app.getSolverFactory()}};
			postJoin->setPostJoinNode();
			rootOrPostJoinNode = postJoin;
			result->addChild(postJoin);
		}

		// Simulate recursion on htd's generated TD
		std::stack<std::pair<htd::vertex_t, DecompositionPtr>> stack;
		stack.push({decomposition.root(), rootOrPostJoinNode});

		while(stack.empty() == false) {
			htd::vertex_t htdParent = stack.top().first;
			DecompositionPtr parent = stack.top().second;
			stack.pop();
			size_t numChildren = decomposition.childCount(htdParent);
			for(size_t i = 0; i < numChildren; ++i) {
				htd::vertex_t htdChild = decomposition.child(htdParent, i);
				const auto htdChildBag = decomposition.bagContent(htdChild);
				DecompositionNode::Bag childBag;
				for(auto v : htdChildBag)
					childBag.insert(graph.vertexName(v));

				// Add post join node if necessary
				Decomposition* parentOrPostJoinNode = parent.get();

				if(addPostJoinNodes && decomposition.isJoinNode(htdChild)) {
					DecompositionPtr postJoin{new Decomposition{childBag, app.getSolverFactory()}};
					postJoin->setPostJoinNode();
					parentOrPostJoinNode = postJoin.get();
					parent->addChild(std::move(postJoin));
				}

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
	// Which algorithm to use?
	if(optEliminationOrdering.getValue() == "min-degree")
		htd::OrderingAlgorithmFactory::instance().setConstructionTemplate(new htd::MinDegreeOrderingAlgorithm());
	else {
		assert(optEliminationOrdering.getValue() == "min-fill");
		htd::OrderingAlgorithmFactory::instance().setConstructionTemplate(new htd::MinFillOrderingAlgorithm());
	}
	Hypergraph graph = buildNamedHypergraph(instance);

	// Use htd to decompose
	std::unique_ptr<htd::ITreeDecompositionAlgorithm> treeDecompositionAlgorithm;
	if(optPathDecomposition.isUsed())
		treeDecompositionAlgorithm.reset(htd::PathDecompositionAlgorithmFactory::instance().getPathDecompositionAlgorithm());
	else
		treeDecompositionAlgorithm.reset(htd::TreeDecompositionAlgorithmFactory::instance().getTreeDecompositionAlgorithm());
	std::unique_ptr<htd::ITreeDecomposition> decomposition{treeDecompositionAlgorithm->computeDecomposition(graph.internalGraph())};

	// Make mutable decomposition (for normalization etc.)
	std::unique_ptr<htd::IMutableTreeDecomposition> mutableDecomposition{htd::TreeDecompositionFactory::instance().getTreeDecomposition(*decomposition)};

	// Compress
	htd::CompressionOperation{}.apply(*mutableDecomposition);

	// Add empty leaves
	if(optNoEmptyLeaves.isUsed() == false)
		htd::AddEmptyLeavesOperation{}.apply(*mutableDecomposition);

	// Add empty root
	if(optNoEmptyRoot.isUsed() == false)
		htd::AddEmptyRootOperation{}.apply(*mutableDecomposition);

	// Normalize
	if(optNormalization.getValue() == "semi")
		htd::SemiNormalizationOperation{}.apply(*mutableDecomposition);
	else if(optNormalization.getValue() == "weak")
		htd::WeakNormalizationOperation{}.apply(*mutableDecomposition);
	else if(optNormalization.getValue() == "normalized")
		htd::NormalizationOperation{}.apply(*mutableDecomposition);

	// Transform htd's tree decomposition into our format
	DecompositionPtr result = transformTd(*mutableDecomposition, graph, optPostJoin.isUsed(), app);
	result->setRoot();
	return result;
}

} // namespace decomposer
