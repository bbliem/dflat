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

Solver::Solver(const Decomposition& decomposition, const Application& app, const std::vector<std::string>& encodingFiles, bool reground, BranchAndBoundLevel bbLevel, bool cardinalityCost)
	: ::LazySolver(decomposition, app, bbLevel)
	, reground(reground)
    , cardinalityCost(cardinalityCost)
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
				else if(n == "childCost") {
					ASP_CHECK(v.args().size() == 1, "'childCost' predicate does not have arity 1");
					std::ostringstream argument;
					v.args().front().print(argument);
                    self->costsToLitIndices.emplace(std::stol(argument.str()), self->literals.size());
					self->literals.push_back(Clasp::posLit(atomUid));
				}
				else if(n == "childCounter") {
                    ASP_CHECK(v.args().size() <= 2, "'childCounter' predicate does not have greater than 1");
                    std::ostringstream counter;
                    v.args().front().print(counter);
                    std::ostringstream value;
                    v.args()[1].print(value);
                    self->countersToLitIndices.emplace(std::make_pair(counter.str(),std::stol(value.str())), self->literals.size());
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
        for(const auto& atom : gringoOutput.getCurrentCostAtomInfos())
			currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, claspProgramBuilder));
        for(const auto& atom : gringoOutput.getCostAtomInfos())
            costAtomInfos.emplace_back(CostAtomInfo(atom, claspProgramBuilder));
 //       for(const auto& atom : gringoOutput.getCounterRemAtomInfos())
 //           counterRemAtomInfos.emplace_back(CounterRemAtomInfo(atom, claspProgramBuilder));
        for(const auto& atom : gringoOutput.getCounterRemAtomInfos())
            counterRemAtomInfos.insert(std::pair<std::string, Clasp::Literal>(atom.arguments.counterName, Clasp::Literal(claspProgramBuilder.getLiteral(atom.atomId))));
        for(const auto& counterIncAtomInfos : gringoOutput.getAllCounterIncAtomInfos())
            for(const auto& atom : counterIncAtomInfos.second)
                allCounterIncAtomInfos[counterIncAtomInfos.first].emplace_back(CounterIncAtomInfo(atom, claspProgramBuilder));
        for(const auto& currentCounterIncAtomInfos : gringoOutput.getAllCurrentCounterIncAtomInfos())
            for(const auto& atom : currentCounterIncAtomInfos.second)
                allCurrentCounterIncAtomInfos[currentCounterIncAtomInfos.first].emplace_back(CurrentCounterIncAtomInfo(atom, claspProgramBuilder));
        for(const auto& counterAtomInfos : gringoOutput.getAllCounterAtomInfos())
            for(const auto& atom : counterAtomInfos.second)
                allCounterAtomInfos[counterAtomInfos.first].emplace_back(CounterAtomInfo(atom, claspProgramBuilder));
        for(const auto& currentCounterAtomInfos : gringoOutput.getAllCurrentCounterAtomInfos())
            for(const auto& atom : currentCounterAtomInfos.second)
                allCurrentCounterAtomInfos[currentCounterAtomInfos.first].emplace_back(CurrentCounterAtomInfo(atom, claspProgramBuilder));

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
			for(const auto& item : row->getCounters())
                *childRowsInput << "childCounter(" << item.first << "," << item.second << ")." << std::endl;
			const auto& item = row->getCost();
			*childRowsInput << "childCost(" << item << ")." << std::endl;
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
        for(const auto& atom : gringoOutput.getAuxItemAtomInfos())
            auxItemAtomInfos.emplace_back(AuxItemAtomInfo(atom, claspProgramBuilder));
        for(const auto& atom : gringoOutput.getCurrentCostAtomInfos())
            currentCostAtomInfos.emplace_back(CurrentCostAtomInfo(atom, claspProgramBuilder));
        for(const auto& atom : gringoOutput.getCostAtomInfos())
            costAtomInfos.emplace_back(CostAtomInfo(atom, claspProgramBuilder));
        for(const auto& atom : gringoOutput.getCounterRemAtomInfos())
            counterRemAtomInfos.insert(std::pair<std::string, Clasp::Literal>(atom.arguments.counterName, Clasp::Literal(claspProgramBuilder.getLiteral(atom.atomId))));
        for(const auto& counterIncAtomInfos : gringoOutput.getAllCounterIncAtomInfos())
            for(const auto& atom : counterIncAtomInfos.second)
                allCounterIncAtomInfos[counterIncAtomInfos.first].emplace_back(CounterIncAtomInfo(atom, claspProgramBuilder));
        for(const auto& currentCounterIncAtomInfos : gringoOutput.getAllCurrentCounterIncAtomInfos())
            for(const auto& atom : currentCounterIncAtomInfos.second)
                allCurrentCounterIncAtomInfos[currentCounterIncAtomInfos.first].emplace_back(CurrentCounterIncAtomInfo(atom, claspProgramBuilder));
        for(const auto& counterAtomInfos : gringoOutput.getAllCounterAtomInfos())
            for(const auto& atom : counterAtomInfos.second)
                allCounterAtomInfos[counterAtomInfos.first].emplace_back(CounterAtomInfo(atom, claspProgramBuilder));
        for(const auto& currentCounterAtomInfos : gringoOutput.getAllCurrentCounterAtomInfos())
            for(const auto& atom : currentCounterAtomInfos.second)
                allCurrentCounterAtomInfos[currentCounterAtomInfos.first].emplace_back(CurrentCounterAtomInfo(atom, claspProgramBuilder));
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

            if(countersToLitIndices.size() > 0) {
                for(const auto& item : row->getCounters()) {
                    assert(countersToLitIndices.find(item) != countersToLitIndices.end());
                    assert(countersToLitIndices.at(item) < literals.size());
    #ifdef DISABLE_CHECKS
                    literals[countersToLitIndices.at(item)].watch();
    #else
                    try {
                        literals[countersToLitIndices.at(item)].watch();
                    }
                    catch(const std::out_of_range&) {
                        std::ostringstream msg;
                        msg << "Unknown variable; atom childCounter(" << item.first << "," << item.second << ") not shown or not declared as external?";
                        throw std::runtime_error(msg.str());
                    }
    #endif
                }
            }/*
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
            encodingChecker->check();*/
            const auto& item = row->getCost();
            //if((item != 0 || costsToLitIndices.find(item) != costsToLitIndices.end()) && !cardinalityCost)
            if(!cardinalityCost && costsToLitIndices.size() > 0){
                assert(costsToLitIndices.find(item) != costsToLitIndices.end());
                assert(costsToLitIndices.at(item) < literals.size());
#ifdef DISABLE_CHECKS
                literals[costsToLitIndices.at(item)].watch();
#else
                try {
                    literals[costsToLitIndices.at(item)].watch();
                }
                catch(const std::out_of_range&) {
                    std::ostringstream msg;
                    msg << "Unknown variable; atom childCost(" << item << ") not shown or not declared as external?";
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

	// Check if counters are used correctly {{{
#ifndef DISABLE_CHECKS
	for(const auto& counterAtomInfos : allCounterAtomInfos) {
		if(counterAtomInfos.first.compare("cost") == 0) {
			ASP_CHECK(countTrue(m, allCounterIncAtomInfos[counterAtomInfos.first]) == 0 || countTrue(m, counterAtomInfos.second) == 0,
				"Both 'counter'/'cost' and 'counterInc' predicates used for setting the cost");
			ASP_CHECK(countTrue(m, counterAtomInfos.second) <= 1, "More than one true atom for setting the cost");
		} else {
			ASP_CHECK(countTrue(m, allCounterIncAtomInfos[counterAtomInfos.first]) == 0 || countTrue(m, counterAtomInfos.second) == 0,
				"Both 'counter' and 'counterInc' predicates used for setting the " + counterAtomInfos.first + " counter");
			ASP_CHECK(countTrue(m, counterAtomInfos.second) <= 1, "More than one true atom for setting the " + counterAtomInfos.first + " counter");
		}
	}

	for(const auto& currentCounterAtomInfos : allCurrentCounterAtomInfos) {
		if(currentCounterAtomInfos.first.compare("cost") == 0){
			ASP_CHECK(countTrue(m, currentCounterAtomInfos.second) == 0 || countTrue(m, allCounterAtomInfos[currentCounterAtomInfos.first]) == 1,
					  "True current cost atom without true cost atom");
			ASP_CHECK(countTrue(m, allCurrentCounterIncAtomInfos[currentCounterAtomInfos.first]) == 0 || countTrue(m, currentCounterAtomInfos.second) == 0,
				"Both 'currentCounter'/'currentCost' and 'currentCounterInc' predicates used for setting the current cost");
			ASP_CHECK(countTrue(m, currentCounterAtomInfos.second) <= 1, "More than one true atom for setting the current cost");
		} else {
			ASP_CHECK(countTrue(m, currentCounterAtomInfos.second) == 0 || countTrue(m, allCounterAtomInfos[currentCounterAtomInfos.first]) == 1,
					  "True " + currentCounterAtomInfos.first + " current counter atom without true " + currentCounterAtomInfos.first + " counter atom");
			ASP_CHECK(countTrue(m, allCurrentCounterIncAtomInfos[currentCounterAtomInfos.first]) == 0 || countTrue(m, currentCounterAtomInfos.second) == 0,
				"Both 'currentCounter' and 'currentCounterInc' predicates used for setting the " + currentCounterAtomInfos.first + " current counter");
			ASP_CHECK(countTrue(m, currentCounterAtomInfos.second) <= 1, "More than one true atom for setting the " + currentCounterAtomInfos.first + " current counter");
		}
	}
#endif // }}}	
	
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

	assert(itemTree);
	// Create item tree node {{{
	std::shared_ptr<ItemTreeNode> node(new ItemTreeNode(std::move(items), std::move(auxItems), {getCurrentRowCombination()}));
	// }}}

    if(!app.isOptimizationDisabled()) {

        long cost = 0;
        long currentCost = 0;

        // Set (current) counters and compute (if optimization is not disabled) cost {{{
        std::map<std::string,long> counterValues;
        for(const auto& counterIncAtomInfos : allCounterIncAtomInfos) {
            // Note that here forFirstTrue is not sufficient
            forEachTrue(m, counterIncAtomInfos.second, [&counterValues](const GringoOutputProcessor::CounterIncAtomArguments& arguments) {
                    counterValues[arguments.counterName] += arguments.counterInc;
            });
        }

        for(const auto& row : getCurrentRowCombination()) {
            for(const auto& counter : row->getCounters())
                counterValues[counter.first] += counter.second;
            counterValues["cost"] += row->getCost();
        }

        for(const auto& counterAtomInfos : allCounterAtomInfos) {
            forFirstTrue(m, counterAtomInfos.second, [&counterValues](const GringoOutputProcessor::CounterAtomArguments& arguments) {
                    counterValues[arguments.counterName] = arguments.counter;
            });
        }
        forFirstTrue(m, costAtomInfos, [&counterValues](const GringoOutputProcessor::CostAtomArguments& arguments) {
                counterValues["cost"] = arguments.cost;
        });

        for(const auto& counterValue : counterValues) {
            if(counterRemAtomInfos.find(counterValue.first) == counterRemAtomInfos.end() || !m.isTrue(counterRemAtomInfos[counterValue.first])) {
                if(counterValue.first == "cost") {
                    if(!app.isOptimizationDisabled())
                        cost = counterValue.second;
                }
                else
                    node->setCounter(counterValue.first, counterValue.second);
            }
        }

    // Possibly update counters of root [XXX necessary?]
    //	for(const auto& counter : itemTree->getNode()->getCounters())
    //		itemTree->getNode()->setCounter(counter.first, std::min(itemTree->getNode()->getCounter(counter.first), counterValues[counter.first]));

        std::map<std::string,long> currentCounterValues;
        for(const auto& currentCounterIncAtomInfos : allCurrentCounterIncAtomInfos) {
            // Note that here forFirstTrue is not sufficient
            forEachTrue(m, currentCounterIncAtomInfos.second, [&currentCounterValues](const GringoOutputProcessor::CurrentCounterIncAtomArguments& arguments) {
                    currentCounterValues[arguments.currentCounterName] += arguments.currentCounterInc;
            });
        }

        for(const auto& row : getCurrentRowCombination()) {
            for(const auto& currentCounter : row->getCurrentCounters())
                currentCounterValues[currentCounter.first] += row->getCurrentCounter(currentCounter.first);
            currentCounterValues["cost"] += row->getCurrentCost();
        }

        for(const auto& currentCounterAtomInfos : allCurrentCounterAtomInfos) {
            forFirstTrue(m, currentCounterAtomInfos.second, [&currentCounterValues](const GringoOutputProcessor::CurrentCounterAtomArguments& arguments) {
                    currentCounterValues[arguments.currentCounterName] = arguments.currentCounter;
            });
        }
        forFirstTrue(m, currentCostAtomInfos, [&currentCounterValues](const GringoOutputProcessor::CurrentCostAtomArguments& arguments) {
                currentCounterValues["cost"] = arguments.currentCost;
        });

        for(const auto& currentCounterValue : currentCounterValues) {
            if(counterRemAtomInfos.find(currentCounterValue.first) == counterRemAtomInfos.end() || !m.isTrue(counterRemAtomInfos[currentCounterValue.first])) {
                if(currentCounterValue.first == "cost") {
                    if(!app.isOptimizationDisabled())
                        currentCost = currentCounterValue.second;
                }
                else
                    node->setCurrentCounter(currentCounterValue.first, currentCounterValue.second);
            }
        }
        // }}}

        if(cardinalityCost) {
            cost = node->getItems().size();
            for(const auto& row : node->getExtensionPointers().front()) {
                const auto& oldItems = row->getItems();
                ItemTreeNode::Items intersection;
                std::set_intersection(node->getItems().begin(), node->getItems().end(), oldItems.begin(), oldItems.end(), std::inserter(intersection, intersection.begin()));
                cost += row->getCost() - intersection.size();
            }

            currentCost = node->getItems().size();
        }

        if(cost >= costBound) {
    //		++discardedModels;
            newestRow = itemTree->getChildren().end();
            return;
        }

        node->setCost(cost);
        node->setCurrentCost(currentCost);

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
