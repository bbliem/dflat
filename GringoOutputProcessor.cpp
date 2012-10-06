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

GringoOutputProcessor::GringoOutputProcessor(bool shiftDisj)
	: LparseConverter(0, shiftDisj)
	, b_(0)
	, lastUnnamed_(0)
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
	std::stringstream arg; // The first argument
	ss << name;
	if(arity > 0)
	{
		ValVec::const_iterator k = vals_.begin() + atom.second;
		ValVec::const_iterator end = k + arity;
		ss << "(";
		k->print(s_, arg);
		ss << arg.str();
		for(++k; k != end; ++k)
		{
			ss << ",";
			k->print(s_, ss);
		}
		ss << ")";
	}
	b_->setAtomName(atom.first, ss.str().c_str());
	atomUnnamed_[atom.first - lastUnnamed_] = false;

	if(name == "mAtom") // FIXME: I'm dirty
		mAtom.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first)); // +1 because of the leading "m" of the argument
	else if(name == "mRule")
		mRule.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first));
	else if(name == "chosenOldM")
		chosenOldM.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first));
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

GringoOutputProcessor::~GringoOutputProcessor()
{
}
