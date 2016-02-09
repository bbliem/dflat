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
#include "GringoOutputProcessor.h"
#include "../clasp/Solver.h"
#include "../../Application.h"
#include "../../Printer.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"

namespace solver { namespace lazy_clasp {

//unsigned Solver::solverSetups = 0;
//unsigned Solver::solveCalls = 0;
//unsigned Solver::models = 0;
//unsigned Solver::discardedModels = 0;

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool reground, BranchAndBoundLevel bbLevel)
	: ::LazySolver(decomposition, app, bbLevel)
	, reground(reground)
	, encodingFiles(encodingFiles)
{
	Gringo::message_printer()->disable(Gringo::W_ATOM_UNDEFINED);

	if(!reground) {
		// Set up ASP solver
		config.solve.numModels = 0;
		Clasp::Asp::LogicProgram& claspProgramBuilder = static_cast<Clasp::Asp::LogicProgram&>(clasp.startAsp(config, true)); // TODO In leaves updates might not be necessary.

		struct LazyGringoOutputProcessor : GringoOutputProcessor
		{
			LazyGringoOutputProcessor(Solver* s, Clasp::Asp::LogicProgram& prg)
				: GringoOutputProcessor(prg), self(s)
			{
			}

			void storeAtom(unsigned int atomUid, Gringo::Value v) override
			{
				const std::string& n = *v.name();
				if(n == "childItem") {
					ASP_CHECK(v.args().size() == 1, "'childItem' predicate does not have arity 1");
					std::ostringstream argument;
					v.args().front().print(argument);
					self->itemsToLitIndices.emplace(String(argument.str()), self->literals.size());
					self->literals.push_back(Clasp::posLit(atomUid));
				}
				else if(n == "childAuxItem") {
					ASP_CHECK(v.args().size() == 1, "'childAuxItem' predicate does not have arity 1");
					std::ostringstream argument;
					v.args().front().print(argument);
					self->auxItemsToLitIndices.emplace(String(argument.str()), self->literals.size());
					self->literals.push_back(Clasp::posLit(atomUid));
				}
				GringoOutputProcessor::storeAtom(atomUid, v);
			}

			Solver* self;
		} gringoOutput(this, claspProgramBuilder);

		std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, gringoOutput));
		Gringo::Input::Program program;
		asp_utils::DummyGringoModule module;
		Gringo::Scripts scripts(module);
		Gringo::Defines defs;
		Gringo::Input::NongroundProgramBuilder gringoProgramBuilder(scripts, program, *out, defs);
		Gringo::Input::NonGroundParser parser(gringoProgramBuilder);

		// Input: Induced subinstance
		std::unique_ptr<std::stringstream> instanceInput(new std::stringstream);
		asp_utils::induceSubinstance(*instanceInput, app.getInstance(), decomposition.getNode().getBag());
		app.getPrinter().solverInvocationInput(decomposition, instanceInput->str());

		// Input: Decomposition
		std::unique_ptr<std::stringstream> decompositionInput(new std::stringstream);
		asp_utils::declareDecomposition(decomposition, *decompositionInput);
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

		// Set value of external atoms to free
		for(const auto& p : literals)
			claspProgramBuilder.freeze(p.var(), Clasp::value_free);

		// Finalize ground program and create solver literals
		claspProgramBuilder.endProgram();

		// Map externals to their solver literals
		for(auto& p : literals) {
			p = claspProgramBuilder.getLiteral(p.var());
			assert(!p.watched()); // Literal must not be watched
		}

		for(const auto& atom : gringoOutput.getItemAtomInfos())
			itemAtomInfos.emplace_back(ItemAtomInfo(atom, claspProgramBuilder));
		for(const auto& atom : gringoOutput.getAuxItemAtomInfos())
			auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, claspProgramBuilder));
//		for(const auto& atom : gringoOutput->getCurrentCostAtomInfos())
//			currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, claspProgramBuilder));
//		for(const auto& atom : gringoOutput->getCostAtomInfos())
//			costAtomInfos.emplace_back(CostAtomInfo(atom, claspProgramBuilder)));

		// Prepare for solving.
		clasp.prepare();
	}
}

