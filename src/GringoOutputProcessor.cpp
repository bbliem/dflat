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

// Originally from gringo. A few modifications were made.
// Copyright (c) 2010, Roland Kaminski <kaminski@cs.uni-potsdam.de>
//
// This file is part of gringo.
//
// gringo is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// gringo is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with gringo.  If not, see <http://www.gnu.org/licenses/>.

#include "GringoOutputProcessor.h"
#include <clasp/program_builder.h>
#include <clasp/shared_context.h>
#include <gringo/storage.h>
#include <gringo/domain.h>
#include <boost/lexical_cast.hpp>

GringoOutputProcessor::GringoOutputProcessor(bool ignoreOptimization)
	: LparseConverter(0, false), b_(0), lastUnnamed_(0), ignoreOptimization(ignoreOptimization)
#ifndef NDEBUG
	, itemArity(0)
#endif
{
}

void GringoOutputProcessor::initialize()
{
	LparseConverter::initialize();
	b_->setCompute(false_, false);
	lastUnnamed_ = atomUnnamed_.size();
	atomUnnamed_.clear();
}

void GringoOutputProcessor::printBasicRule(int head, const AtomVec &pos, const AtomVec &neg)
{
	b_->startRule();
	b_->addHead(head);
	foreach(AtomVec::value_type atom, neg) { b_->addToBody(atom, false); }
	foreach(AtomVec::value_type atom, pos) { b_->addToBody(atom, true); }
	b_->endRule();
}

void GringoOutputProcessor::printConstraintRule(int head, int bound, const AtomVec &pos, const AtomVec &neg)
{
	b_->startRule(Clasp::CONSTRAINTRULE, bound);
	b_->addHead(head);
	foreach(AtomVec::value_type atom, neg) { b_->addToBody(atom, false); }
	foreach(AtomVec::value_type atom, pos) { b_->addToBody(atom, true); }
	b_->endRule();
}

void GringoOutputProcessor::printChoiceRule(const AtomVec &head, const AtomVec &pos, const AtomVec &neg)
{
	b_->startRule(Clasp::CHOICERULE);
	foreach(AtomVec::value_type atom, head) { b_->addHead(atom); }
	foreach(AtomVec::value_type atom, neg) { b_->addToBody(atom, false); }
	foreach(AtomVec::value_type atom, pos) { b_->addToBody(atom, true); }
	b_->endRule();
}

void GringoOutputProcessor::printWeightRule(int head, int bound, const AtomVec &pos, const AtomVec &neg, const WeightVec &wPos, const WeightVec &wNeg)
{
	b_->startRule(Clasp::WEIGHTRULE, bound);
	b_->addHead(head);
	WeightVec::const_iterator itW = wNeg.begin();
	for(AtomVec::const_iterator it = neg.begin(); it != neg.end(); it++, itW++)
		b_->addToBody(*it, false, *itW);
	itW = wPos.begin();
	for(AtomVec::const_iterator it = pos.begin(); it != pos.end(); it++, itW++)
		b_->addToBody(*it, true, *itW);
	b_->endRule();
}

void GringoOutputProcessor::printMinimizeRule(const AtomVec &pos, const AtomVec &neg, const WeightVec &wPos, const WeightVec &wNeg)
{
	b_->startRule(Clasp::OPTIMIZERULE);
	WeightVec::const_iterator itW = wNeg.begin();
	for(AtomVec::const_iterator it = neg.begin(); it != neg.end(); it++, itW++)
		b_->addToBody(*it, false, *itW);
	itW = wPos.begin();
	for(AtomVec::const_iterator it = pos.begin(); it != pos.end(); it++, itW++)
		b_->addToBody(*it, true, *itW);
	b_->endRule();
}

void GringoOutputProcessor::printDisjunctiveRule(const AtomVec &head, const AtomVec &pos, const AtomVec &neg)
{
	(void)head;
	(void)pos;
	(void)neg;
	throw std::runtime_error("Error: clasp cannot handle disjunctive rules use option --shift!");
}

