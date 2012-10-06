#pragma once

#include <set>
#include <sharp/main>

struct Tuple : public sharp::Tuple
{
	virtual ~Tuple();
	virtual bool operator<(const sharp::Tuple&) const;
	virtual bool operator==(const sharp::Tuple&) const;
	virtual int hash() const;

	typedef std::set<sharp::Vertex> VertexSet;
	VertexSet atoms; // Atoms of the current bag that are true
	VertexSet rules; // Rules of the current bag that are true

#ifdef VERBOSE
	void print(std::ostream&, class Problem&) const; // prints names instead of vertex numbers for atoms
#endif
};
