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

namespace {

class DummyGringoModule : public Gringo::GringoModule
{
    virtual Gringo::Control *newControl(int argc, char const **argv) override { throw std::logic_error("DummyGringoModule"); };
    virtual void freeControl(Gringo::Control *ctrl) override { throw std::logic_error("DummyGringoModule"); };
    virtual Gringo::Value parseValue(std::string const &repr) override { throw std::logic_error("DummyGringoModule"); };
};

}

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles)
	: ::Solver(decomposition, app)
	, encodingFiles(encodingFiles)
{
	for(const auto& child : decomposition.getChildren())
		nonExhaustedChildSolvers.push_back(static_cast<Solver*>(&child->getSolver()));
	nextChildSolverToCall = nonExhaustedChildSolvers.begin();

	Gringo::message_printer()->disable(Gringo::W_ATOM_UNDEFINED);

	// Set up ASP solver
	config.solve.numModels = 0;
	Clasp::Asp::LogicProgram& claspProgramBuilder = static_cast<Clasp::Asp::LogicProgram&>(clasp.startAsp(config, true)); // TODO In leaves updates might not be necessary.
	//std::unique_ptr<Gringo::Output::LparseOutputter> lpOut(new GringoOutputProcessor(claspProgramBuilder));
	lpOut.reset(new GringoOutputProcessor(claspProgramBuilder));
	claspCallback.reset(new ClaspCallback(dynamic_cast<GringoOutputProcessor&>(*lpOut), app));
	std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, *lpOut));
	Gringo::Input::Program program;
	DummyGringoModule module;
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
	// TODO: Prepare reasonable without starting solving yet?
	clasp.prepare();
	claspCallback->prepare(clasp.ctx.symbolTable());

	// We need to know which clasp variable corresponds to each childItem(_) atom.
	for(const auto& pair : clasp.ctx.symbolTable()) {
		if(!pair.second.name.empty()) {
			const std::string name = pair.second.name.c_str();
			if(name.compare(0, 10, "childItem(") == 0) {
				const std::string argument = name.substr(10, name.length()-11);
				itemsToVars.emplace(argument, pair.first);
			}
		}
	}

	//asyncResult = clasp.solveAsync(claspCallback.get());
}

ItemTreePtr Solver::compute()
{
	// Currently this is only called at the root of the decomposition.
	assert(decomposition.isRoot());
	nextRow();
	//while(nextRow() != getItemTreeSoFar()->getChildren().end())
	//	;
	ItemTreePtr result = claspCallback->finalize(false, false);
	app.getPrinter().solverInvocationResult(decomposition, result.get());
	return result;
}

ItemTree::Children::const_iterator Solver::nextRow()
{
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	//std::cout << decomposition.getNode().getGlobalId() << " nextRow\n";

	if(!claspCallback->getItemTree()) {
		loadFirstChildRowCombination();
		//std::cout << decomposition.getNode().getGlobalId() << " first child row combination loaded\n";
		startSolvingForCurrentRowCombination();
	}

	assert(claspCallback->getItemTree());
	assert(getItemTreeSoFar());

	assert(asyncResult);

	do {
		while(asyncResult->end()) {
			//std::cout << decomposition.getNode().getGlobalId() << " asyncResult->end() == true\n";
			if(loadNextChildRowCombination() == false)
				return getItemTreeSoFar()->getChildren().end();
			startSolvingForCurrentRowCombination();
		}

		// XXX solver correct / necessary?
		// XXX claspCallback does not need to be a clasp callback in fact
		claspCallback->onModel(*clasp.ctx.master(), asyncResult->model());

		//std::cout << decomposition.getNode().getGlobalId() << " asyncResult->end() == false\n";

		// Model has now already been processed by claspCallback
		asyncResult->next();

		if(claspCallback->getNewestRow() == claspCallback->getItemTree()->getChildren().end()) {
			//std::cout << decomposition.getNode().getGlobalId() << " skipping model not yielding new row\n";
		}
	} while(claspCallback->getNewestRow() == claspCallback->getItemTree()->getChildren().end());

	return claspCallback->getNewestRow();
}

