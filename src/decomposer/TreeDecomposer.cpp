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

#include <cassert>
#include <sharp/Problem.hpp>
#include <sharp/ExtendedHypertree.hpp>
#include <sharp/BucketEliminationAlgorithm.hpp>
#include <sharp/AbstractEliminationOrdering.hpp>
#include <sharp/MinimumDegreeOrdering.hpp>
#include <sharp/MinimumFillOrdering.hpp>
#include <sharp/MaximumCardinalitySearchOrdering.hpp>

#include "TreeDecomposer.h"
#include "../Hypergraph.h"
#include "../Decomposition.h"
#include "../Application.h"
#include "../Debugger.h"

namespace {
	class SharpProblem : public sharp::Problem
	{
	public:
		SharpProblem(const Hypergraph& instance, sharp::AbstractHypertreeDecompositionAlgorithm& algorithm)
			: sharp::Problem(&algorithm)
			, instance(instance)
		{
		}

		virtual void parse() override {}

		virtual sharp::Hypergraph* buildHypergraphRepresentation() override
		{
			sharp::VertexSet vertices;
			sharp::HyperedgeSet hyperedges;

			for(auto v : instance.getVertices())
				vertices.insert(storeVertexName(v));

			for(auto e : instance.getEdges()) {
				sharp::VertexSet vs;
				for(auto v : e)
					vs.insert(getVertexId(v));
				hyperedges.insert(vs);
			}

			return createHypergraphFromSets(vertices, hyperedges);
		}

	private:
		const Hypergraph& instance;
	};

	DecompositionPtr transformTd(sharp::ExtendedHypertree& td, bool emptyLeaves, sharp::NormalizationType normalizationType, sharp::Problem& problem, const Application& app)
	{
		Hypergraph::Vertices bag;
		for(sharp::Vertex v : td.getVertices())
#ifdef DECOMPOSITION_COMPATIBILITY // Define this to generate the same decompositions as D-FLAT 0.2 when setting the same random seed
			bag.push_back(problem.getVertexName(v));
#else
			bag.insert(problem.getVertexName(v));
#endif

		DecompositionPtr result(new Decomposition(bag, app.getSolverFactory()));
		// If there are no children, optionally add empty leaves
		if(td.getChildren()->empty()) {
			if(emptyLeaves) {
				if(normalizationType == sharp::DefaultNormalization) {
					Decomposition* currentNode = result.get();
					while(bag.empty() == false) {
						bag.erase(bag.begin());
						DecompositionPtr child(new Decomposition(DecompositionNode(bag), app.getSolverFactory()));
						Decomposition* nextNode = child.get();
						currentNode->addChild(std::move(child));
						currentNode = nextNode;
					}
				}
				else
					result->addChild(DecompositionPtr(new Decomposition(DecompositionNode({}), app.getSolverFactory())));
			}
		}
		else {
			for(sharp::Hypertree* child : *td.getChildren())
				result->addChild(transformTd(*dynamic_cast<sharp::ExtendedHypertree*>(child), emptyLeaves, normalizationType, problem, app));
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
}

DecompositionPtr TreeDecomposer::decompose(const Hypergraph& instance) const
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
	std::unique_ptr<sharp::ExtendedHypertree> normalized(td->normalize(normalizationType));
	td.reset();

	// Transform SHARP's tree decomposition into our format
	DecompositionPtr transformed = transformTd(*normalized, !optNoEmptyLeaves.isUsed(), normalizationType, problem, app);

	// Optionally add empty root
	DecompositionPtr result;
	if(optNoEmptyRoot.isUsed())
		result = transformed;
	else {
		if(normalizationType == sharp::DefaultNormalization) {
			Hypergraph::Vertices bag = transformed->getRoot().getBag();
			while(bag.empty() == false) {
				bag.erase(bag.begin());
				result.reset(new Decomposition(DecompositionNode(bag), app.getSolverFactory()));
				result->addChild(std::move(transformed));
				transformed = result;
			}
		}
		else {
			result.reset(new Decomposition(DecompositionNode({}), app.getSolverFactory()));
			result->addChild(std::move(transformed));
		}
	}

	app.getDebugger().decomposerResult(*result);
	return result;
}

} // namespace decomposer
