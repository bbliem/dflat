#pragma once

#include <sharp/main>
#include <boost/container/map.hpp>

#include "../Tuple.h"

namespace asdp {

struct Tuple : public ::Tuple
{
	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;

	virtual bool matches(const ::Tuple& other) const;
	virtual Tuple* join(const ::Tuple& other) const;
	virtual void declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const char* predicateSuffix = "") const;

#ifdef VERBOSE
	virtual void print(std::ostream&, const class sharp::Problem&) const;
#endif

	// Each tuple has an assignment of values to vertex names
	// Each assignment has a set of subordinate assignments
	typedef std::map<std::string, std::string> Assignment;

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
};

} // namespace asdp
