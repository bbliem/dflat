/*{{{
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
#include "trees/GringoOutputProcessor.h"

namespace solver { namespace asp {

namespace {

std::unique_ptr<GringoOutputProcessor> newGringoOutputProcessor(Clasp::Asp::LogicProgram& claspProgramBuilder, const ChildItemTrees& childItemTrees, bool tableMode)
{
	if(tableMode)
		return std::unique_ptr<GringoOutputProcessor>(new tables::GringoOutputProcessor(claspProgramBuilder, childItemTrees));
	else
		return std::unique_ptr<GringoOutputProcessor>(new trees::GringoOutputProcessor(claspProgramBuilder, childItemTrees));
}

std::unique_ptr<ClaspCallback> newClaspCallback(bool tableMode, const Gringo::Output::LparseOutputter& gringoOutput, const ChildItemTrees& childItemTrees, const Application& app, bool root)
{
	if(tableMode)
		return std::unique_ptr<ClaspCallback>(new tables::ClaspCallback(dynamic_cast<const tables::GringoOutputProcessor&>(gringoOutput), childItemTrees, app, root));
	else
		return std::unique_ptr<ClaspCallback>(new trees::ClaspCallback(dynamic_cast<const trees::GringoOutputProcessor&>(gringoOutput), childItemTrees, app));
}

} // anonymous namespace

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool tableMode)
	: ::Solver(decomposition, app)
	, encodingFiles(encodingFiles)
	, tableMode(tableMode)
{
	Gringo::message_printer()->disable(Gringo::W_ATOM_UNDEFINED);
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
		childItemTrees.emplace(child->getRoot().getGlobalId(), std::move(itree));
	}

	// Input: Child item trees
	std::unique_ptr<std::stringstream> childItemTreesInput(new std::stringstream);
	*childItemTreesInput << "% Child item tree facts" << std::endl;

	for(const auto& childItemTree : childItemTrees) {
		std::ostringstream rootItemSetName;
		rootItemSetName << 'n' << childItemTree.first;
		declareItemTree(*childItemTreesInput, childItemTree.second.get(), tableMode, childItemTree.first, rootItemSetName.str());
	}

	app.getPrinter().solverInvocationInput(decomposition, childItemTreesInput->str());

	// Input: Original problem instance
	std::unique_ptr<std::stringstream> instanceInput(new std::stringstream);
	*instanceInput << app.getInputString();

	// Input: Decomposition
	std::unique_ptr<std::stringstream> decompositionInput(new std::stringstream);
	declareDecomposition(decomposition, *decompositionInput);

	app.getPrinter().solverInvocationInput(decomposition, decompositionInput->str());

	// Set up ASP solver
	Clasp::ClaspConfig config;
	config.enumerate.numModels = 0;
	Clasp::ClaspFacade clasp;
	Clasp::Asp::LogicProgram& claspProgramBuilder = dynamic_cast<Clasp::Asp::LogicProgram&>(clasp.start(config, Clasp::Problem_t::ASP, true));
	std::unique_ptr<Gringo::Output::LparseOutputter> lpOut(newGringoOutputProcessor(claspProgramBuilder, childItemTrees, tableMode));
	std::unique_ptr<Gringo::Output::OutputBase> out(new Gringo::Output::OutputBase({}, *lpOut));
	Gringo::Input::Program program;
	Gringo::Scripts scripts;
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

	clasp.prepare();
	const bool root = decomposition.getParents().empty();
	std::unique_ptr<ClaspCallback> cb(newClaspCallback(tableMode, *lpOut, childItemTrees, app, root));
	cb->prepare(clasp.ctx.symbolTable());
	clasp.solve(cb.get());

	ItemTreePtr result = cb->finalize(root, app.isPruningDisabled() == false || root);
	app.getPrinter().solverInvocationResult(decomposition, result.get());
	return result;
}

void Solver::declareDecomposition(const Decomposition& decomposition, std::ostream& out)
{
	out << "% Decomposition facts" << std::endl;
	out << "currentNode(" << decomposition.getRoot().getGlobalId() << ")." << std::endl;
	for(const auto& v : decomposition.getRoot().getBag()) {
		out << "bag(" << decomposition.getRoot().getGlobalId() << ',' << v << "). ";
		out << "current(" << v << ")." << std::endl;
	}

	out << "#const numChildNodes=" << decomposition.getChildren().size() << '.' << std::endl;
	if(decomposition.getChildren().empty())
		out << "initial." << std::endl;
	else {
		for(const auto& child : decomposition.getChildren()) {
			out << "childNode(" << child->getRoot().getGlobalId() << ")." << std::endl;
			for(const auto& v : child->getRoot().getBag()) {
				out << "bag(" << child->getRoot().getGlobalId() << ',' << v << "). ";
				out << "-introduced(" << v << ")." << std::endl; // Redundant
			}
		}
	}

	if(decomposition.getParents().empty())
		out << "final." << std::endl;
	else {
		for(const auto& parent : decomposition.getParents()) {
			out << "parentNode(" << parent->getRoot().getGlobalId() << ")." << std::endl;
			for(const auto& v : parent->getRoot().getBag())
				out << "bag(" << parent->getRoot().getGlobalId() << ',' << v << ")." << std::endl;
		}
	}

	// Redundant predicates for convenience...
	out << "introduced(X) :- current(X), not -introduced(X)." << std::endl;
	out << "removed(X) :- childNode(N), bag(N,X), not current(X)." << std::endl;
}

void Solver::declareItemTree(std::ostream& out, const ItemTree* itemTree, bool tableMode, unsigned int nodeId, const std::string& itemSetName, const std::string& parent, unsigned int level)
{
	if(!itemTree)
		return;

	// Declare this item set
	if(tableMode) {
		if(parent.empty() == false)
			out << "childRow(" << itemSetName << ',' << nodeId << ")." << std::endl;
	} else {
		out << "atLevel(" << itemSetName << ',' << level << ")." << std::endl;
		out << "atNode(" << itemSetName << ',' << nodeId << ")." << std::endl;
		if(parent.empty()) {
			out << "root(" << itemSetName << ")." << std::endl;
			out << "rootOf(" << itemSetName << ',' << nodeId << ")." << std::endl;
		} else {
			out << "sub(" << parent << ',' << itemSetName << ")." << std::endl;
			if(itemTree->getChildren().empty()) {
				out << "leaf(" << itemSetName << ")." << std::endl;
				out << "leafOf(" << itemSetName << ',' << nodeId << ")." << std::endl;
			}
		}
	}
	for(const auto& item : itemTree->getRoot()->getItems())
		out << "childItem(" << itemSetName << ',' << item << ")." << std::endl;
	for(const auto& item : itemTree->getRoot()->getAuxItems())
		out << "childAuxItem(" << itemSetName << ',' << item << ")." << std::endl;

	// Declare item tree node type
	switch(itemTree->getRoot()->getType()) {
		case ItemTreeNode::Type::UNDEFINED:
			break;
		case ItemTreeNode::Type::OR:
			out << "childOr(" << itemSetName << ")." << std::endl;
			break;
		case ItemTreeNode::Type::AND:
			out << "childAnd(" << itemSetName << ")." << std::endl;
			break;
		case ItemTreeNode::Type::ACCEPT:
			out << "childAccept(" << itemSetName << ")." << std::endl;
			break;
		case ItemTreeNode::Type::REJECT:
			out << "childReject(" << itemSetName << ")." << std::endl;
			break;
	}

	// If this is a leaf, declare cost
	const ItemTree::Children& children = itemTree->getChildren();
	if(children.empty()) {
		out << "childCost(" << itemSetName << ',' << itemTree->getRoot()->getCost() << ")." << std::endl;
	}
	else {
		// Declare child item sets
		size_t i = 0;
		for(const auto& child : children) {
			std::ostringstream childName;
			childName << itemSetName << '_' << i++;
			declareItemTree(out, child.get(), tableMode, nodeId, childName.str(), itemSetName, level+1);
		}
	}
}

}} // namespace solver::asp
