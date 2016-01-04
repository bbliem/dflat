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
#include "SolverFactory.h"
#include "LeafSolver.h"
#include "AtomIntroductionSolver.h"
#include "RuleIntroductionSolver.h"
#include "AtomRemovalSolver.h"
#include "RuleRemovalSolver.h"
#include "../default_join/Solver.h"
#include "../../Decomposition.h"
#include "../../Application.h"

namespace solver { namespace asp {

SolverFactory::SolverFactory(Application& app, bool newDefault)
	: ::SolverFactory(app, "asp", "Answer Set Programming solver", newDefault)
{
}

std::unique_ptr<::Solver> SolverFactory::newSolver(const Decomposition& decomposition) const
{
	if(decomposition.isJoinNode())
		return std::unique_ptr<::Solver>(new default_join::Solver(decomposition, app, false));

	// Presuppose normalization
	else if(decomposition.getChildren().size() > 1)
		throw std::runtime_error("ASP solver requires normalization");

	// Empty leaves
	else if(decomposition.getChildren().empty())
		return std::unique_ptr<::Solver>(new LeafSolver(decomposition, app));

	// Presuppose empty root
	else if(decomposition.isRoot() && decomposition.getNode().getBag().empty() == false)
			throw std::runtime_error("ASP solver requires empty root");

	// Exchange node
	else {
		assert(decomposition.getChildren().size() == 1);
		Decomposition& childNode = **decomposition.getChildren().begin();
		const DecompositionNode::Bag& bag = decomposition.getNode().getBag();
		const DecompositionNode::Bag& childBag = childNode.getNode().getBag();

		DecompositionNode::Bag bagDifference;
		std::set_symmetric_difference(bag.begin(), bag.end(), childBag.begin(), childBag.end(), std::inserter(bagDifference, bagDifference.begin()));
		if(bagDifference.size() != 1)
			throw std::runtime_error("ASP solver requires normalization");
		const String differentElement = *bagDifference.begin();

		if(bag.size() > childBag.size()) {
			// Introduction node
			if(isAtom(differentElement))
				return std::unique_ptr<::Solver>(new AtomIntroductionSolver(decomposition, app, differentElement));
			else if(isRule(differentElement))
				return std::unique_ptr<::Solver>(new RuleIntroductionSolver(decomposition, app, differentElement));
			else
				throw std::runtime_error("Introduced bag element is neither atom nor rule");
		}
		else {
			// Removal node
			if(isAtom(differentElement))
				return std::unique_ptr<::Solver>(new AtomRemovalSolver(decomposition, app, differentElement));
			else if(isRule(differentElement))
				return std::unique_ptr<::Solver>(new RuleRemovalSolver(decomposition, app, differentElement));
			else
				throw std::runtime_error("Removed bag element is neither atom nor rule");
		}
	}
}

bool SolverFactory::isAtom(String v) const
{
	// FIXME inefficient
	for(const Instance::Edge& edge : app.getInstance().getEdgeFactsOfPredicate("atom")) {
		if(edge.size() != 1)
			throw std::runtime_error("Atom hyperedges must have arity 1");
		if(edge.front() == v)
			return true;
	}
	return false;
}

bool SolverFactory::isRule(String v) const
{
	// FIXME inefficient
	for(const Instance::Edge& edge : app.getInstance().getEdgeFactsOfPredicate("rule")) {
		if(edge.size() != 1)
			throw std::runtime_error("Rule hyperedges must have arity 1");
		if(edge.front() == v)
			return true;
	}
	return false;
}

}} // namespace solver::asp
