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
#include "IntroductionSolver.h"
#include "RemovalSolver.h"
#include "JoinSolver.h"
#include "../../Application.h"
#include "../../Decomposition.h"

namespace solver { namespace steiner {

const std::string SolverFactory::OPTION_SECTION = "Steiner Tree solver";

SolverFactory::SolverFactory(Application& app, bool newDefault)
	: ::SolverFactory(app, "steiner", "Steiner Tree solver", newDefault)
	, optNoBinarySearch ("no-binary-search", "Disable binary search in lazy default join")
	, optBbLevel        ("bb", "s",          "Use branch and bound stategy <s>")
{
	optNoBinarySearch.addCondition(selected);
	app.getOptionHandler().addOption(optNoBinarySearch, OPTION_SECTION);

	optBbLevel.addCondition(selected);
	optBbLevel.addCondition(condOptimization);
	optBbLevel.addChoice("none", "No branch and bound");
	optBbLevel.addChoice("basic", "Prevent rows not cheaper than current provisional solution");
	optBbLevel.addChoice("full", "Improve bounds using solutions for forgotten subgraphs", true);
	app.getOptionHandler().addOption(optBbLevel, OPTION_SECTION);
}

std::unique_ptr<::Solver> SolverFactory::newSolver(const Decomposition& decomposition) const
{
	LazySolver::BranchAndBoundLevel bbLevel;
	if(optBbLevel.getValue() == "none")
		bbLevel = LazySolver::BranchAndBoundLevel::none;
	else if(optBbLevel.getValue() == "basic")
		bbLevel = LazySolver::BranchAndBoundLevel::basic;
	else {
		assert(optBbLevel.getValue() == "full");
		bbLevel = LazySolver::BranchAndBoundLevel::full;
	}

	if(decomposition.isJoinNode())
		return std::unique_ptr<::Solver>(new JoinSolver(decomposition, app, bbLevel, !optNoBinarySearch.isUsed()));

	// Presuppose normalization
	else if(decomposition.getChildren().size() > 1)
		throw std::runtime_error("Steiner Tree solver requires normalization");

	// Empty leaves
	else if(decomposition.getChildren().empty()) {
		if(decomposition.getNode().getInducedInstance().getNumVertices() > 0)
			throw std::runtime_error("Steiner Tree solver requires empty leaves");
		return std::unique_ptr<::Solver>(new LeafSolver(decomposition, app, bbLevel));
	}

	// Presuppose empty root
	else if(decomposition.isRoot() && decomposition.getNode().getInducedInstance().getNumVertices() > 0)
		throw std::runtime_error("Steiner Tree solver requires empty root");

	// Exchange node
	else {
		assert(decomposition.getChildren().size() == 1);
		Decomposition& childNode = **decomposition.getChildren().begin();
		const auto& bag = decomposition.getNode().getInducedInstance().getVertexNames();
		const auto& childBag = childNode.getNode().getInducedInstance().getVertexNames();

		assert(std::is_sorted(bag.begin(), bag.end()));
		assert(std::is_sorted(childBag.begin(), childBag.end()));
		std::vector<unsigned> bagDifference;
		std::set_symmetric_difference(bag.begin(), bag.end(), childBag.begin(), childBag.end(), std::inserter(bagDifference, bagDifference.begin()));
		if(bagDifference.size() != 1)
			throw std::runtime_error("Steiner Tree solver requires normalization");
		const auto differentElement = *bagDifference.begin();

		// Introduction or removal?
		if(bag.size() > childBag.size())
			return std::unique_ptr<::Solver>(new IntroductionSolver(decomposition, app, differentElement, bbLevel));
		else
			return std::unique_ptr<::Solver>(new RemovalSolver(decomposition, app, differentElement, bbLevel));
	}
}

void SolverFactory::notify()
{
	::SolverFactory::notify();

	if(!app.isOptimizationDisabled())
		condOptimization.setSatisfied();
}

}} // namespace solver::steiner
