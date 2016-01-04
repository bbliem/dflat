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
#include "SolverBase.h"
#include "../../ItemTree.h"
#include "../../Decomposition.h"
#include "../../Application.h"
#include "../../Printer.h"

namespace solver { namespace asp {

SolverBase::SolverBase(const Decomposition& decomposition, const Application& app)
	: ::Solver(decomposition, app)
{
	// FIXME This should not be computed for each solver
	for(const Instance::Edge& edge : app.getInstance().getEdgeFactsOfPredicate("atom")) {
#ifndef DISABLE_CHECKS
		if(edge.size() != 1)
			throw std::runtime_error("Atom hyperedges must have arity 1");
#endif
		const String& v = edge.front();
		if(isInBag(v))
			atoms.insert(v);
	}

	for(const Instance::Edge& edge : app.getInstance().getEdgeFactsOfPredicate("rule")) {
#ifndef DISABLE_CHECKS
		if(edge.size() != 1)
			throw std::runtime_error("Rule hyperedges must have arity 1");
#endif
		const String& v = edge.front();
		if(isInBag(v)) {
			rules.insert(v);
			// Make sure that for this rule v, heads[v], positiveBody[v] and negativeBody[v] exist.
			// We require that if a rule is used in head/2, pos/2 or neg/2, then it is declared via rule/1.
			heads[v];
			positiveBody[v];
			negativeBody[v];
		}
	}

	for(const Instance::Edge& edge : app.getInstance().getEdgeFactsOfPredicate("head")) {
#ifndef DISABLE_CHECKS
		if(edge.size() != 2)
			throw std::runtime_error("Head hyperedges must have arity 2");
		const auto& declaredRules = app.getInstance().getEdgeFactsOfPredicate("rule");
		if(declaredRules.find({edge[0]}) == declaredRules.end())
			throw std::runtime_error("Rules used in head/2, pos/2 or neg/2 must be declared via rule/1");
#endif
		if(isInBag(edge[0]) && isInBag(edge[1]))
			heads[edge[0]].insert(edge[1]);
	}

	for(const Instance::Edge& edge : app.getInstance().getEdgeFactsOfPredicate("pos")) {
#ifndef DISABLE_CHECKS
		if(edge.size() != 2)
			throw std::runtime_error("Positive body hyperedges must have arity 2");
		const auto& declaredRules = app.getInstance().getEdgeFactsOfPredicate("rule");
		if(declaredRules.find({edge[0]}) == declaredRules.end())
			throw std::runtime_error("Rules used in head/2, pos/2 or neg/2 must be declared via rule/1");
#endif
		if(isInBag(edge[0]) && isInBag(edge[1]))
			positiveBody[edge[0]].insert(edge[1]);
	}

	for(const Instance::Edge& edge : app.getInstance().getEdgeFactsOfPredicate("neg")) {
#ifndef DISABLE_CHECKS
		if(edge.size() != 2)
			throw std::runtime_error("Negative body hyperedges must have arity 2");
		const auto& declaredRules = app.getInstance().getEdgeFactsOfPredicate("rule");
		if(declaredRules.find({edge[0]}) == declaredRules.end())
			throw std::runtime_error("Rules used in head/2, pos/2 or neg/2 must be declared via rule/1");
#endif
		if(isInBag(edge[0]) && isInBag(edge[1]))
			negativeBody[edge[0]].insert(edge[1]);
	}
}

ItemTreePtr SolverBase::extendRoot(const ItemTreePtr& childRoot)
{
	return ItemTreePtr(new ItemTree(ItemTree::Node(new ItemTreeNode({}, {}, {{childRoot->getNode()}}, ItemTreeNode::Type::OR))));
}

ItemTreePtr SolverBase::extendCandidate(ItemTreeNode::Items&& items, ItemTreeNode::Items&& auxItems, const ItemTreePtr& childCandidate)
{
	return ItemTreePtr(new ItemTree(ItemTree::Node(new ItemTreeNode(std::move(items), std::move(auxItems), {{childCandidate->getNode()}}, ItemTreeNode::Type::AND))));
}

ItemTreePtr SolverBase::extendCertificate(ItemTreeNode::Items&& items, ItemTreeNode::Items&& auxItems, const ItemTreePtr& childCertificate, ItemTreeNode::Type type)
{
	return ItemTreePtr(new ItemTree(ItemTree::Node(new ItemTreeNode(std::move(items), std::move(auxItems), {{childCertificate->getNode()}}, type))));
}

inline bool SolverBase::isInBag(const String& element) const
{
	return decomposition.getNode().getBag().find(element) != decomposition.getNode().getBag().end();
}

}} // namespace solver::asp
