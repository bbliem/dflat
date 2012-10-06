// Component.cpp: implementation of the Component class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>

#include <Component.hpp>
#include <Globals.hpp>

namespace sharp
{

	//////////////////////////////////////////////////////////////////////
	// Construction/Destruction
	//////////////////////////////////////////////////////////////////////
	
	
	Component::Component(int ID, int name, int iSize, int iNbrOfNeighbours)
	{
		int i;
		iMyID = ID;
		this->name = name;
	  
		MyComponents.reserve(iSize);
		for(i=0; i < iSize; i++)
			MyComponents[i] = NULL;
		iMyMaxSize = iSize;
	
		MyNeighbours.reserve(iNbrOfNeighbours);
		for(i=0; i < iNbrOfNeighbours; i++)
			MyNeighbours[i] = NULL;
		iMyMaxNbrOfNeighbours = iNbrOfNeighbours;
	
		iMyLabel = 0;
		bMySpecial = false;
	}
	
	Component::Component(int ID, int name)
	{
		iMyID = ID;
		this->name = name;
	
		iMyMaxSize = iMyMaxNbrOfNeighbours = 0;
	}
	
	Component::~Component()
	{
	}
	
	
	
	//////////////////////////////////////////////////////////////////////
	// Class methods
	//////////////////////////////////////////////////////////////////////
	
	
	// Returns the ID of the component
	int Component::getID()
	{
		return iMyID;
	}
	
	
	// Returns the name of the component
	int Component::getName()
	{
		return name;
	}
	
	
	// Returns the number of contained components
	int Component::size()
	{
		return iMyMaxSize;
	}
	
	
	// Returns the number of neighbours
	int Component::getNbrOfNeighbours()
	{
		return iMyMaxNbrOfNeighbours;
	}
	
	
	/*
	***Description***
	The method updates the neighbourhood relation, i.e., it resets the neighbours of the actual
	component.
	
	INPUT:
	OUTPUT: 
	
	***History***
	Written: (06.05.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Component::updateNeighbourhood()
	{
		int i, j;
		set<Component *> Neighbours;
		set<Component *>::iterator SetIter;
	
		// Compute new neighbours
		for(i=0; i < iMyMaxSize; i++)
			for(j=0; j < MyComponents[i]->iMyMaxSize; j++)
				if(MyComponents[i]->get(j) != this)
					Neighbours.insert(MyComponents[i]->get(j));
	
		// Allocate memory for pointers to new neighbours
		MyNeighbours.clear();
		iMyMaxNbrOfNeighbours = Neighbours.size();
		MyNeighbours.reserve(iMyMaxNbrOfNeighbours);
	
		// Set new neighbours
		for(i=0, SetIter=Neighbours.begin(); SetIter != Neighbours.end(); i++, SetIter++)
			MyNeighbours[i] = *SetIter;
	}
	
	
	/*
	***Description***
	The method updates the size of the component, i.e., the size is reduced to the number of pointers
	found in MyComponents until the first NULL pointer is found.
	
	INPUT:
	OUTPUT: 
	
	***History***
	Written: (09.05.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Component::updateSize()
	{
		int i;
	
		for(i=0; (i < iMyMaxSize) && (MyComponents[i] != NULL); i++);
		iMyMaxSize = i;
	}
	
	
	/*
	***Description***
	The method inserts the pointer Comp at position iPos in MyComponents.
	
	INPUT:	Comp: Pointer to a component
			iPos: Position at which the component has to be inserted
	OUTPUT: 
	
	***History***
	Written: (19.11.04, AD, MS, NM)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Component::ins(Component *Comp, int iPos)
	{
		if(iPos < iMyMaxSize)
			MyComponents[iPos] = Comp;
		else
			writeErrorMsg("Position not available.", "Component::ins");
	}
	
	void Component::ins(Component *Comp)
	{
		MyComponents.push_back(Comp);
		++iMyMaxSize;
	}
	
	
	/*
	***Description***
	The method removes the pointer Comp in MyComponents.
	
	INPUT:	Comp: Pointer to a component
	OUTPUT: 
	
	***History***
	Written: (08.03.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	bool Component::rem(Component *Comp)
	{
		int i;
		bool bFound = false;
	
		for(i=0; i < iMyMaxSize; i++)
			if(MyComponents[i] == Comp) {
				bFound = true;
				break;
			}
	
		if(bFound) {
			--iMyMaxSize;
			for(; i < iMyMaxSize; i++)
				MyComponents[i] = MyComponents[i+1];
			MyComponents[iMyMaxSize] = NULL;
		}
	
		return bFound;
	}
	
	
	/*
	***Description***
	The method returns the pointer at position iPos in MyComponents.
	
	INPUT:	iPos: Position in the array of pointers to contained components
	OUTPUT: return: Contained component at position iPos
	 
	***History***
	Written: (19.11.04, AD, MS, NM)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	Component *Component::get(int iPos)
	{
		if(iPos < iMyMaxSize)
			return MyComponents[iPos];
		else {
			writeErrorMsg("Position not available.", "Component::get");
			return NULL;
		}
	}
	
	
	/*
	***Description***
	The method inserts the pointer Neighbour at position iPos in MyNeighbours.
	
	INPUT: 	Neighbour: Pointer to a neighbour component
			iPos: Position in the array of pointers to neighbours
	OUTPUT: 
	
	***History***
	Written: (23.11.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	void Component::insNeighbour(Component *Neighbour, int iPos)
	{
		if(iPos < iMyMaxNbrOfNeighbours)
			MyNeighbours[iPos] = Neighbour;
		else
			writeErrorMsg("Position not available.", "Component::insNeighbour");
	}
	
	
	/*
	***Description***
	The method removes the pointer Neighbour in MyNeighbours.
	
	INPUT:	Neighbour: Pointer to a neighbour component
	OUTPUT: 
	
	***History***
	Written: (08.03.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	bool Component::remNeighbour(Component *Neighbour)
	{
		int i;
		bool bFound = false;
	
		for(i=0; i < iMyMaxNbrOfNeighbours; i++)
			if(MyNeighbours[i] == Neighbour) {
				bFound = true;
				break;
			}
	
		if(bFound) {
			--iMyMaxNbrOfNeighbours;
			for(; i < iMyMaxNbrOfNeighbours; i++)
				MyNeighbours[i] = MyNeighbours[i+1];
			MyNeighbours[iMyMaxNbrOfNeighbours] = NULL;
		}
	
		return bFound;
	}
	
	
	/*
	***Description***
	The method returns the pointer at position iPos in MyNeighbours.
	
	INPUT:	iPos: Position in the array of pointers to neighbours 
	OUTPUT:	return: Pointer to neighbour component at position iPos
	
	***History***
	Written: (23.11.04, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	Component *Component::getNeighbour(int iPos)
	{
		if(iPos < iMyMaxNbrOfNeighbours)
			return MyNeighbours[iPos];
		else {
			writeErrorMsg("Position not available.", "Component::getNeighbour");
			return NULL;
		}
	}
	
	
	// Sets label to iLabel
	void Component::setLabel(int iLabel)
	{
		iMyLabel = iLabel;
	}
	
	
	// Increments the label by iInc
	void Component::incLabel(int iInc)
	{
		iMyLabel += iInc;
	}
	
	
	// Decrements the label by iDec
	void Component::decLabel(int iDec)
	{
		iMyLabel -= iDec;
	}
	
	
	// Returns the actual label
	int Component::getLabel()
	{
		return iMyLabel;
	}
	
	
	// Returns the special flag
	bool Component::isSpecial()
	{
		return bMySpecial;
	}
	
	
	/*
	***Description***
	The method creates a clone of the component, i.e., a component with the same ID, the same name,
	the same size, and the same number of neighbours.
	
	INPUT:
	OUTPUT:	return: Pointer to the clone
	
	***History***
	Written: (03.05.05, MS)
	------------------------------------------------------------------------------------------------
	Changed: (date, author)
	Comments:
	------------------------------------------------------------------------------------------------
	*/
	
	Component *Component::clone()
	{
		int i;
		Component *Clone;
		list<int>::iterator ListIter;
	
		// Create clone
		Clone = new Component(iMyID, name, iMyMaxSize, 0);
		if(Clone == NULL)
			writeErrorMsg("Error assigning memory.", "Component::clone");
	
		// Initialize pointers
		for(i=0; i < iMyMaxSize; i++)
			Clone->MyComponents[i] = NULL;
	
		// Copy covered component IDs
		for(ListIter = MyCoveredCompIDs.begin(); ListIter != MyCoveredCompIDs.end(); ListIter++)
			Clone->MyCoveredCompIDs.push_back(*ListIter);
	
		return Clone;
	}
	
	
	// Inserts a component ID into MyCoveredCompIDs
	void Component::insCoveredID(int iID)
	{
		MyCoveredCompIDs.push_back(iID);
	}
	
	
	// Returns a pointer to MyCoveredCompIDs
	list<int> *Component::getCoveredIDs()
	{
		return &MyCoveredCompIDs;
	}

} // namespace sharp
