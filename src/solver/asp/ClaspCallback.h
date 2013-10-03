/*
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

#pragma once

#include <unordered_map>
#include <clasp/clasp_facade.h>

#include "../../ItemTree.h"
#include "GringoOutputProcessor.h"

namespace solver { namespace asp {
class ItemSetLookupTable;

// Gets called by clasp whenever a model has been found
class ClaspCallback : public Clasp::ClaspFacade::Callback
{
public:
	template<typename T>
	struct AtomInfo {
		AtomInfo(const GringoOutputProcessor::AtomInfo<T>& gringoAtomInfo, const Clasp::SymbolTable& symTab)
			: arguments(gringoAtomInfo.arguments) // XXX move?
			, literal(symTab[gringoAtomInfo.symbolTableKey].lit)
		{
		}

		T arguments;
		Clasp::Literal literal;
	};

	// Key: Global ID of child node; value: the child node's item tree
	typedef std::unordered_map<unsigned int, ItemTreePtr> ChildItemTrees;

	ClaspCallback(const ChildItemTrees& childItemTrees);

	// Call this after all answer sets have been processed. It returns the resulting item tree (and calls prepareRandomAccessToChildren() on it).
	ItemTreePtr getItemTree();

	// Called if the current configuration contains unsafe/unreasonable options
	virtual void warning(const char* msg) override;

	// Called on entering/exiting a state
	virtual void state(Clasp::ClaspFacade::Event, Clasp::ClaspFacade&) override = 0;

	// Called for important events, e.g. a model has been found
	virtual void event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f) override = 0;

protected:
	ItemTreePtr itemTree;
	const ChildItemTrees& childItemTrees;
};

}} // namespace solver::asp
