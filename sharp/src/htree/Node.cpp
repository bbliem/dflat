// Node.cpp: implementation of the Node class.
//
//////////////////////////////////////////////////////////////////////

#include <Node.hpp>

#include <set>
#include <algorithm>
#include <iostream>

using namespace std;

using namespace sharp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


Node::Node(int ID, int name, int iSize, int iNbrOfNeighbours) : Component(ID, name, iSize, iNbrOfNeighbours)
{
}

Node::Node(int ID, int name) : Component(ID, name)
{
}

Node::~Node()
{
}



//////////////////////////////////////////////////////////////////////
// Class methods
//////////////////////////////////////////////////////////////////////


// Returns the number of hyperedges containing the node
int Node::getNbrOfEdges()
{
	return Component::size();
}


// Inserts a hyperedge at position iPos
void Node::insEdge(Hyperedge *Comp, int iPos)
{
	Component::ins((Component *)Comp, iPos);
}

void Node::insEdge(Hyperedge *Comp)
{
	Component::ins((Component *)Comp);
}

// Removes a hyperedge
bool Node::remEdge(Hyperedge *Comp)
{
	return Component::rem((Component *)Comp);
}


// Returns the hyperedge contained at position iPos
Hyperedge *Node::getEdge(int iPos)
{
	return (Hyperedge *)Component::get(iPos);
}


// Inserts a neighbour node at position iPos
void Node::insNeighbour(Node *Neighbour, int iPos)
{
	Component::insNeighbour((Component *)Neighbour, iPos);
}


// Removes a neighbour node
bool Node::remNeighbour(Node *Neighbour)
{
	return Component::remNeighbour((Component *)Neighbour);
}


// Returns the neighbour node at position iPos
Node *Node::getNeighbour(int iPos)
{
	return (Node *)Component::getNeighbour(iPos);
}


// Creates a clone of the node
Node *Node::clone()
{
	return (Node *)Component::clone();
}

int Node::getFillIn()
{
	set<Component *> nbh(this->MyNeighbours.begin(), this->MyNeighbours.end());
	cout << "neihbours: " << nbh.size() << endl;

	int fillin = 0;
	for(set<Component *>::const_iterator i = nbh.begin(); i != nbh.end(); ++i)
	{
		set<Component *> nnbh((*i)->MyNeighbours.begin(), (*i)->MyNeighbours.end());
		vector<Component *> diff;
		vector<Component *>::iterator it;
		it = set_difference(nbh.begin(), nbh.end(), nnbh.begin(), nnbh.end(), diff.begin());
		cout << nnbh.size() << ";" << int(it-diff.begin())-1 << ",";
	
		fillin += int(it - diff.begin()) - 1;
	}
	cout << endl;

	return fillin/2;
}
