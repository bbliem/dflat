#pragma once

#include <cassert>
#include <sharp/main>

struct Tuple : public sharp::Tuple
{
	//! Must be asymmetric
	virtual bool operator<(const sharp::Tuple&) const = 0;
	virtual bool operator==(const sharp::Tuple&) const = 0;

	virtual int hash() const { assert(false); return 0; } // TODO

	//! Only called by the default join algorithm
	//! @return true iff this tuple is a join partner of the other
	virtual bool matches(const Tuple& other) const = 0;

	//! Only called by the default join algorithm
	//! @return a new tuple resulting from this joining with the other
	virtual Tuple* join(const Tuple& other) const = 0;

	//! Declares this tuple in ASP.
	//! @param tupleAndSolution reference to the entry in the TupleSet that contains this tuple
	//! @param predicateSuffix string that will be printed between "childTuple" and "("
	virtual void declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const char* predicateSuffix = "") const = 0;

#ifdef VERBOSE
	virtual void print(std::ostream&, const sharp::Problem&) const = 0;
#endif
};
