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
#include <unordered_set>
#include <unordered_map>

#include "../../Solver.h"
#include "../../Instance.h"
#include "../../ItemTree.h"

namespace solver { namespace asp {

class SolverBase : public ::Solver
{
public:
	SolverBase(const Decomposition& decomposition, const Application& app);

protected:
	// Return new item tree node extending childRoot
	static ItemTreePtr extendRoot(const ItemTreePtr& childRoot);

	// Return new item tree node extending childCandidate
	static ItemTreePtr extendCandidate(ItemTreeNode::Items&& items, ItemTreeNode::Items&& auxItems, const ItemTreePtr& childCandidate);

	// Return new item tree node extending childCertificate
	static ItemTreePtr extendCertificate(ItemTreeNode::Items&& items, ItemTreeNode::Items&& auxItems, const ItemTreePtr& childCertificate, ItemTreeNode::Type type = ItemTreeNode::Type::UNDEFINED);

	typedef std::set<String> Atoms;
	typedef std::set<String> Rules;
	typedef std::map<String, Atoms> AtomsInRule;

	// Part of the logic program concerning only the bag contents
	Atoms atoms;
	Rules rules;
	AtomsInRule heads;
	AtomsInRule positiveBody;
	AtomsInRule negativeBody;

	bool isInBag(const String& element) const;
};

}} // namespace solver::asp
