#pragma once

#include <cassert>
#include <sharp/main>

struct Tuple : public sharp::Tuple
{
	//! Must be asymmetric
	virtual bool operator<(const sharp::Tuple&) const = 0;
	virtual bool operator==(const sharp::Tuple&) const = 0;

	virtual int hash() const { assert(false); return 0; } // TODO

	//! @return true iff this tuple is a join partner of the other
	virtual bool matches(const Tuple& other) const = 0;

	//! @return a new tuple resulting from this joining with the other
	virtual Tuple* join(const Tuple& other) const = 0;

	//! Declares this tuple in ASP.
	//! @param tupleAndSolution reference to the entry in the TupleSet that contains this tuple
	virtual void declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution, const sharp::VertexSet& currentVertices) const = 0;

	// TODO: We might be able to dispense with this method if we use an empty root and let the exchange program filter out invalid tuples
	//! @return true iff this tuple (which is supposed to be in root) corresponds to a valid solution
	virtual bool isValid(const sharp::Problem&, const sharp::ExtendedHypertree& root) const = 0;

#ifdef VERBOSE
	virtual void print(std::ostream&, const sharp::Problem&) const = 0;
#endif
};
