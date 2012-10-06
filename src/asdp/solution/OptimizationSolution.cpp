#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "OptimizationSolution.h"

namespace asdp { namespace solution {

OptimizationSolution::OptimizationSolution()
{
}

OptimizationSolution* OptimizationSolution::leaf(const Tuple::Assignment& leafSolution, unsigned cost)
{
	OptimizationSolution* s = new OptimizationSolution;
	s->cost = cost;
	s->assignments.insert(leafSolution);
	return s;
}

OptimizationSolution* OptimizationSolution::extend(OptimizationSolution* base, const Tuple::Assignment& extension, unsigned cost)
{
	OptimizationSolution* s = new OptimizationSolution;
	foreach(const Tuple::Assignment& baseAssignment, base->assignments) {
		Tuple::Assignment newAssignment = extension;
		newAssignment.insert(baseAssignment.begin(), baseAssignment.end());
		s->assignments.insert(newAssignment);
	}
	s->cost = cost;
	delete base;
	return s;
}

OptimizationSolution* OptimizationSolution::unify(OptimizationSolution* left, OptimizationSolution* right, unsigned cost)
{
	left->assignments.insert(right->assignments.begin(), right->assignments.end());
	left->cost = cost;
	delete right;
	return left;
}

OptimizationSolution* OptimizationSolution::join(OptimizationSolution* left, OptimizationSolution* right, unsigned cost)
{
	OptimizationSolution* s = new OptimizationSolution;
	foreach(const Tuple::Assignment& l, left->assignments) {
		foreach(const Tuple::Assignment& r, right->assignments) {
			Tuple::Assignment j = l;
			j.insert(r.begin(), r.end());
			s->assignments.insert(j);
		}
	}
	s->cost = cost;
	delete left;
	delete right;
	return s;
}

unsigned OptimizationSolution::getCost() const
{
	return cost;
}

const std::set<Tuple::Assignment>& OptimizationSolution::getSolutions() const
{
	return assignments;
}

}} // namespace asdp::solution
