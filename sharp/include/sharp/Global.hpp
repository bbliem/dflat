#ifndef GLOBAL_H_
#define GLOBAL_H_

#include <set>
#include <map>
#include <list>
#include <vector>
#include <string>

#include <sharp/support.hpp>

namespace sharp
{
	typedef unsigned int Vertex;
	#define VERTEXNOTFOUND ((Vertex)0)
	
	typedef std::set<Vertex> VertexSet;
	typedef std::set<std::pair<Vertex, Vertex> > EdgeSet;
	typedef std::set<VertexSet> HyperedgeSet;
	
	typedef std::vector<std::string> NameMap;
	typedef std::map<std::string, Vertex> ReverseNameMap;

	bool containsAll(const std::set<unsigned int> &a, const std::set<unsigned int> &b);
	void printIntSet(const VertexSet &toprint);
	void printIntList(const std::list<Vertex> &toprint);
	void printIntVector(const VertexSet &toprint);
	void printBoolVector(const std::vector<bool> &toprint);
	void printBoolList(const std::list<bool> &toprint);

} // namespace sharp

#endif /*GLOBAL_H_*/
