// CompSet.cpp: implementation of the CompSet class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>

#include <CompSet.hpp>
#include <Component.hpp>
#include <Globals.hpp>

namespace sharp
{

	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	
	CompSet::CompSet(int iNbrOfComps)
	{
		// Create pointer array
		MyComponents = new Component*[iNbrOfComps];
		if(MyComponents == NULL)
			writeErrorMsg("Error assigning memory.", "CompSet::CompSet");
		iMyMaxNbrOfComps = iNbrOfComps;
	
		// Initialize pointers in the array
		iMyNbrOfComps = 0;
		for(int i=0; i < iMyMaxNbrOfComps; i++)
			MyComponents[i] = NULL;
	
		bMySpecial = false;
	}
	
	
	CompSet::~CompSet()
	{
		delete [] MyComponents;
	}
	
	
	
	//////////////////////////////////////////////////////////////////////
	// Class methods
	//////////////////////////////////////////////////////////////////////
	
	
	/*
	***Description***
	The method checks whether the pointer Comp is already contained in the set. If so, it returns
	false;  otherwise, it inserts the pointer and returns true.
	
	INPUT: 	Comp: Pointer to a component that has to be inserted
	OUTPUT: return: true if the insertion was successfull; otherwise false
	 
	***History***
	Written: (14.02.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	bool CompSet::insComp(Component *Comp)
	{
		// Check whether Comp is already contained (multiple occurrences are not allowed)
		for(int i=0; i < iMyNbrOfComps; i++)
			if(MyComponents[i] == Comp)
				return false;
	
		if(iMyNbrOfComps >= iMyMaxNbrOfComps)
			writeErrorMsg("Set size exceeded.", "CompSet::insComp");
	
		// Insert the pointer
		MyComponents[iMyNbrOfComps++] = Comp;
		return true;
	}
	
	
	/*
	***Description***
	The method removes the component at position iPos from the set (resp. array).
	
	INPUT: 	iPos: Position in the set
	OUTPUT: 
	
	***History***
	Written: (14.02.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void CompSet::remComp(int iPos)
	{
		if(iPos >= iMyNbrOfComps)
			writeErrorMsg("Position not available.", "CompSet::remEdge");
	
		// Remove pointer
		--iMyNbrOfComps;
		for(int i=iPos; i < iMyNbrOfComps; i++)
			MyComponents[i] = MyComponents[i+1];
		MyComponents[iMyNbrOfComps] = NULL;
	}
	
	
	/*
	***Description***
	The method removes the component Comp from the set (resp. array).
	
	INPUT: 	Comp: Component to be removed
	OUTPUT: 
	
	***History***
	Written: (08.03.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	bool CompSet::remComp(Component *Comp)
	{
		int i;
		bool bFound = false;
	
		for(i=0; i < iMyNbrOfComps; i++)
			if(MyComponents[i] == Comp) {
				bFound = true;
				break;
			}
	
		if(bFound) {
			--iMyNbrOfComps;
			for(; i < iMyNbrOfComps; i++)
				MyComponents[i] = MyComponents[i+1];
			MyComponents[iMyNbrOfComps] = NULL;
		}
	
		return bFound;
	}
	
	
	// Returns the number of components currently in the set
	int CompSet::size()
	{
		return iMyNbrOfComps;
	}
	
	
	// Returns true if the set is empty; otherwise false
	bool CompSet::empty()
	{
		return iMyNbrOfComps == 0;
	}
	
	
	// Returns true if Comp is contained in the set; otherwise false
	bool CompSet::find(Component *Comp)
	{
		int i;
	
		for(i=0; i < iMyNbrOfComps; i++)
			if(MyComponents[i] == Comp)
				return true;
	
		return false;
	}
	
	
	// Returns the component at position iPos
	Component *CompSet::operator[](int iPos)
	{
		if(iPos >= iMyNbrOfComps)
			return NULL;
	
		return MyComponents[iPos];
	}
	
	
	// Returns the special flag
	bool CompSet::isSpecial()
	{
		return bMySpecial;
	}
	
	
	// Sorts the set according to the labels of its components
	void CompSet::sortByLabels()
	{
		int *iOrder, i;
	
		if(iMyNbrOfComps > 1) {
			iOrder = new int[iMyNbrOfComps];
			if(iOrder == NULL)
				writeErrorMsg("Error assigning memory.", "CompSet::sortByLabels");
	
			for(i=0; i < iMyNbrOfComps; i++)
				iOrder[i] = MyComponents[i]->getLabel();
			sortPointers((void **)MyComponents, iOrder, 0, iMyNbrOfComps-1);
			delete [] iOrder;
		}
	}
	
	
	/*
	***Description***
	The method checks whether the actual set is a subset of the set given as argument.
	
	INPUT: 	Set: Set of components
	OUTPUT: return: true if the actual set is a subset of Set; otherwise false
	
	***History***
	Written: (26.04.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	bool CompSet::isSubset(CompSet *Set)
	{
		int i;
	
		// Set labels of all nodes in the actual set to 0
		for(i=0; i < iMyNbrOfComps; i++)
			MyComponents[i]->setLabel(0);
		// Set labels of all nodes in Set to 1
		for(i=0; i < Set->iMyNbrOfComps; i++)
			Set->MyComponents[i]->setLabel(1);
	
		// Check whether all node labels in the actual set are 1; if so, the actual set is a subset of Set
		for(i=0; i < iMyNbrOfComps; i++)
			if(MyComponents[i]->getLabel() == 0)
				return false;
	
		return true;
	}

} // namespace sharp
