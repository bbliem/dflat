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

// Taken from gringo (GPL), made some modifications.
#pragma once
//}}}
#include <gringo/output/lparseoutputter.hh>
#include <clasp/literal.h>
#include <clasp/logic_program.h>
#include <unordered_map>

#include "../ItemTree.h"

namespace asp_utils {

class GringoOutputProcessor : public Gringo::Output::LparseOutputter
{
public:
	GringoOutputProcessor(Clasp::Asp::LogicProgram& out) : prg_(out) {
		false_ = prg_.newAtom();
		prg_.setCompute(false_, false);
	}
	unsigned falseUid() { return false_; }
	unsigned newUid()   { return prg_.newAtom(); }
	void printBasicRule(unsigned head, LitVec const &body);
	void printChoiceRule(AtomVec const &atoms, LitVec const &body);
	void printCardinalityRule(unsigned head, unsigned lower, LitVec const &body);
	void printWeightRule(unsigned head, unsigned lower, LitWeightVec const &body);
	void printMinimize(LitWeightVec const &body);
	void printDisjunctiveRule(AtomVec const &atoms, LitVec const &body);
	void finishRules()   { /* noop */ }
	void printSymbol(unsigned atomUid, Gringo::Value v);
	void printExternal(unsigned atomUid, Gringo::TruthValue type);
	void finishSymbols() { /* noop */ }
	bool &disposeMinimize();

private:
	void addBody(const LitVec& body);
	void addBody(const LitWeightVec& body);
	GringoOutputProcessor(const GringoOutputProcessor&);
	GringoOutputProcessor& operator=(const GringoOutputProcessor&);
	Clasp::Asp::LogicProgram& prg_;
	unsigned false_;
	std::stringstream str_;
	bool disposeMinimize_ = true;

protected:
	virtual void storeAtom(unsigned int atomUid, Gringo::Value v) = 0;
};

} // namespace asp_utils
