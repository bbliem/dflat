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

	Decomposition* transformTd(sharp::ExtendedHypertree* td, sharp::Problem& problem)
	{
		Hypergraph::Vertices rootBag;
		for(sharp::Vertex v : td->getVertices())
			rootBag.insert(problem.getVertexName(v));

		Decomposition* transformedTd = new Decomposition(Node(rootBag));
		for(sharp::Hypertree* child : *td->getChildren())
			transformedTd->addChild(transformTd(dynamic_cast<sharp::ExtendedHypertree*>(child), problem));
		return transformedTd;
	}
}

namespace decomposer {

const std::string TreeDecomposer::OPTION_SECTION = "Tree decomposition";

TreeDecomposer::TreeDecomposer(Application& app, bool newDefault)
	: Decomposer(app, "td", "Tree decomposition (bucket elimination)", newDefault)
	, optNormalization("n", "normalization", "Use normal form <normalization> for the tree decomposition")
	, optEliminationOrdering("elimination-ordering", "o", "Use elimination ordering <o> for bucket elimination")
{
	optNormalization.addCondition(selected);
	optNormalization.addChoice("none", "No normalization", true);
	optNormalization.addChoice("semi", "Semi-normalization");
	optNormalization.addChoice("normalized", "Normalization");
	app.getOptionHandler().addOption(optNormalization, OPTION_SECTION);

	optEliminationOrdering.addCondition(selected);
	optEliminationOrdering.addChoice("min-degree", "Minimum degree ordering", true);
	optEliminationOrdering.addChoice("min-fill", "Minimum fill ordering");
	optEliminationOrdering.addChoice("mcs", "Maximum cardinality search");
	app.getOptionHandler().addOption(optEliminationOrdering, OPTION_SECTION);
}

Decomposition TreeDecomposer::decompose(const Hypergraph& instance) const
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
	sharp::ExtendedHypertree* td = problem.calculateHypertreeDecomposition();
	assert(td);

	// Transform td into our format
	Hypergraph::Vertices rootBag;
	for(sharp::Vertex v : td->getVertices())
		rootBag.insert(problem.getVertexName(v));

	Decomposition transformedTd = Node(rootBag);
	for(sharp::Hypertree* child : *td->getChildren())
		transformedTd.addChild(transformTd(dynamic_cast<sharp::ExtendedHypertree*>(child), problem));
	return transformedTd;
}

} // namespace decomposer
