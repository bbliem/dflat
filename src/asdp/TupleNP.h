#pragma once

#include <sharp/main>

#include "../Tuple.h"

namespace asdp {

struct TupleNP : public ::Tuple
{
	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;

	virtual bool matches(const ::Tuple& other) const;
	virtual TupleNP* join(const ::Tuple& other) const;
	virtual void declare(std::ostream& out, const sharp::TupleSet::value_type& tupleAndSolution) const;
	virtual bool isValid(const sharp::Problem&, const sharp::ExtendedHypertree& root) const;

#ifdef VERBOSE
	virtual void print(std::ostream&, const class sharp::Problem&) const;
#endif

	typedef std::map<std::string, std::string> Assignments; // Assigns values to vertex names
	Assignments assignments;
};

} // namespace asdp
