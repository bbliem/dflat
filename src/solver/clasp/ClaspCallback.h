/*{{{
Copyright 2012-2015, Bernhard Bliem
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
//}}}
#include <clasp/clasp_facade.h>

#include "GringoOutputProcessor.h"

class Application;

namespace solver {
class Asp;
namespace clasp {
class ItemSetLookupTable;

// Gets called by clasp whenever a model has been found
class ClaspCallback : public Clasp::EventHandler
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

	ClaspCallback(const Application&);

	// Call this after all answer sets have been processed. It returns the resulting item tree (and calls finalize() on it).
	virtual ItemTreeChildPtr finalize(bool pruneUndefined, bool pruneRejecting);

	// Called when a model has been found
	virtual bool onModel(const Clasp::Solver&, const Clasp::Model&) override;

	// Returns the item tree that is being constructed by this object.
	// You may use this method before all answer sets have been processed to get the current state of the item tree.
	// To obtain the finished product, you should probably call finalize() instead.
	const ItemTreeChildPtr& getItemTree() const;

	// Call this when clasp's symbol table is available to let this object get the clasp literals corresponding to atoms using output predicates
	virtual void prepare(const Clasp::SymbolTable&) {}

protected:
	template<typename T, typename F>
	static void forEachTrue(const Clasp::Model& m, const std::vector<AtomInfo<T>>& atomInfos, F function)
	{
		for(const auto& atom : atomInfos) {
			if(m.isTrue(atom.literal))
				function(atom.arguments);
		}
	}

	template<typename T, typename F>
	static void forEachTrueLimited(const Clasp::Model& m, const std::vector<AtomInfo<T>>& atomInfos, F function)
	{
		for(const auto& atom : atomInfos) {
			if(m.isTrue(atom.literal)) {
				if(function(atom.arguments) == false)
					return;
			}
		}
	}

	template<typename T, typename F>
	static void forFirstTrue(const Clasp::Model& m, const std::vector<AtomInfo<T>>& atomInfos, F function)
	{
		for(const auto& atom : atomInfos) {
			if(m.isTrue(atom.literal)) {
				function(atom.arguments);
				return;
			}
		}
	}

#ifndef DISABLE_CHECKS
	template<typename T>
	static size_t countTrue(const Clasp::Model& m, const std::vector<AtomInfo<T>>& atomInfos)
	{
		return std::count_if(atomInfos.begin(), atomInfos.end(), [&m](const AtomInfo<T>& atom) {
			return m.isTrue(atom.literal);
		});
	}
#endif

	ItemTreeChildPtr itemTree;
	const Application& app;
	bool prune;
};

}} // namespace solver::clasp
