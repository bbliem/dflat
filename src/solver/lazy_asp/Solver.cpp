/*{{{
Copyright 2012-2014, Bernhard Bliem
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
#include <thread>
#include <gringo/input/nongroundparser.hh>
#include <gringo/input/program.hh>
#include <gringo/input/programbuilder.hh>
#include <gringo/output/output.hh>
#include <gringo/logger.hh>
#include <gringo/scripts.hh>
#include <clasp/clasp_facade.h>

#include "Solver.h"
#include "GringoOutputProcessor.h"
#include "../asp/Solver.h"
#include "../../Application.h"
#include "../../Printer.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"

namespace solver { namespace lazy_asp {

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles)
	: ::Solver(decomposition, app)
	, encodingFiles(encodingFiles)
{
	Gringo::message_printer()->disable(Gringo::W_ATOM_UNDEFINED);

	// Start solver thread and wait until it signals us to proceed
	std::thread(&Solver::workerThreadMain, this).detach();
	std::unique_lock<std::mutex> lock(workerMutex);
	wakeMainThread.wait(lock, [&]() { return wakeMainThreadRequested; });
	wakeMainThreadRequested = false;
}

ItemTreePtr Solver::compute()
{
	// Currently this is only called at the root of the decomposition.
	assert(decomposition.getParents().empty());
	nextRow();
	ItemTreePtr result = claspCallback->finalize(false, false);
	app.getPrinter().solverInvocationResult(decomposition, result.get());
	return result;
}

ItemTree::Children::const_iterator Solver::nextRow()
{
	std::unique_lock<std::mutex> lock(workerMutex);
	const auto nodeStackElement = app.getPrinter().visitNode(decomposition);

	if(noMoreModels)
		return claspCallback->getItemTree()->getChildren().end();

	// Let the solver know we want it to work
	wakeWorkerThreadRequested = true;
	wakeWorkerThread.notify_one();

	// Wait until a new row has been produced
	wakeMainThread.wait(lock, [&]() { return wakeMainThreadRequested; });
	wakeMainThreadRequested = false;

	assert(noMoreModels || claspCallback->getNewestRow() != claspCallback->getItemTree()->getChildren().end());
	if(noMoreModels)
		return claspCallback->getItemTree()->getChildren().end();

	return claspCallback->getNewestRow();
}

const ItemTreePtr& Solver::getItemTreeSoFar() const
{
	return claspCallback->getItemTree();
}

void Solver::proceed(std::unique_lock<std::mutex>& lock)
{
	assert(lock.mutex() == &workerMutex && lock.owns_lock());
	wakeMainThreadRequested = true;
	wakeMainThread.notify_one();
	wakeWorkerThread.wait(lock, [&]() { return wakeWorkerThreadRequested; });
	wakeWorkerThreadRequested = false;
}

void Solver::workerThreadMain()
{
	std::unique_lock<std::mutex> lock(workerMutex);

	// Set up ASP solver
	Clasp::ClaspFacade clasp;
	Clasp::ClaspConfig config;
	config.enumerate.numModels = 0;
	Clasp::Asp::LogicProgram& claspProgramBuilder = dynamic_cast<Clasp::Asp::LogicProgram&>(clasp.start(config, Clasp::Problem_t::ASP, true));
	std::unique_ptr<Gringo::Output::LparseOutputter> lpOut(new GringoOutputProcessor(claspProgramBuilder));
	claspCallback.reset(new ClaspCallback(dynamic_cast<GringoOutputProcessor&>(*lpOut), app, *this, lock));
	std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, *lpOut));
	Gringo::Input::Program program;
	Gringo::Scripts scripts;
	Gringo::Defines defs;
	Gringo::Input::NongroundProgramBuilder gringoProgramBuilder(scripts, program, *out, defs);
	Gringo::Input::NonGroundParser parser(gringoProgramBuilder);

	// Input: Original problem instance
	std::unique_ptr<std::stringstream> instanceInput(new std::stringstream);
	*instanceInput << app.getInputString();

	// Input: Decomposition
	std::unique_ptr<std::stringstream> decompositionInput(new std::stringstream);
	solver::asp::Solver::declareDecomposition(decomposition, *decompositionInput);
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

	// Let main thread finish the constructor
	wakeMainThreadRequested = true;
	wakeMainThread.notify_one();

	// Wait until we should do work
	wakeWorkerThread.wait(lock, [&]() { return wakeWorkerThreadRequested; });
	wakeWorkerThreadRequested = false;

	if(decomposition.getChildren().empty()) {
		// This is a leaf solver.
		clasp.solve(claspCallback.get());
	}
	else {
		// Get the first row from each child node
		std::unordered_map<unsigned int, ItemTree::Children::const_iterator> childRows;
		childRows.reserve(decomposition.getChildren().size());
		for(const auto& child : decomposition.getChildren()) {
			const ItemTree::Children::const_iterator newRow = dynamic_cast<Solver&>(child->getSolver()).nextRow();
			if(newRow == dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().end()) {
				// Notify main thread that solving is complete
				noMoreModels = true;
				wakeMainThreadRequested = true;
				wakeMainThread.notify_one();
				return;
			}
			childRows.emplace(child->getNode().getGlobalId(), newRow);
		}

		// Let the combination of these rows be the input for our ASP call
		ItemTreeNode::ExtensionPointerTuple rootExtensionPointers;
		for(const auto& child : decomposition.getChildren())
			rootExtensionPointers.emplace(child->getNode().getGlobalId(), dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getNode());
		claspCallback->setRootExtensionPointers(std::move(rootExtensionPointers));

		ItemTreeNode::ExtensionPointerTuple extendedRows;
		for(const auto& nodeIdAndRow : childRows)
			extendedRows.emplace(nodeIdAndRow.first, (*nodeIdAndRow.second)->getNode());
		claspCallback->setExtendedRows(std::move(extendedRows));

		{
			Clasp::Asp::LogicProgram& prg = static_cast<Clasp::Asp::LogicProgram&>(clasp.update());
			for(const auto& nodeIdAndRow : childRows) {
				for(const auto& item : (*nodeIdAndRow.second)->getNode()->getItems()) {
					prg.freeze(itemsToVars.at(item), Clasp::value_true);
				}
			}
		}
		clasp.prepare();
		claspCallback->prepare(clasp.ctx.symbolTable());
		clasp.solve(claspCallback.get());
		{
			// XXX Necessary to update so often? Is the overhead bad?
			Clasp::Asp::LogicProgram& prg = static_cast<Clasp::Asp::LogicProgram&>(clasp.update());
			for(const auto& nodeIdAndRow : childRows) {
				for(const auto& item : (*nodeIdAndRow.second)->getNode()->getItems()) {
					prg.freeze(itemsToVars.at(item), Clasp::value_false);
				}
			}
		}

		// Now there are no models anymore for this row combination
		// Until there are no new rows anymore, generate one new row at some child node and combine it with all rows from other child nodes
		bool foundNewRow;
		do {
			foundNewRow = false;
			for(const auto& child : decomposition.getChildren()) {
				ItemTree::Children::const_iterator newRow = dynamic_cast<Solver&>(child->getSolver()).nextRow();
				if(newRow != dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().end()) {
					foundNewRow = true;
					aspCallsOnNewRowFromChild(newRow, child, clasp);
				}
			}
		} while(foundNewRow);
	}

	// Notify main thread that solving is complete
	noMoreModels = true;
	wakeMainThreadRequested = true;
	wakeMainThread.notify_one();
}

void Solver::aspCallsOnNewRowFromChild(ItemTree::Children::const_iterator newRow, const DecompositionPtr& originatingChild, Clasp::ClaspFacade& clasp)
{
	std::vector<std::pair<Decomposition*, ItemTree::Children::const_iterator>> rowIterators; // Key: Child node; Value: Row in the item tree at this child
	rowIterators.reserve(decomposition.getChildren().size());
	// At index 0 we will store newRow. This iterator will not be incremented.
	rowIterators.emplace_back(originatingChild.get(), newRow);
	for(const auto& child : decomposition.getChildren()) {
		if(child != originatingChild) {
			rowIterators.emplace_back(child.get(), dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().begin());
			assert(rowIterators.back().second != dynamic_cast<Solver&>(child->getSolver()).getItemTreeSoFar()->getChildren().end());
		}
	}

	do {
		ItemTreeNode::ExtensionPointerTuple extendedRows;
		for(const auto& nodeAndRow : rowIterators)
			extendedRows.emplace(nodeAndRow.first->getNode().getGlobalId(), (*nodeAndRow.second)->getNode());
		claspCallback->setExtendedRows(std::move(extendedRows));

		{
			Clasp::Asp::LogicProgram& prg = static_cast<Clasp::Asp::LogicProgram&>(clasp.update());
			for(const auto& nodeAndRow : rowIterators) {
				for(const auto& item : (*nodeAndRow.second)->getNode()->getItems()) {
					//				std::cout << "W " << std::this_thread::get_id() << " [" << decomposition.getNode().getGlobalId() << "]: Setting " << item << " to true\n";
					prg.freeze(itemsToVars.at(item), Clasp::value_true);
				}
			}
		}
		clasp.prepare();
		claspCallback->prepare(clasp.ctx.symbolTable());
		clasp.solve(claspCallback.get());
		{
			// XXX Necessary to update so often? Is the overhead bad?
			Clasp::Asp::LogicProgram& prg = static_cast<Clasp::Asp::LogicProgram&>(clasp.update());
			for(const auto& nodeAndRow : rowIterators) {
				for(const auto& item : (*nodeAndRow.second)->getNode()->getItems()) {
					//				std::cout << "W " << std::this_thread::get_id() << " [" << decomposition.getNode().getGlobalId() << "]: Setting " << item << " to false\n";
					prg.freeze(itemsToVars.at(item), Clasp::value_false);
				}
			}
		}
	} while(nextRowCombination(rowIterators));
}

bool Solver::nextRowCombination(std::vector<std::pair<Decomposition*, ItemTree::Children::const_iterator>>& rowIterators, size_t incrementPos)
{
	// Increment the iterator at index incrementPos, then reset all iterators before it except at index 0 (this is the new row which should be combined with all "old" rows from other child nodes)
	if(incrementPos == rowIterators.size())
		return false;

	if(++rowIterators[incrementPos].second == dynamic_cast<Solver&>(rowIterators[incrementPos].first->getSolver()).getItemTreeSoFar()->getChildren().end())
		return nextRowCombination(rowIterators, incrementPos+1);
	else {
		for(size_t i = 1; i < incrementPos; ++i)
			rowIterators[i].second = dynamic_cast<Solver&>(rowIterators[i].first->getSolver()).getItemTreeSoFar()->getChildren().begin();
	}
	return true;
}

}} // namespace solver::lazy_asp
