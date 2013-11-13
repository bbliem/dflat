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

#include <clasp/clasp_facade.h>

#include "GringoOutputProcessor.h"

class Debugger;

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

	using ChildItemTrees = GringoOutputProcessor::ChildItemTrees;

	ClaspCallback(const ChildItemTrees& childItemTrees, bool prune, const Debugger& debugger);

	// Call this after all answer sets have been processed. It returns the resulting item tree (and calls finalize() on it).
	ItemTreePtr finalize();

	// Called if the current configuration contains unsafe/unreasonable options
	virtual void warning(const char* msg) override;

	// Called for important events, e.g. a model has been found
	virtual void event(const Clasp::Solver& s, Clasp::ClaspFacade::Event e, Clasp::ClaspFacade& f) override;

protected:
	template<typename T, typename F>
	static void forEachTrue(const Clasp::Solver& s, const std::vector<AtomInfo<T>>& atomInfos, F function)
	{
		for(const auto& atom : atomInfos) {
			if(s.isTrue(atom.literal))
				function(atom.arguments);
		}
	}

	template<typename T, typename F>
	static void forEachTrueLimited(const Clasp::Solver& s, const std::vector<AtomInfo<T>>& atomInfos, F function)
	{
		for(const auto& atom : atomInfos) {
			if(s.isTrue(atom.literal)) {
				if(function(atom.arguments) == false)
					return;
			}
		}
	}

	template<typename T, typename F>
	static void forFirstTrue(const Clasp::Solver& s, const std::vector<AtomInfo<T>>& atomInfos, F function)
	{
		for(const auto& atom : atomInfos) {
			if(s.isTrue(atom.literal)) {
				function(atom.arguments);
				return;
			}
		}
	}

#ifndef DISABLE_CHECKS
	template<typename T>
	static size_t countTrue(const Clasp::Solver& s, const std::vector<AtomInfo<T>>& atomInfos)
	{
		return std::count_if(atomInfos.begin(), atomInfos.end(), [&s](const AtomInfo<T>& atom) {
			return s.isTrue(atom.literal);
		});
	}
#endif

	ItemTreePtr itemTree;
	const ChildItemTrees& childItemTrees;
	const Debugger& debugger;
	bool prune;
};

}} // namespace solver::asp