bool Solver::loadFirstChildRowCombination()
{
	assert(!claspCallback->getItemTree());
	assert(!asyncResult);

	//std::cout << decomposition.getNode().getGlobalId() << " loadFirstChildRowCombination\n";

	// Get the first row from each child node
	assert(rowIterators.empty());
	rowIterators.reserve(decomposition.getChildren().size());
	for(const auto& child : decomposition.getChildren()) {
		assert(!dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar());
		const ItemTree::Children::const_iterator firstRow = dynamic_cast<Solver&>(child->getSolver()).nextRow();
		assert(firstRow == dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().begin());
		if(firstRow == dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().end())
			return false;
		rowIterators.emplace_back(child.get(), firstRow);
	}

	// Initialize claspCallback by telling it the roots of the child item trees
	ItemTreeNode::ExtensionPointerTuple rootExtensionPointers;
	rootExtensionPointers.reserve(decomposition.getChildren().size());
	for(const auto& child : decomposition.getChildren())
		rootExtensionPointers.push_back(dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getNode());
	claspCallback->initializeItemTree(std::move(rootExtensionPointers));
	return true;
}

bool Solver::loadNextChildRowCombination()
{
	assert(asyncResult);
	assert(!asyncResult->running());

	//std::cout << decomposition.getNode().getGlobalId() << " loadNextChildRowCombination\n";

	if(decomposition.getChildren().empty()) {
		//std::cout << decomposition.getNode().getGlobalId() << " loadNextChildRowCombination return false (no children)\n";
		return false;
	}

	if(nextExistingRowCombination()) {
		//std::cout << decomposition.getNode().getGlobalId() << " loadNextChildRowCombination return true (next combination loaded)\n";
		return true;
	}

	// There is no combination of existing child rows, so we compute a new one
	assert(nextChildSolverToCall != nonExhaustedChildSolvers.end());
	Solver* childSolver = *nextChildSolverToCall;
	ItemTree::Children::const_iterator newRow = childSolver->nextRow();

	while(newRow == childSolver->getItemTreeSoFar()->getChildren().end()) {
		// The child solver is now exhausted
		//std::cout << decomposition.getNode().getGlobalId() << " loadNextChildRowCombination child solver " << childSolver->decomposition.getNode().getGlobalId() << " exhausted\n";
		// Remove it from nonExhaustedChildSolvers
		// Set nextChildSolverToCall to the next one
		nonExhaustedChildSolvers.erase(nextChildSolverToCall++);

		if(nonExhaustedChildSolvers.empty()) {
			//std::cout << decomposition.getNode().getGlobalId() << " loadNextChildRowCombination return false (no nonexhausted left)\n";
			return false;
		}

		if(nextChildSolverToCall == nonExhaustedChildSolvers.end())
			nextChildSolverToCall = nonExhaustedChildSolvers.begin();

		assert(nextChildSolverToCall != nonExhaustedChildSolvers.end());
		childSolver = *nextChildSolverToCall;
		newRow = childSolver->nextRow();
	}

	// Now we have computed a new child row
	originOfLastChildRow = childSolver->decomposition.getNode().getGlobalId();
	resetRowIteratorsOnNewRow(newRow);
	++nextChildSolverToCall;
	if(nextChildSolverToCall == nonExhaustedChildSolvers.end())
		nextChildSolverToCall = nonExhaustedChildSolvers.begin();
	assert(nextChildSolverToCall != nonExhaustedChildSolvers.end());

	//std::cout << decomposition.getNode().getGlobalId() << " loadNextChildRowCombination return true (new row fetched)\n";
	return true;
}

const ItemTreePtr& Solver::getItemTreeSoFar() const
{
	return claspCallback->getItemTree();
}

