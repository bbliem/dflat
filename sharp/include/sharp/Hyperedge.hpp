/*
***Description***
Class Hyperedge models a hyperedge of a hypergraph.
*/
//////////////////////////////////////////////////////////////////////


#if !defined(CLS_HYPEREDGE)
#define CLS_HYPEREDGE


#include <sharp/Component.hpp>

namespace sharp
{

	class Node;
	
	class Hyperedge : public Component
	{
	public:
		// Constructor
		Hyperedge(int ID, int name, int iSize, int iNbrOfNeighbours);
		Hyperedge(int ID, int name);
	
		// Destructor
		virtual ~Hyperedge();
	
		// Returns the number of nodes contained in the hyperedge
		int getNbrOfNodes();
	
		// Inserts a node at position iPos
		void insNode(Node *Comp, int iPos);
		void insNode(Node *Comp);
	
		// Removes a node
		bool remNode(Node *Comp);
	
		// Returns the node contained at position iPos
		Node *getNode(int iPos);
	
		// Inserts a neighbour hyperedge at position iPos
		void insNeighbour(Hyperedge *Edge, int iPos);
	
		// Removes a neighbour hyperedge
		bool remNeighbour(Hyperedge *Edge);
	
		// Returns the neighbour hyperedge at position iPos
		Hyperedge *getNeighbour(int iPos);
	
		// Creates a clone of the hyperedge
		Hyperedge *clone();
	};

} // namespace sharp

#endif // !defined(CLS_HYPEREDGE)

