#include <config.h>

#include <Global.hpp>

#include <set>
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

namespace sharp
{

	bool containsAll(const set<unsigned int> &a, const set<unsigned int> &b)
	{
		if(a.size() < b.size()) return false;
	
		vector<unsigned int> temp(b.size());
		vector<unsigned int>::iterator it;
	
		it = set_intersection(a.begin(), a.end(), b.begin(), b.end(), temp.begin());
	
		if(it - temp.begin() == (int)b.size()) return true; else return false;
	}
	
	void printIntSet(const set<unsigned int> &toprint)
	{
		const char *prefix = "";
	
		for(set<unsigned int>::const_iterator it = toprint.begin(); it != toprint.end(); ++it)
		{
			cout << prefix << *it;
			prefix = ", ";
		}	
	}
	
	void printIntList(const list<unsigned int> &toprint)
	{
		const char *prefix = "";
		
		for(list<unsigned int>::const_iterator it = toprint.begin(); it != toprint.end(); ++it)
		{
			cout << prefix << *it;
			prefix = ", ";
		}
	}
	
	void printBoolList(const list<bool> &toprint)
	{
		const char *prefix = "";
		
		for(list<bool>::const_iterator it = toprint.begin(); it != toprint.end(); ++it)
		{
			cout << prefix << (*it ? "1" : "0");
			prefix = ", ";
		}
	}
	
	void printIntVector(const vector<Vertex> &toprint)
	{
	        const char *prefix = "";
	
	        for(vector<unsigned int>::const_iterator it = toprint.begin(); it != toprint.end(); ++it)
	        {
	                cout << prefix << *it;
	                prefix = ",";
	        }
	}
	
	void printBoolVector(const vector<bool> &toprint)
	{
	        for(vector<bool>::const_iterator it = toprint.begin(); it != toprint.end(); ++it)
	        {
	                cout << (*it ? "1" : "0");
	        }
	}

} // namespace sharp
