#pragma once

#include <sharp/main>
#include <boost/container/map.hpp>

#include "../Tuple.h"

namespace asdp {

class Tuple : public ::Tuple
{
public:
	Tuple();

	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;

	virtual bool matches(const ::Tuple& other) const;
	virtual Tuple* join(const ::Tuple& other) const;
	virtual void declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateSuffix = "") const;
	virtual const Assignment& getAssignment() const;
	virtual unsigned int getCurrentCost() const;
	virtual unsigned int getIntroducedCost() const;

#ifdef VERBOSE
	virtual void print(std::ostream&) const;
#endif

	// Each assignment has a set of subordinate assignments
	struct Tree
	{
		typedef boost::container::map<Assignment, Tree> Children; // The node data is stored as the keys
		Children children;
		// std::set won't work because Tree is an incomplete type at this time.
		// Cf. http://stackoverflow.com/questions/6527917/how-can-i-emulate-a-recursive-type-definition-in-c
		// Cf. http://www.boost.org/doc/libs/1_48_0/doc/html/container/containers_of_incomplete_types.html

		bool operator==(const Tree& rhs) const;
		bool operator<(const Tree& rhs) const;

		//! Adds the given path of assignments as descendants to this tree
		template <class Iterator> void addPath(Iterator begin, Iterator end)
		{
			if(begin != end) {
				Tree& child = children[*begin];
				child.addPath(++begin, end);
			}
		}
	} tree; // It is your responsibility that "tree" only has one child (viz. the actual root)

	// TODO: We might distinguish tuples with cost information from those without, but OTOH the memory consumption should not be that critical
	unsigned int currentCost;
	unsigned int introducedCost;
};

} // namespace asdp
