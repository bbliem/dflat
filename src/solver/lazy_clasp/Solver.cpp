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

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool reground, bool branchAndBound)
	: ::LazySolver(decomposition, app, branchAndBound)
	, reground(reground)
	, encodingFiles(encodingFiles)
{
	Gringo::message_printer()->disable(Gringo::W_ATOM_UNDEFINED);

	if(!reground) {
		// Set up ASP solver
		config.solve.numModels = 0;
		Clasp::Asp::LogicProgram& claspProgramBuilder = static_cast<Clasp::Asp::LogicProgram&>(clasp.startAsp(config, true)); // TODO In leaves updates might not be necessary.
		gringoOutput.reset(new GringoOutputProcessor(claspProgramBuilder));
		std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, *gringoOutput));
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

		for(const auto& atom : gringoOutput->getItemAtomInfos())
			itemAtomInfos.emplace_back(ItemAtomInfo(atom, clasp.ctx.symbolTable()));
		for(const auto& atom : gringoOutput->getAuxItemAtomInfos())
			auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, clasp.ctx.symbolTable()));
//		for(const auto& atom : gringoOutput->getCurrentCostAtomInfos())
//			currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, clasp.ctx.symbolTable()));
//		for(const auto& atom : gringoOutput->getCostAtomInfos())
//			costAtomInfos.emplace_back(CostAtomInfo(atom, clasp.ctx.symbolTable()));
	}
}

ItemTreePtr Solver::finalize()
{
	if(itemTree && itemTree->finalize(app, false, false) == false)
		itemTree.reset();
	return std::move(itemTree);
}

void Solver::startSolvingForCurrentRowCombination()
{
	asyncResult.reset();

	if(reground) {
		// Set up ASP solver
		config.solve.numModels = 0;
		// TODO The last parameter of clasp.startAsp in the next line is "allowUpdate". Does setting it to false have benefits?
		Clasp::Asp::LogicProgram& claspProgramBuilder = static_cast<Clasp::Asp::LogicProgram&>(clasp.startAsp(config));
		gringoOutput.reset(new GringoOutputProcessor(claspProgramBuilder));
		std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, *gringoOutput));
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

		// Input: Child item trees
		std::unique_ptr<std::stringstream> childRowsInput(new std::stringstream);
		*childRowsInput << "% Child row facts" << std::endl;
		for(const auto& row : getCurrentRowCombination()) {
			for(const auto& item : row->getItems())
				*childRowsInput << "childItem(" << item << ")." << std::endl;
			// TODO auxItems, costs, etc.
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

		clasp.prepare();

		itemAtomInfos.clear();
		for(const auto& atom : gringoOutput->getItemAtomInfos())
			itemAtomInfos.emplace_back(ItemAtomInfo(atom, clasp.ctx.symbolTable()));
		auxItemAtomInfos.clear();
		for(const auto& atom : gringoOutput->getAuxItemAtomInfos())
			auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, clasp.ctx.symbolTable()));
		// TODO costs etc.
	}

	else {
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
	}

	// If there was already a conflict, clasp.prepared() is false
	if(clasp.prepared())
		asyncResult.reset(new BasicSolveIter(clasp));
}

bool Solver::endOfRowCandidates() const
{
	return !asyncResult || asyncResult->end();
}

void Solver::nextRowCandidate()
{
	assert(asyncResult);
	asyncResult->next();
}

void Solver::handleRowCandidate(long costBound)
{
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
