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
//}}}
#include "GringoOutputProcessor.h"
#include "ClaspCallback.h"

namespace solver { namespace asp {

GringoOutputProcessor::GringoOutputProcessor(Clasp::Asp::LogicProgram& out)
	: prg_(out)
{
	false_ = prg_.newAtom();
	prg_.setCompute(false_, false);
#ifdef DEBUG_OUTPUT
	std::cerr << "pb.setCompute(" << false_ << ",false);\n";
#endif
}

void GringoOutputProcessor::addBody(const LitVec& body) {
    for (auto x : body) {
        prg_.addToBody((Clasp::Var)std::abs(x), x > 0);
#ifdef DEBUG_OUTPUT
        std::cerr << ".addToBody(" << std::abs(x) << "," << (x > 0) << ")";
#endif
    }
}
void GringoOutputProcessor::addBody(const LitWeightVec& body) {
    for (auto x : body) {
        prg_.addToBody((Clasp::Var)std::abs(x.first), x.first > 0, x.second);
#ifdef DEBUG_OUTPUT
        std::cerr << ".addToBody(" << std::abs(x.first) << "," << (x.first > 0) << "," << x.second << ")";
#endif
    }
}
void GringoOutputProcessor::printBasicRule(unsigned head, LitVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule().addHead(" << head << ")";
#endif
    prg_.startRule().addHead(head);
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printChoiceRule(AtomVec const &atoms, LitVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::CHOICERULE)";
    for (auto x : atoms) { std::cerr << ".addHead(" << x << ")"; }
#endif
    prg_.startRule(Clasp::Asp::CHOICERULE);
    for (auto x : atoms) { prg_.addHead(x); }
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printCardinalityRule(unsigned head, unsigned lower, LitVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::CONSTRAINTRULE, " << lower << ").addHead(" << head << ")";
#endif
    prg_.startRule(Clasp::Asp::CONSTRAINTRULE, lower).addHead(head);
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printWeightRule(unsigned head, unsigned lower, LitWeightVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::WEIGHTRULE, " << lower << ").addHead(" << head << ")";
#endif
    prg_.startRule(Clasp::Asp::WEIGHTRULE, lower).addHead(head);
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printMinimize(LitWeightVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::OPTIMIZERULE)";
#endif
    prg_.startRule(Clasp::Asp::OPTIMIZERULE);
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printDisjunctiveRule(AtomVec const &atoms, LitVec const &body) {
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.startRule(Clasp::Asp::DISJUNCTIVERULE)";
    for (auto x : atoms) { std::cerr << ".addHead(" << x << ")"; }
#endif
    prg_.startRule(Clasp::Asp::DISJUNCTIVERULE);
    for (auto x : atoms) { prg_.addHead(x); }
    addBody(body);
    prg_.endRule();
#ifdef DEBUG_OUTPUT
    std::cerr << ".endRule();\n";
#endif
}

void GringoOutputProcessor::printSymbol(unsigned atomUid, Gringo::Value v) {
	if (v.type() == Gringo::Value::ID || v.type() == Gringo::Value::STRING) {
		prg_.setAtomName(atomUid, (*v.string()).c_str());
	}
	else {
		str_.str("");
		v.print(str_);
		prg_.setAtomName(atomUid, str_.str().c_str());
	}

	// BB: Process special predicates
	storeAtom(atomUid, v);

#ifdef DEBUG_OUTPUT
    std::cerr << "pb.setAtomName(" << atomUid << ",\"" << v << "\");\n";
#endif
}

void GringoOutputProcessor::printExternal(unsigned atomUid) {
    prg_.freeze(atomUid);
#ifdef DEBUG_OUTPUT
    std::cerr << "pb.freeze(" << atomUid << ");\n";
#endif
}

bool &GringoOutputProcessor::disposeMinimize() {
    return disposeMinimize_;
}

}} // namespace solver::asp
