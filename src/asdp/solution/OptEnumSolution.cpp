#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "OptEnumSolution.h"

namespace asdp { namespace solution {

OptEnumSolution::OptEnumSolution()
{
}

OptEnumSolution* OptEnumSolution::leaf(const Tuple::Assignment& leafSolution, unsigned cost)
{
	OptEnumSolution* s = new OptEnumSolution;
	s->cost = cost;
	s->assignments.insert(leafSolution);
	return s;
}

OptEnumSolution* OptEnumSolution::extend(OptEnumSolution* base, const Tuple::Assignment& extension, unsigned cost)
{
	OptEnumSolution* s = new OptEnumSolution;
	foreach(const Tuple::Assignment& baseAssignment, base->assignments) {
		Tuple::Assignment newAssignment = extension;
		newAssignment.insert(baseAssignment.begin(), baseAssignment.end());
		s->assignments.insert(newAssignment);
	}
	s->cost = cost;
	delete base;
	return s;
}

OptEnumSolution* OptEnumSolution::unify(OptEnumSolution* left, OptEnumSolution* right, unsigned cost)
{
	left->assignments.insert(right->assignments.begin(), right->assignments.end());
	left->cost = cost;
	delete right;
	return left;
}

OptEnumSolution* OptEnumSolution::join(OptEnumSolution* left, OptEnumSolution* right, unsigned cost)
{
	OptEnumSolution* s = new OptEnumSolution;
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

unsigned OptEnumSolution::getCost() const
{
	return cost;
}

const std::set<Tuple::Assignment>& OptEnumSolution::getSolutions() const
{
	return assignments;
}

}} // namespace asdp::solution
