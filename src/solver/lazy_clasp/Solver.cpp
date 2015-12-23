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
#include <sstream>
#include <gringo/input/nongroundparser.hh>
#include <gringo/input/program.hh>
#include <gringo/input/programbuilder.hh>
#include <gringo/output/output.hh>
#include <gringo/logger.hh>
#include <gringo/scripts.hh>
#include <clasp/clasp_facade.h>

#include "Solver.h"
#include "GringoOutputProcessor.h"
#include "../clasp/Solver.h"
#include "../../Application.h"
#include "../../Printer.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"

namespace solver { namespace lazy_clasp {

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool branchAndBound)
	: ::LazySolver(decomposition, app, branchAndBound)
	, encodingFiles(encodingFiles)
{
	Gringo::message_printer()->disable(Gringo::W_ATOM_UNDEFINED);

	// Set up ASP solver
	config.solve.numModels = 0;
	Clasp::Asp::LogicProgram& claspProgramBuilder = static_cast<Clasp::Asp::LogicProgram&>(clasp.startAsp(config, true)); // TODO In leaves updates might not be necessary.
	lpOut.reset(new GringoOutputProcessor(claspProgramBuilder));
	claspCallback.reset(new ClaspCallback(dynamic_cast<GringoOutputProcessor&>(*lpOut), app));
	std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, *lpOut));
	Gringo::Input::Program program;
	asp_utils::DummyGringoModule module;
	Gringo::Scripts scripts(module);
	Gringo::Defines defs;
	Gringo::Input::NongroundProgramBuilder gringoProgramBuilder(scripts, program, *out, defs);
	Gringo::Input::NonGroundParser parser(gringoProgramBuilder);

	// Input: Original problem instance
	std::unique_ptr<std::stringstream> instanceInput(new std::stringstream);
	*instanceInput << app.getInputString();

	// Input: Decomposition
	std::unique_ptr<std::stringstream> decompositionInput(new std::stringstream);
	solver::clasp::Solver::declareDecomposition(decomposition, *decompositionInput);
	app.getPrinter().solverInvocationInput(decomposition, decompositionInput->str());

	// Pass input to ASP solver
	for(const auto& file : encodingFiles)
		parser.pushFile(std::string(file));
	parser.pushStream("<instance>", std::move(instanceInput));
	parser.pushStream("<decomposition>", std::move(decompositionInput));
	parser.parse();

	// Ground
	program.rewrite(defs);
	program.check();
	if(Gringo::message_printer()->hasError())
		throw std::runtime_error("Grounding stopped because of errors");
	auto gPrg = program.toGround(out->domains);
	Gringo::Ground::Parameters params;
	params.add("base", {});
	gPrg.ground(params, scripts, *out);
	params.clear();

	// Prepare for solving. (This makes clasp's symbol table available.)
	clasp.prepare();

	// We need to know which clasp variable corresponds to each childItem(_) atom.
	for(const auto& pair : clasp.ctx.symbolTable()) {
		if(!pair.second.name.empty()) {
			const std::string name = pair.second.name.c_str();
			if(name.compare(0, 10, "childItem(") == 0) {
				itemsToVarIndices.emplace(String(name.substr(10, name.length()-11)), variables.size());
				variables.push_back(pair.first);
			}
		}
	}

	clasp.update();
	for(const auto& var : variables)
		claspProgramBuilder.freeze(var, Clasp::value_free);
	clasp.prepare();

	claspCallback->prepare(clasp.ctx.symbolTable());
}

const ItemTreePtr& Solver::getItemTree() const
{
	return claspCallback->getItemTree();
}

void Solver::setItemTree(ItemTreePtr&& itemTree)
{
	claspCallback->setItemTree(std::move(itemTree));
}

ItemTree::Children::const_iterator Solver::getNewestRow() const
{
	return claspCallback->getNewestRow();
}

ItemTreePtr Solver::finalize()
{
	return claspCallback->finalize(false, false);
}

void Solver::startSolvingForCurrentRowCombination()
{
	asyncResult.reset();

	// Set external variables to the values of the current child row combination
	Clasp::Asp::LogicProgram& prg = static_cast<Clasp::Asp::LogicProgram&>(clasp.update(false, false));

	clasp.prepare();

	// Mark atoms corresponding to items from the currently extended rows
	const unsigned int IN_SET = 2147483648; // 2^31 (atom IDs are always smaller)
	for(const auto& row : getCurrentRowCombination()) {
		for(const auto& item : row->getItems())
			variables[itemsToVarIndices.at(item)] |= IN_SET;
	}
	// Set marked atoms to true and all others to false
	for(auto& var : variables) {
		if(var & IN_SET) {
			var ^= IN_SET;
			clasp.assume(prg.getLiteral(var));
		}
		else
			clasp.assume(~prg.getLiteral(var));
	}

	asyncResult.reset(new BasicSolveIter(clasp));
}

bool Solver::endOfRowCandidates() const
{
	assert(asyncResult);
	return asyncResult->end();
}

void Solver::nextRowCandidate()
{
	assert(asyncResult);
	asyncResult->next();
}

void Solver::handleRowCandidate(long costBound)
{
	assert(asyncResult);
	// XXX claspCallback does not need to be a clasp callback in fact
	claspCallback->setExtendedRows(getCurrentRowCombination());
	claspCallback->setCostBound(costBound);
	claspCallback->onModel(*clasp.ctx.master(), asyncResult->model());
}

}} // namespace solver::lazy_clasp
