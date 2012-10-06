#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include "EnumerationSolution.h"

namespace asdp { namespace solution {

EnumerationSolution::EnumerationSolution()
{
}

EnumerationSolution* EnumerationSolution::leaf(const Tuple::Assignment& leafSolution)
{
	EnumerationSolution* s = new EnumerationSolution;
	s->assignments.insert(leafSolution);
	return s;
}

EnumerationSolution* EnumerationSolution::extend(EnumerationSolution* base, const Tuple::Assignment& extension)
{
	EnumerationSolution* s = new EnumerationSolution;
	foreach(const Tuple::Assignment& baseAssignment, base->assignments) {
		Tuple::Assignment newAssignment = extension;
		newAssignment.insert(baseAssignment.begin(), baseAssignment.end());
		s->assignments.insert(newAssignment);
	}
	delete base;
	return s;
}

EnumerationSolution* EnumerationSolution::unify(EnumerationSolution* left, EnumerationSolution* right)
{
	left->assignments.insert(right->assignments.begin(), right->assignments.end());
	delete right;
	return left;
}

EnumerationSolution* EnumerationSolution::join(EnumerationSolution* left, EnumerationSolution* right)
{
	EnumerationSolution* s = new EnumerationSolution;
	foreach(const Tuple::Assignment& l, left->assignments) {
		foreach(const Tuple::Assignment& r, right->assignments) {
			Tuple::Assignment j = l;
			j.insert(r.begin(), r.end());
			s->assignments.insert(j);
		}
	}
	delete left;
	delete right;
	return s;
}

const std::set<Tuple::Assignment>& EnumerationSolution::getSolutions() const
{
	return assignments;
}

}} // namespace asdp::solution
