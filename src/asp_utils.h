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

#pragma once
//}}}
#include <clasp/clasp_facade.h>
#include <gringo/control.hh>

#include "Decomposition.h"
#include "Instance.h"

#ifdef DISABLE_CHECKS
#	define ASP_CHECK(cond, error)
#else
#	define ASP_CHECK(cond, error)\
	if((cond) == false)                 \
		throw std::runtime_error(error);
#endif

namespace asp_utils {
	class DummyGringoModule : public Gringo::GringoModule
	{
		virtual Gringo::Control *newControl(int argc, char const **argv) override { throw std::logic_error("DummyGringoModule"); };
		virtual void freeControl(Gringo::Control *ctrl) override { throw std::logic_error("DummyGringoModule"); };
		virtual Gringo::Value parseValue(std::string const &repr) override { throw std::logic_error("DummyGringoModule"); };
	};

	template<typename T>
	struct GringoAtomInfo {
		T arguments;
		Clasp::Var atomId;
	};

	template<typename T>
	struct ClaspAtomInfo {
		ClaspAtomInfo(const GringoAtomInfo<T>& gringoAtomInfo, const Clasp::Asp::LogicProgram& prg)
			: arguments(gringoAtomInfo.arguments) // XXX move?
			, literal(prg.getLiteral(gringoAtomInfo.atomId))
		{
		}

		T arguments;
		Clasp::Literal literal;
	};

	// Iteratate over models
	template<typename T, typename F>
	static void forEachTrue(const Clasp::Model& m, const std::vector<ClaspAtomInfo<T>>& atomInfos, F function)
	{
		for(const auto& atom : atomInfos) {
			if(m.isTrue(atom.literal))
				function(atom.arguments);
		}
	}

	template<typename T, typename F>
	static void forEachTrueLimited(const Clasp::Model& m, const std::vector<ClaspAtomInfo<T>>& atomInfos, F function)
	{
		for(const auto& atom : atomInfos) {
			if(m.isTrue(atom.literal)) {
				if(function(atom.arguments) == false)
					return;
			}
		}
	}

	template<typename T, typename F>
	static void forFirstTrue(const Clasp::Model& m, const std::vector<ClaspAtomInfo<T>>& atomInfos, F function)
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
	static size_t countTrue(const Clasp::Model& m, const std::vector<ClaspAtomInfo<T>>& atomInfos)
	{
		return std::count_if(atomInfos.begin(), atomInfos.end(), [&m](const ClaspAtomInfo<T>& atom) {
			return m.isTrue(atom.literal);
		});
	}
#endif

	void declareDecomposition(const Decomposition& decomposition, std::ostream& out);
	void declareItemTree(std::ostream& out, const ItemTree* itemTree, bool tableMode, unsigned int nodeId, const std::string& itemSetName, const std::string& parent = "", unsigned int level = 0);
	void induceSubinstance(std::ostream& out, const Instance& instance, const DecompositionNode::Bag& bag);
} // namespace asp_utils