void Solver::startSolvingForCurrentRowCombination()
{
//	++solverSetups;
	asyncResult.reset();

	if(reground) {
		// Set up ASP solver
		config.solve.numModels = 0;
		// TODO The last parameter of clasp.startAsp in the next line is "allowUpdate". Does setting it to false have benefits?
		// WORKAROUND for BUG in ClaspFacade::startAsp()
		// TODO remove on update to new version
		if(clasp.ctx.numVars() == 0 && clasp.ctx.frozen())
			clasp.ctx.reset();

		Clasp::Asp::LogicProgram& claspProgramBuilder = static_cast<Clasp::Asp::LogicProgram&>(clasp.startAsp(config));
		GringoOutputProcessor gringoOutput(claspProgramBuilder);
		std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, gringoOutput));
		Gringo::Input::Program program;
		asp_utils::DummyGringoModule module;
		Gringo::Scripts scripts(module);
		Gringo::Defines defs;
		Gringo::Input::NongroundProgramBuilder gringoProgramBuilder(scripts, program, *out, defs);
		Gringo::Input::NonGroundParser parser(gringoProgramBuilder);

		// Input: Induced subinstance
		std::unique_ptr<std::stringstream> instanceInput(new std::stringstream);
		asp_utils::induceSubinstance(*instanceInput, app.getInstance(), decomposition.getNode().getBag());
		app.getPrinter().solverInvocationInput(decomposition, instanceInput->str());

		// Input: Decomposition
		std::unique_ptr<std::stringstream> decompositionInput(new std::stringstream);
		asp_utils::declareDecomposition(decomposition, *decompositionInput);
		app.getPrinter().solverInvocationInput(decomposition, decompositionInput->str());

		// Input: Child rows
		std::unique_ptr<std::stringstream> childRowsInput(new std::stringstream);
		*childRowsInput << "% Child row facts" << std::endl;
		for(const auto& row : getCurrentRowCombination()) {
			for(const auto& item : row->getItems())
				*childRowsInput << "childItem(" << item << ")." << std::endl;
			for(const auto& item : row->getAuxItems())
				*childRowsInput << "childAuxItem(" << item << ")." << std::endl;
			// TODO costs, etc.
		}
		app.getPrinter().solverInvocationInput(decomposition, childRowsInput->str());

		// Pass input to ASP solver
		for(const auto& file : encodingFiles)
			parser.pushFile(std::string(file));
		parser.pushStream("<instance>", std::move(instanceInput));
		parser.pushStream("<decomposition>", std::move(decompositionInput));
		parser.pushStream("<child_rows>", std::move(childRowsInput));
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

		claspProgramBuilder.endProgram();

		itemAtomInfos.clear();
		for(const auto& atom : gringoOutput.getItemAtomInfos())
			itemAtomInfos.emplace_back(ItemAtomInfo(atom, claspProgramBuilder));
		auxItemAtomInfos.clear();
		for(const auto& atom : gringoOutput.getAuxItemAtomInfos())
			auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, claspProgramBuilder));
		// TODO costs etc.

		clasp.prepare();
	}

	else {
		// Set external variables to the values of the current child row combination
		clasp.update(false, false);

		clasp.prepare();

		// Mark atoms corresponding to items from the currently extended rows
		for(const auto& row : getCurrentRowCombination()) {
			for(const auto& item : row->getItems()) {
				assert(itemsToLitIndices.find(item) != itemsToLitIndices.end());
				assert(itemsToLitIndices.at(item) < literals.size());
#ifdef DISABLE_CHECKS
				literals[itemsToLitIndices.at(item)].watch();
#else
				try {
					literals[itemsToLitIndices.at(item)].watch();
				}
				catch(const std::out_of_range&) {
					std::ostringstream msg;
					msg << "Unknown variable; atom childItem(" << *item << ") not shown or not declared as external?";
					throw std::runtime_error(msg.str());
				}
#endif
			}
			for(const auto& item : row->getAuxItems()) {
				assert(auxItemsToLitIndices.find(item) != auxItemsToLitIndices.end());
				assert(auxItemsToLitIndices.at(item) < literals.size());
#ifdef DISABLE_CHECKS
				literals[auxItemsToLitIndices.at(item)].watch();
#else
				try {
					literals[auxItemsToLitIndices.at(item)].watch();
				}
				catch(const std::out_of_range&) {
					std::ostringstream msg;
					msg << "Unknown variable; atom childAuxItem(" << *item << ") not shown or not declared as external?";
					throw std::runtime_error(msg.str());
				}
#endif
			}
		}
		// Set marked literals to true and all others to false
		for(auto& lit : literals) {
			if(lit.watched()) {
				lit.clearWatch();
				clasp.assume(lit);
			}
			else
				clasp.assume(~lit);
		}
	}

	asyncResult.reset(new BasicSolveIter(clasp));
}

