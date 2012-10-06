/*
***Description***
Library of useful global functions.
*/
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_GLOBALS)
#define CLS_GLOBALS


#define REAL double

#include <set>

using namespace std;

namespace sharp
{

	class Hypergraph;
	class Component;
	class CompSet;
	class Hyperedge;
	class Node;
	
	
	// The function writes an error message to the standard error output stream
	void writeErrorMsg(const char *cMessage, const char *cLocation, bool bExitProgram = true);
	
	// The function converts an unsigned integer number into a string
	char *uitoa(unsigned int iNumber, char *cString);
	
	// The function sorts an integer array in non-decreasing order
	void sortArray(int *iArray, int iL, int iR);
	
	// The function searches in an integer array for a given value
	bool searchArray(int *iArray, int iL, int iR, int iKey);
	
	// The function sorts an array of pointers in non-decreasing order according to a given int array
	void sortPointers(void **Ptr, int *iEval, int iL, int iR);
	
	// The function sorts an array of pointers in non-decreasing order according to a given REAL array
	void sortPointers(void **Ptr, REAL *rEval, int iL, int iR);
	
	// The function checks whether Set1 is a subset of Set2
	bool isSubset(set<Node *> *Set1, set<Node *> *Set2);
	
	// The function checks whether Set1 is a subset of Set2
	bool isSubset(set<Hyperedge *> *Set1, set<Hyperedge *> *Set2);
	
	// The function returns a random integer between iLB and iUB
	int random_range(int iLB, int iUB);

}

#endif // !defined(CLS_GLOBALS)

