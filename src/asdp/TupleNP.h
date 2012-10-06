#pragma once

#include <sharp/main>

#include "../Tuple.h"

namespace asdp {

class TupleNP : public ::Tuple
{
public:
	TupleNP();

	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;
	virtual void unify(const sharp::Tuple& old);

	virtual bool matches(const ::Tuple& other) const;
	virtual TupleNP* join(const ::Tuple& other) const;
	virtual void declare(std::ostream& out, const sharp::TupleTable::value_type& tupleAndSolution, const char* predicateSuffix) const;
	virtual const Assignment& getAssignment() const;
	virtual unsigned int getCurrentCost() const;
	virtual unsigned int getCost() const;

#ifdef VERBOSE
	virtual void print(std::ostream&) const;
#endif

	Assignment assignment;
	// TODO: We might distinguish tuples with cost information from those without, but OTOH the memory consumption should not be that critical
	unsigned int currentCost;
	unsigned int cost;
};

} // namespace asdp