bool Solver::endOfRowCandidates() const
{
//	++solveCalls;
	return !asyncResult || asyncResult->end();
}

void Solver::nextRowCandidate()
{
	assert(asyncResult);
	asyncResult->next();
}

void Solver::handleRowCandidate(long costBound)
{
//	++models;
	assert(asyncResult);
	const Clasp::Model& m = asyncResult->model();

	// Get items {{{
	ItemTreeNode::Items items;
	asp_utils::forEachTrue(m, itemAtomInfos, [&items](const GringoOutputProcessor::ItemAtomArguments& arguments) {
			items.insert(arguments.item);
	});
	ItemTreeNode::Items auxItems;
	asp_utils::forEachTrue(m, auxItemAtomInfos, [&auxItems](const GringoOutputProcessor::AuxItemAtomArguments& arguments) {
			auxItems.insert(arguments.item);
	});

	ASP_CHECK(std::find_if(items.begin(), items.end(), [&auxItems](const String& item) {
				return auxItems.find(item) != auxItems.end();
	}) == items.end(), "Items and auxiliary items not disjoint");
	// }}}
	// FIXME Do proper cost computations, not this item-set cardinality proof of concept
	// Compute cost {{{
//	ASP_CHECK(asp_utils::countTrue(m, costAtomInfos) <= 1, "More than one true cost/1 atom");
//	long cost = 0;
//	asp_utils::forFirstTrue(m, costAtomInfos, [&cost](const GringoOutputProcessor::CostAtomArguments& arguments) {
//			cost = arguments.cost;
//	});
//	node->setCost(cost);

	long cost = items.size();
	for(const auto& row : getCurrentRowCombination()) {
		const auto& oldItems = row->getItems();
		ItemTreeNode::Items intersection;
		std::set_intersection(items.begin(), items.end(), oldItems.begin(), oldItems.end(), std::inserter(intersection, intersection.begin()));
		cost += row->getCost() - intersection.size();
	}
	// }}}
	if(cost >= costBound) {
//		++discardedModels;
		newestRow = itemTree->getChildren().end();
		return;
	}

	assert(itemTree);
	// Create item tree node {{{
	std::shared_ptr<ItemTreeNode> node(new ItemTreeNode(std::move(items), std::move(auxItems), {getCurrentRowCombination()}));
	// }}}
	if(!app.isOptimizationDisabled()) {
		// Set cost {{{
		node->setCost(cost);
		// }}}
		// Set current cost {{{
//		ASP_CHECK(asp_utils::countTrue(m, currentCostAtomInfos) <= 1, "More than one true currentCost/1 atom");
//		ASP_CHECK(asp_utils::countTrue(m, currentCostAtomInfos) == 0 || asp_utils::countTrue(m, costAtomInfos) == 1, "True currentCost/1 atom without true cost/1 atom");
//		long currentCost = 0;
//		asp_utils::forFirstTrue(m, currentCostAtomInfos, [&currentCost](const GringoOutputProcessor::CurrentCostAtomArguments& arguments) {
//				currentCost = arguments.currentCost;
//		});
//		node->setCurrentCost(currentCost);
		node->setCurrentCost(node->getItems().size());
		// }}}
		// Possibly update cost of root {{{
		itemTree->getNode()->setCost(std::min(itemTree->getNode()->getCost(), cost));
		// }}}
	}
	// Add node to item tree {{{
	//ItemTree::Children::const_iterator newChild = itemTree->addChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::move(node))));
	newestRow = itemTree->costChangeAfterAddChildAndMerge(ItemTree::ChildPtr(new ItemTree(std::move(node))));
	// }}}

	//if(newChild != itemTree->getChildren().end())
	//	newestRow = newChild;
}

}} // namespace solver::lazy_clasp