void Solver::startSolvingForCurrentRowCombination()
{
	//std::cout << decomposition.getNode().getGlobalId() << " startSolvingForCurrentRowCombination\n";

	// Set extension pointers for all upcoming rows
	ItemTreeNode::ExtensionPointerTuple extendedRows;
	for(const auto& nodeAndRow : rowIterators)
		extendedRows.push_back((*nodeAndRow.second)->getNode());
	claspCallback->setExtendedRows(std::move(extendedRows));

	// Set external variables to the values of the current child row combination
	// TODO assumptions instead of freezing
	Clasp::Asp::LogicProgram& prg = static_cast<Clasp::Asp::LogicProgram&>(clasp.update());
	for(const auto& pair : itemsToVars)
		prg.freeze(pair.second, Clasp::value_false);
	for(const auto& nodeAndRow : rowIterators) {
		for(const auto& item : (*nodeAndRow.second)->getNode()->getItems())
			prg.freeze(itemsToVars.at(*item), Clasp::value_true);
	}

	clasp.prepare();
	claspCallback->prepare(clasp.ctx.symbolTable());

//	for(const auto& nodeAndRow : rowIterators) {
//		for(const auto& item : (*nodeAndRow.second)->getNode()->getItems()) {
//			std::cout << "assume " << *item << '\n';
//			clasp.assume(itemsToLiterals.at(*item));
//		}
//	}

	asyncResult.reset(new Clasp::ClaspFacade::AsyncResult(clasp.startSolveAsync()));
}

void Solver::resetRowIteratorsOnNewRow(ItemTree::Children::const_iterator newRow)
{
	//std::cout << decomposition.getNode().getGlobalId() << " resetRowIteratorsOnNewRow\n";

	rowIterators.clear();
	rowIterators.reserve(decomposition.getChildren().size());
//	// At index 0 we will store newRow. This iterator will not be incremented.
//	rowIterators.emplace_back(originatingChild.get(), newRow);
	for(const auto& child : decomposition.getChildren()) {
//		if(child != originatingChild) {
//			rowIterators.emplace_back(child.get(), static_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().begin());
//			assert(rowIterators.back().second != static_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().end());
//		}
		if(child->getNode().getGlobalId() == originOfLastChildRow)
			rowIterators.emplace_back(child.get(), newRow);
		else {
			rowIterators.emplace_back(child.get(), static_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().begin());
			assert(rowIterators.back().second != static_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().end());
		}
	}
}

bool Solver::nextExistingRowCombination(size_t incrementPos)
{
	//std::cout << decomposition.getNode().getGlobalId() << " nextExistingRowCombination " << incrementPos << "\n";
	// Increment the iterator at index incrementPos, then reset all iterators before it except at index 0 (this is the new row which should be combined with all "old" rows from other child nodes)
	if(incrementPos >= rowIterators.size()) {
		//std::cout << decomposition.getNode().getGlobalId() << " nextExistingRowCombination return false\n";
		return false;
	}

	// Don't increment originOfLastChildRow since this is the new row we want to combine with all existing ones
	if(rowIterators[incrementPos].first->getNode().getGlobalId() == originOfLastChildRow) {
		//std::cout << decomposition.getNode().getGlobalId() << " nextExistingRowCombination skip originOfLastChildRow\n";
		return nextExistingRowCombination(incrementPos+1);
	}

	if(++rowIterators[incrementPos].second == static_cast<Solver&>(rowIterators[incrementPos].first->getSolver()).getItemTreeSoFar()->getChildren().end())
		return nextExistingRowCombination(incrementPos+1);
	else {
		for(size_t i = 0; i < incrementPos; ++i) {
			if(rowIterators[i].first->getNode().getGlobalId() != originOfLastChildRow)
				rowIterators[i].second = static_cast<Solver&>(rowIterators[i].first->getSolver()).getItemTreeSoFar()->getChildren().begin();
		}
	}
	//std::cout << decomposition.getNode().getGlobalId() << " nextExistingRowCombination return true\n";
	return true;
}

}} // namespace solver::lazy_clasp
