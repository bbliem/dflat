/*
***Description***
Class Component models a component of a hypergraph, i.e., a hyperedge or a node.
*/
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_COMPONENT)
#define CLS_COMPONENT


#include <list>
#include <vector>
using namespace std;

namespace sharp
{

	class Component  
	{
	protected:
		// Unique ID of the component
		int iMyID;
	
		// Name of the component
		int name;
	
		// Size of the component
		int iMyMaxSize;
	
		// Maximum number of neighbours
		int iMyMaxNbrOfNeighbours;
	
		// Array of pointers to contained components
		vector<Component *> MyComponents;
	
	public:
		// Array of pointers to components that contain common components
		vector<Component *> MyNeighbours;
	
	protected:
		// List of component IDs that are covered by the actual component
		list<int> MyCoveredCompIDs;
	
		// Label of the component (useful for miscellaneous computations)
		int iMyLabel;
	
		// Special flag
		bool bMySpecial;
	
		// Returns the number of contained components
		int size();
	
		// Inserts a component at position iPos
		void ins(Component *Comp, int iPos);
		void ins(Component *Comp);
	
		// Removes a component
		bool rem(Component *Comp);
	
		// Returns the component contained at position iPos
		Component *get(int iPos);
	
		// Inserts a neighbour of the component at position iPos
		void insNeighbour(Component *Neighbour, int iPos);
	
		// Removes a neighbour of the component
		bool remNeighbour(Component *Neighbour);
	
		// Returns the neighbour component at position iPos
		Component *getNeighbour(int iPos);
	
	public:
		// Constructor
		Component(int ID, int name, int iSize, int iNbrOfNeighbours);
		Component(int ID, int name);
	
		// Destructor
		virtual ~Component();
	
		// Returns the ID of the component
		int getID();
	
		// Returns the name of the component
		int getName();
	
		// Returns the number of neighbours
		int getNbrOfNeighbours();
	
		// Updates the neighbourhood relation
		void updateNeighbourhood();
	
		// Updates the size of the component
		void updateSize();
	
		// Sets label to iLabel
		void setLabel(int iLabel);
	
		// Increments the label by iInc
		void incLabel(int iInc = 1);
	
		// Decrements the label by iDec
		void decLabel(int iDec = 1);
	
		// Returns the actual label
		int getLabel();
	
		// Returns the special flag
		bool isSpecial();
	
		// Creates a clone of the component
		Component *clone();
	
		// Inserts a component ID into MyCoveredCompIDs
		void insCoveredID(int iID);
	
		// Returns a pointer to MyCoveredCompIDs
		list<int> *getCoveredIDs();
	};

} // namespace sharp

#endif // !defined(CLS_COMPONENT)

