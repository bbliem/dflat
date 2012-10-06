/*
Copyright 2012, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dynasp/dflat/>.

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

#pragma once

#include <sharp/main>
#include <clasp/clasp_facade.h>

class GringoOutputProcessor;

class Algorithm : public sharp::AbstractSemiNormalizedHTDAlgorithm
{
public:
	//! @param normalizationType either sharp::SemiNormalization or sharp::DefaultNormalization
	Algorithm(sharp::Problem& problem, const sharp::PlanFactory& planFactory, const std::string& instanceFacts, const char* exchangeNodeProgram, const char* joinNodeProgram = 0, sharp::NormalizationType normalizationType = sharp::SemiNormalization, unsigned int level = 0);

protected:
	virtual sharp::Plan* selectPlan(sharp::TupleTable* table, const sharp::ExtendedHypertree* root);
	virtual sharp::ExtendedHypertree* prepareHypertreeDecomposition(sharp::ExtendedHypertree* root);

	// If you want to derive from this class, do not override evaluate*Node, but rather exchange*/join.
	sharp::TupleTable* evaluatePermutationNode(const sharp::ExtendedHypertree* node);
	sharp::TupleTable* evaluateBranchNode(const sharp::ExtendedHypertree* node);

	virtual sharp::TupleTable* exchange(const sharp::VertexSet& vertices, const sharp::VertexSet& introduced, const sharp::VertexSet& removed, const sharp::TupleTable& childTable, bool isRoot = false);
	virtual sharp::TupleTable* join(const sharp::VertexSet& vertices, sharp::TupleTable& childTableLeft, sharp::TupleTable& childTableRight);

	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::TupleTable& newTable, const GringoOutputProcessor&, const sharp::VertexSet& currentVertices) const;
	virtual std::auto_ptr<GringoOutputProcessor> newGringoOutputProcessor() const;

	sharp::Problem& problem;
	sharp::NormalizationType normalizationType;
	const std::string& instanceFacts;
	const char* exchangeNodeProgram;
	const char* joinNodeProgram;
	unsigned int level;
	Clasp::ClaspFacade clasp;

#ifdef PROGRESS_REPORT
	int nodesProcessed; // For progress report
	virtual void printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildTuples = 0);
	virtual sharp::TupleTable* evaluateNode(const sharp::ExtendedHypertree* node);
#endif
#ifdef VERBOSE
	virtual void printBagContents(const sharp::VertexSet& vertices) const;
#endif
};
