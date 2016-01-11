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
#include <sstream>
#include <gringo/input/nongroundparser.hh>
#include <gringo/input/program.hh>
#include <gringo/input/programbuilder.hh>
#include <gringo/output/output.hh>
#include <gringo/logger.hh>
#include <gringo/scripts.hh>
#include <clasp/clasp_facade.h>

#include "Solver.h"
#include "../../Application.h"
#include "../../Printer.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "tables/ClaspCallback.h"
#include "tables/GringoOutputProcessor.h"
#include "trees/ClaspCallback.h"
#include "trees/EncodingChecker.h"
#include "trees/GringoOutputProcessor.h"

namespace solver { namespace clasp {

namespace {

std::unique_ptr<asp_utils::GringoOutputProcessor> newGringoOutputProcessor(Clasp::Asp::LogicProgram& claspProgramBuilder, const ChildItemTrees& childItemTrees, bool tableMode)
{
	if(tableMode)
		return std::unique_ptr<asp_utils::GringoOutputProcessor>(new tables::GringoOutputProcessor(claspProgramBuilder, childItemTrees));
	else
		return std::unique_ptr<asp_utils::GringoOutputProcessor>(new trees::GringoOutputProcessor(claspProgramBuilder, childItemTrees));
}

std::unique_ptr<asp_utils::ClaspCallback> newClaspCallback(bool tableMode, const Gringo::Output::LparseOutputter& gringoOutput, const ChildItemTrees& childItemTrees, const Application& app, bool root, const Decomposition& decomposition, bool cardinalityCost)
{
	if(tableMode)
		return std::unique_ptr<asp_utils::ClaspCallback>(new tables::ClaspCallback(dynamic_cast<const tables::GringoOutputProcessor&>(gringoOutput), childItemTrees, app, root, cardinalityCost));
	else {
		assert(cardinalityCost == false);
		return std::unique_ptr<asp_utils::ClaspCallback>(new trees::ClaspCallback(dynamic_cast<const trees::GringoOutputProcessor&>(gringoOutput), childItemTrees, app, decomposition));
	}
}

} // anonymous namespace

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool tableMode, bool cardinalityCost)
	: ::Solver(decomposition, app)
	, encodingFiles(encodingFiles)
	, tableMode(tableMode)
	, cardinalityCost(cardinalityCost)
{
	Gringo::message_printer()->disable(Gringo::W_ATOM_UNDEFINED);

#ifndef DISABLE_CHECKS
	// TODO: Implement tables::EncodingChecker
	if(!tableMode) {
		// Check the encoding, but only in the decomposition root.
		// Otherwise we'd probably do checks redundantly.
		if(decomposition.isRoot()) {
			std::ofstream dummyStream;
			std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, dummyStream));
			Gringo::Input::Program program;
			asp_utils::DummyGringoModule module;
			Gringo::Scripts scripts(module);
			Gringo::Defines defs;
			std::unique_ptr<EncodingChecker> encodingChecker{new trees::EncodingChecker(scripts, program, *out, defs)};
			Gringo::Input::NonGroundParser parser(*encodingChecker);
			for(const auto& file : encodingFiles)
				parser.pushFile(std::string(file));
			parser.parse();
			encodingChecker->check();
		}
	}
#endif
}

ItemTreePtr Solver::compute()
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	// Compute item trees of child nodes
	ChildItemTrees childItemTrees;
	for(const auto& child : decomposition.getChildren()) {
		ItemTreePtr itree = child->getSolver().compute();
		if(!itree)
			return itree;
		childItemTrees.emplace(child->getNode().getGlobalId(), std::move(itree));
	}

	// Input: Child item trees
	std::unique_ptr<std::stringstream> childItemTreesInput(new std::stringstream);
	*childItemTreesInput << "% Child item tree facts" << std::endl;

	for(const auto& childItemTree : childItemTrees) {
		std::ostringstream rootItemSetName;
		rootItemSetName << 'n' << childItemTree.first;
		asp_utils::declareItemTree(*childItemTreesInput, childItemTree.second.get(), tableMode, childItemTree.first, rootItemSetName.str());
	}
	app.getPrinter().solverInvocationInput(decomposition, childItemTreesInput->str());

	// Input: Induced subinstance
	std::unique_ptr<std::stringstream> instanceInput(new std::stringstream);
	asp_utils::induceSubinstance(*instanceInput, app.getInstance(), decomposition.getNode().getBag());
	app.getPrinter().solverInvocationInput(decomposition, instanceInput->str());

	// Input: Decomposition
	std::unique_ptr<std::stringstream> decompositionInput(new std::stringstream);
	asp_utils::declareDecomposition(decomposition, *decompositionInput);
	app.getPrinter().solverInvocationInput(decomposition, decompositionInput->str());

	// Set up ASP solver
	Clasp::ClaspConfig config;
	config.solve.numModels = 0;
	Clasp::ClaspFacade clasp;
	// TODO The last parameter of clasp.startAsp in the next line is "allowUpdate". Does setting it to false have benefits?
	Clasp::Asp::LogicProgram& claspProgramBuilder = dynamic_cast<Clasp::Asp::LogicProgram&>(clasp.startAsp(config));
	std::unique_ptr<Gringo::Output::LparseOutputter> lpOut(newGringoOutputProcessor(claspProgramBuilder, childItemTrees, tableMode));
	std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, *lpOut));
	Gringo::Input::Program program;
	asp_utils::DummyGringoModule module;
	Gringo::Scripts scripts(module);
	Gringo::Defines defs;
	Gringo::Input::NongroundProgramBuilder gringoProgramBuilder(scripts, program, *out, defs);
	Gringo::Input::NonGroundParser parser(gringoProgramBuilder);

	// Pass input to ASP solver
	for(const auto& file : encodingFiles)
		parser.pushFile(std::string(file));
	parser.pushStream("<instance>", std::move(instanceInput));
	parser.pushStream("<decomposition>", std::move(decompositionInput));
	parser.pushStream("<child_itrees>", std::move(childItemTreesInput));
	parser.parse();

	// Ground and solve
	program.rewrite(defs);
	program.check();
	if(Gringo::message_printer()->hasError())
		throw std::runtime_error("Grounding stopped because of errors");
	auto gPrg = program.toGround(out->domains);
	Gringo::Ground::Parameters params;
	params.add("base", {});
	gPrg.ground(params, scripts, *out);
	params.clear();
	// Finalize ground program and create solver variables
	claspProgramBuilder.endProgram();

	std::unique_ptr<asp_utils::ClaspCallback> cb(newClaspCallback(tableMode, *lpOut, childItemTrees, app, decomposition.isRoot(), decomposition, cardinalityCost));
	cb->prepare(claspProgramBuilder);
	clasp.prepare();
	clasp.solve(cb.get());

	ItemTreePtr result = cb->finalize(decomposition.isRoot(), app.isPruningDisabled() == false || decomposition.isRoot());
	app.getPrinter().solverInvocationResult(decomposition, result.get());
	return result;
}

}} // namespace solver::clasp
