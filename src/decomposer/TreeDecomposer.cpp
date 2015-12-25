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
#include <cassert>
#include <sharp/Problem.hpp>
#include <sharp/ExtendedHypertree.hpp>
#include <sharp/BucketEliminationAlgorithm.hpp>
#include <sharp/AbstractEliminationOrdering.hpp>
#include <sharp/MinimumDegreeOrdering.hpp>
#include <sharp/MinimumFillOrdering.hpp>
#include <sharp/MaximumCardinalitySearchOrdering.hpp>

#include "TreeDecomposer.h"
#include "../Instance.h"
#include "../Decomposition.h"
#include "../Application.h"

namespace {
	class SharpProblem : public sharp::Problem
	{
	public:
		SharpProblem(const Instance& instance, sharp::AbstractHypertreeDecompositionAlgorithm& algorithm)
			: sharp::Problem(&algorithm)
			, instance(instance)
		{
		}

		virtual void parse() override {}

		virtual sharp::Hypergraph* buildHypergraphRepresentation() override
		{
			sharp::VertexSet vertices;
			sharp::HyperedgeSet hyperedges;

			for(auto fact : instance.getEdgeFacts()) {
				for(const auto& e : fact.second) {
					sharp::VertexSet vs;
					for(auto v : e) {
						auto id = storeVertexName(*v);
						vertices.insert(id);
						vs.insert(id);
					}
					hyperedges.insert(vs);
				}
			}

			return createHypergraphFromSets(vertices, hyperedges);
		}

	private:
		const Instance& instance;
	};

	DecompositionPtr transformTd(sharp::ExtendedHypertree& td, bool addPostJoinNodes, sharp::NormalizationType normalizationType, sharp::Problem& problem, const Application& app)
	{
		DecompositionNode::Bag bag;
		for(sharp::Vertex v : td.getVertices())
			bag.insert(problem.getVertexName(v));

		DecompositionPtr result(new Decomposition(bag, app.getSolverFactory()));

		for(sharp::Hypertree* child : *td.getChildren())
			result->addChild(transformTd(*dynamic_cast<sharp::ExtendedHypertree*>(child), addPostJoinNodes, normalizationType, problem, app));

		// XXX Join post processing nodes currently lead to weird node numbering
		if(addPostJoinNodes && result->isJoinNode()) {
			DecompositionPtr joinNode = std::move(result);
			result.reset(new Decomposition(bag, app.getSolverFactory()));
			result->addChild(std::move(joinNode));
			result->setPostJoinNode();
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
{
	optNormalization.addCondition(selected);
	optNormalization.addChoice("none", "No normalization", true);
	optNormalization.addChoice("weak", "Weak normalization");
	optNormalization.addChoice("semi", "Semi-normalization");
	optNormalization.addChoice("normalized", "Normalization");
	app.getOptionHandler().addOption(optNormalization, OPTION_SECTION);

	optEliminationOrdering.addCondition(selected);
	optEliminationOrdering.addChoice("min-degree", "Minimum degree ordering", true);
	optEliminationOrdering.addChoice("min-fill", "Minimum fill ordering");
	optEliminationOrdering.addChoice("mcs", "Maximum cardinality search");
	app.getOptionHandler().addOption(optEliminationOrdering, OPTION_SECTION);

	optNoEmptyRoot.addCondition(selected);
	app.getOptionHandler().addOption(optNoEmptyRoot, OPTION_SECTION);

	optNoEmptyLeaves.addCondition(selected);
	app.getOptionHandler().addOption(optNoEmptyLeaves, OPTION_SECTION);

	optPostJoin.addCondition(selected);
	app.getOptionHandler().addOption(optPostJoin, OPTION_SECTION);
}

DecompositionPtr TreeDecomposer::decompose(const Instance& instance) const
{
	// Which algorithm to use?
	sharp::AbstractEliminationOrdering* ordering;
	if(optEliminationOrdering.getValue() == "min-degree")
		ordering = new sharp::MinimumDegreeOrdering;
	else if(optEliminationOrdering.getValue() == "min-fill")
		ordering = new sharp::MinimumFillOrdering;
	else {
		assert(optEliminationOrdering.getValue() == "mcs");
		ordering = new sharp::MaximumCardinalitySearchOrdering;
	}
	sharp::BucketEliminationAlgorithm algorithm(ordering);

	// Use SHARP to decompose
	SharpProblem problem(instance, algorithm);
	std::unique_ptr<sharp::ExtendedHypertree> td(problem.calculateHypertreeDecomposition());
	assert(td);

	// Normalize
	sharp::NormalizationType normalizationType;
	if(optNormalization.getValue() == "semi")
		normalizationType = sharp::SemiNormalization;
	else if(optNormalization.getValue() == "weak")
		normalizationType = sharp::WeakNormalization;
	else if(optNormalization.getValue() == "normalized")
		normalizationType = sharp::DefaultNormalization;
	else
		normalizationType = sharp::NoNormalization;
	std::unique_ptr<sharp::ExtendedHypertree> normalized(td->normalize(normalizationType, !optNoEmptyLeaves.isUsed(), !optNoEmptyRoot.isUsed()));
	td.reset();

	// Transform SHARP's tree decomposition into our format
	DecompositionPtr result = transformTd(*normalized, optPostJoin.isUsed(), normalizationType, problem, app);
	result->setRoot();
	return result;
}

} // namespace decomposer