void GringoOutputProcessor::printComputeRule(int models, const AtomVec &pos, const AtomVec &neg)
{
	(void)models;
	foreach(AtomVec::value_type atom, neg) { b_->setCompute(atom, false); }
	foreach(AtomVec::value_type atom, pos) { b_->setCompute(atom, true); }
}

void GringoOutputProcessor::printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	std::stringstream ss;
	ss << name;
	if(arity > 0)
	{
		ValVec::const_iterator k = vals_.begin() + atom.second;
		ValVec::const_iterator end = k + arity;
		ss << "(";
		k->print(s_, ss);
		for(++k; k != end; ++k)
		{
			ss << ",";
			k->print(s_, ss);
		}
		ss << ")";
	}
	b_->setAtomName(atom.first, ss.str().c_str());
	atomUnnamed_[atom.first - lastUnnamed_] = false;

#ifndef NDEBUG
	// Arity of item must be used consistently
	if(name == "item") {
		assert(!itemArity || itemArity == arity);
		itemArity = arity;
	}
#endif

	if(name == "item") {
		if(arity == 2) {
			std::stringstream firstArg; // First argument
			std::ostringstream secondArg; // Second argument
			ValVec::const_iterator k = vals_.begin() + atom.second;
			(k++)->print(s_, firstArg);
			k->print(s_, secondArg);

			unsigned int firstArgNum = boost::lexical_cast<unsigned int>(firstArg.str());

			itemAtoms.push_back(ItemAtom(firstArgNum, secondArg.str(), atom.first));
		}
		else if(arity == 1) {
			std::ostringstream arg;
			ValVec::const_iterator k = vals_.begin() + atom.second;
			k->print(s_, arg);

			itemAtoms.push_back(ItemAtom(0, arg.str(), atom.first));
		}
	}
	else if(name == "extend") {
		assert(arity == 1);
		storeChildRowAtom(atom, extendAtoms);
	}
	else if(name == "count") {
		assert(arity == 1);
		storeNumberAtom(atom, countAtoms);
	}
	else if(!ignoreOptimization && name == "currentCost") {
		assert(arity == 1);
		storeNumberAtom(atom, currentCostAtoms);
	}
	else if(!ignoreOptimization && name == "cost") {
		assert(arity == 1);
		storeNumberAtom(atom, costAtoms);
	}
}

void GringoOutputProcessor::printExternalTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	(void)atom;
	(void)arity;
	(void)name;
}

uint32_t GringoOutputProcessor::symbol()
{
	uint32_t atom = b_->newAtom();
	atomUnnamed_.resize(atom + 1 - lastUnnamed_, true);
	return atom;
}

void GringoOutputProcessor::doFinalize()
{
	printSymbolTable();
	for(uint32_t i = 0; i < atomUnnamed_.size(); i++) { if(atomUnnamed_[i]) { b_->setAtomName(i + lastUnnamed_, 0); } }
	lastUnnamed_+= atomUnnamed_.size();
	atomUnnamed_.clear();
}

const LparseConverter::SymbolMap &GringoOutputProcessor::symbolMap(uint32_t domId) const
{
	return symTab_[domId];
}

ValRng GringoOutputProcessor::vals(Domain *dom, uint32_t offset) const
{
	return ValRng(vals_.begin() + offset, vals_.begin() + offset + dom->arity());
}

inline void GringoOutputProcessor::storeChildRowAtom(const AtomRef& atom, LongToSymbolTableKey& store)
{
	std::stringstream firstArg; // First argument
	ValVec::const_iterator k = vals_.begin() + atom.second;
	k->print(s_, firstArg);
	store[std::strtol(firstArg.str().c_str()+1, 0, 0)] = atom.first;
}

inline void GringoOutputProcessor::storeNumberAtom(const AtomRef& atom, LongToSymbolTableKey& store)
{
	std::stringstream firstArg; // First argument
	ValVec::const_iterator k = vals_.begin() + atom.second;
	k->print(s_, firstArg);
	store[std::strtol(firstArg.str().c_str(), 0, 0)] = atom.first;
}
