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

class Algorithm : public sharp::AbstractHTDAlgorithm
{
public:
	//! @param ignoreOptimization true iff the predicates responsible for optimization problems should be ignored (e.g., cost/1, currentCost/1)
	Algorithm(sharp::Problem& problem, const std::string& instanceFacts, sharp::NormalizationType normalizationType, bool ignoreOptimization = false, bool multiLevel = false);

protected:
	//! @return the file name of the user program to compute node's table
	virtual const char* getUserProgram(const sharp::ExtendedHypertree& node) = 0;

	//! Writes the facts describing node's bags to "out".
	void declareBag(std::ostream& out, const sharp::ExtendedHypertree& node);
	//! Writes the facts describing the child tables to "out".
	void declareChildTables(std::ostream& out, const sharp::ExtendedHypertree& node, const std::vector<sharp::Table*>& childTables);
	//! Writes auxiliary rules to "out".
	void printAuxiliaryRules(std::ostream& out);

	//! Calls declareBag(), declareChildTables(), printAuxiliaryRules(), and runs the ASP solver to compute the node's table.
	virtual sharp::Table* computeTable(const sharp::ExtendedHypertree& node, const std::vector<sharp::Table*>& childTables);

	virtual sharp::Table* evaluateNode(const sharp::ExtendedHypertree* node);
	virtual sharp::ExtendedHypertree* prepareHypertreeDecomposition(sharp::ExtendedHypertree* root);

	virtual std::auto_ptr<Clasp::ClaspFacade::Callback> newClaspCallback(sharp::Table& newTable, const GringoOutputProcessor&, unsigned int numChildNodes, const sharp::VertexSet& currentVertices) const;
	virtual std::auto_ptr<GringoOutputProcessor> newGringoOutputProcessor() const;

	sharp::Problem& problem;
	const std::string& instanceFacts;
	sharp::NormalizationType normalizationType;
	bool ignoreOptimization;
	bool multiLevel;
	Clasp::ClaspFacade clasp;

#ifdef PROGRESS_REPORT
	int nodesProcessed; // For progress report
	virtual void printProgressLine(const sharp::ExtendedHypertree* node, size_t numChildRows = 0);
#endif
};
