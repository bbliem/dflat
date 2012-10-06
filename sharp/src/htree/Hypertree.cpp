// Hypertree.cpp: implementation of the Hypertree class.
//
//////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <Hypergraph.hpp>
#include <Hyperedge.hpp>
#include <Node.hpp>
#include <CompSet.hpp>
#include <Globals.hpp>
#include <Hypertree.hpp>
#include <SetCover.hpp>

using namespace sharp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


Hypertree::Hypertree()
{
	MyParent = NULL;
	iMyLabel = 0;
}


Hypertree::~Hypertree()
{
	list<Hypertree *>::iterator ChildIter;

	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		delete *ChildIter;
}



//////////////////////////////////////////////////////////////////////
// Class methods
//////////////////////////////////////////////////////////////////////


/*
***Description***
The method labels all hyperedges covered by the chi-set of some node with one.

INPUT:	HGraph: Underlying hypergraph
OUTPUT: 

***History***
Written: (25.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::labelCovEdges(Hypergraph *HGraph)
{
	int i, j;
	bool bCovered;
	set<Node *>::iterator NodeIter;
	list<Hypertree *>::iterator ChildIter;

	// Label all nodes in the chi-set
	for(NodeIter=MyChi.begin(); NodeIter != MyChi.end(); NodeIter++)
		(*NodeIter)->setLabel(1);

	// Label all hyperedges covered by the chi-set
	for(i=0; i < HGraph->getNbrOfEdges(); i++) {
		bCovered = true;
		for(j=0; j < HGraph->getEdge(i)->getNbrOfNodes(); j++)
			if(HGraph->getEdge(i)->getNode(j)->getLabel() == 0) {
				bCovered = false;
				break;
			}
		if(bCovered)
			HGraph->getEdge(i)->setLabel(1);
	}

	// Reset node labels in the chi-set
	for(NodeIter=MyChi.begin(); NodeIter != MyChi.end(); NodeIter++)
		(*NodeIter)->setLabel(0);

	// Label hyperedges covered by the children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->labelCovEdges(HGraph);
}


/*
***Description***
The method labels all variables in a chi-sets of the subtree.

INPUT:	iLabel: Labeling value.
OUTPUT: 

***History***
Written: (25.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (08.04.05, MS)
Comments: Labeling generalized such that nodes are labeled by iLabel if they are labeled with
zero and by -1 if they are labeled with a number greater than zero and less than iLabel.
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::labelChiSets(int iLabel)
{
	set<Node *>::iterator NodeIter;
	list<Hypertree *>::iterator ChildIter;

	// Label all nodes in the chi-set
	for(NodeIter=MyChi.begin(); NodeIter != MyChi.end(); NodeIter++)
		if((*NodeIter)->getLabel() >= 0)
		{
			if(((*NodeIter)->getLabel() > 0) && ((*NodeIter)->getLabel() < iLabel))
				(*NodeIter)->setLabel(-1);
			else
				(*NodeIter)->setLabel(iLabel);
		}

	// Label all nodes in the chi-sets of the children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->labelChiSets(iLabel);
}


/*
***Description***
The method selects for each hyperedge in the lambda-labels a hypertree-node that has to cover
the hyperedge, i.e., all its nodes are contained in the corresponding chi-set. It is assumed 
that the hyperedges in the lambda-set are connected and each hyperedge is uniquely labeled with
an index in the argument array.

INPUT:	CovNodes: Array with pointers to hypertree-nodes selected for each hyperedge.
OUTPUT:

***History***
Written: (07.04.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::selCovHTNodes(Hypertree **CovNodes)
{
	set<Hyperedge *>::iterator LambdaIter;
	list<Hypertree *>::iterator ChildIter;

	// Add all nodes of hyperedges that occur the first time to the chi-set
	for(LambdaIter=MyLambda.begin(); LambdaIter != MyLambda.end(); LambdaIter++)
		if(CovNodes[(*LambdaIter)->getLabel()] == NULL)
			CovNodes[(*LambdaIter)->getLabel()] = this;
		else
			if(MyLambda.size() < CovNodes[(*LambdaIter)->getLabel()]->getLambda()->size())
				CovNodes[(*LambdaIter)->getLabel()] = this;

	// Set the chi-set for all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->selCovHTNodes(CovNodes);
}


/*
***Description***
The method sets the chi-set such that it contains all nodes that occur in some chi-sets of at 
least two subtrees or in the chi-set of at least one subtree and the chi-set of the parent.

INPUT:	HGraph: Underlying hypergraph
OUTPUT:

***History***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::setChi_Conn(Hypergraph *HGraph)
{
	int i;
	set<Node *>::iterator ChiIter;
	list<Hypertree *>::iterator ChildIter;

	HGraph->resetNodeLabels();
	// Label all nodes that occur in the chi-sets of at least two subtrees by -1
	for(i=1, ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); i++, ChildIter++)
		(*ChildIter)->labelChiSets(i);

	if(MyParent != NULL)
		// Label all nodes that occur in the chi-set of the parent and of at least one subtree by -1
		for(ChiIter=MyParent->getChi()->begin(); ChiIter != MyParent->getChi()->end(); ChiIter++)
			if((*ChiIter)->getLabel() > 0)
				(*ChiIter)->setLabel(-1);

	// Put all nodes in the actual chi-set that are labeled with -1
	for(i=0; i < HGraph->getNbrOfNodes(); i++)
		if(HGraph->getNode(i)->getLabel() == -1)
			MyChi.insert(HGraph->getNode(i));
	
	// Set the chi-sets for all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->setChi_Conn(HGraph);
}


/*
***Description***
The method checks whether the hypertree contains cycles of the form A->C, B->C, where A != B.
It is assumed that all labels of hypertree-nodes in the subtree are set to zero.

INPUT:
OUTPUT:	return: true if the hypertree contains cycles; otherwise false

***History***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

bool Hypertree::isCyclic_pvt()
{
	list<Hypertree *>::iterator ChildIter;

	if(iMyLabel != 0)
		return true;
	iMyLabel = 1;

	// Check cyclicity for all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		if((*ChildIter)->isCyclic())
			return true;

	return false;
}


/*
***Description***
The method checks the second hypertee decomposition condition. It is assumed that all nodes
of the underlying hypergraph are labeled by 0. It returns a witness node if the condition is
violated; otherwise the pointer NULL.

INPUT:
OUTPUT:	return: Witness node if the second condition is violated; otherwise NULL

***History***
Written: (25.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Node *Hypertree::checkCond2()
{
	Node *Witness;
	set<Node *>::iterator NodeIter1, NodeIter2;
	list<Hypertree *>::iterator ChildIter;

	// Check "connectedness" for each child and label forbidden nodes
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++) {
		if((Witness = (*ChildIter)->checkCond2()) != NULL)
			return Witness;
		for(NodeIter1=(*ChildIter)->MyChi.begin(); NodeIter1 != (*ChildIter)->MyChi.end(); NodeIter1++) {
			NodeIter2 = MyChi.find(*NodeIter1);
			if(NodeIter2 == MyChi.end())
				(*NodeIter1)->setLabel(1);
		}
	}

	// Check whether there occur forbidden nodes in the actual chi-set
	for(NodeIter1=MyChi.begin(); NodeIter1 != MyChi.end(); NodeIter1++)
		if((*NodeIter1)->getLabel() != 0)
			return *NodeIter1;

	return NULL;
}


/*
***Description***
The method checks the third hypertee decomposition condition. It is assumed that all nodes
of the underlying hypergraph are labeled by 0. It returns a witness hypertree-node if the
condition is violated; otherwise the pointer NULL.

INPUT:
OUTPUT:	return: Witness hypertree node if the third condition is violated; otherwise NULL

***History***
Written: (25.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Hypertree *Hypertree::checkCond3()
{
	set<Hyperedge *>::iterator EdgeIter;
	set<Node *>::iterator NodeIter;
	list<Hypertree *>::iterator ChildIter;
	Hypertree *Witness;

	// Label all variables of hyperedges in the lambda-set
	for(EdgeIter=MyLambda.begin(); EdgeIter != MyLambda.end(); EdgeIter++)
		for(int i=0; i < (*EdgeIter)->getNbrOfNodes(); i++)
			(*EdgeIter)->getNode(i)->setLabel(1);

	// Check whether there is some variable in the chi-set that is not labeled
	for(NodeIter=MyChi.begin(); NodeIter != MyChi.end(); NodeIter++)
		if((*NodeIter)->getLabel() == 0)
			return this;

	// Reset all labels of variables of hyperedges in the lambda-set
	for(EdgeIter=MyLambda.begin(); EdgeIter != MyLambda.end(); EdgeIter++)
		for(int i=0; i < (*EdgeIter)->getNbrOfNodes(); i++)
			(*EdgeIter)->getNode(i)->setLabel(0);

	// Check third condition for all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		if((Witness = (*ChildIter)->checkCond3()) != NULL)
			return Witness;

	return NULL;
}


/*
***Description***
The method reduces the number of labellings in the lambda-sets in such a way that lambda
labellings are replaced by lambda labellings of the parent node if the hypertree conditions
remain satisfied and the hypertree-width becomes smaller.

INPUT:
OUTPUT:

***History***
Written: (15.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::reduceLambdaTopDown()
{
	int iNbrOfNodes, iNbrOfHEdges, i, j;
	bool bFinalOccurrence;
	CompSet *Nodes, *HEdges, *Lambda, *CovEdges;
	set<Hyperedge *>::iterator LambdaIter;
	list<Hypertree *>::iterator ChildIter;
	SetCover SC;

	// Compute an upper bound for the memory needed in this method
	iNbrOfNodes = 0;
	for(LambdaIter=MyLambda.begin(); LambdaIter != MyLambda.end(); LambdaIter++)
		iNbrOfNodes += (*LambdaIter)->getNbrOfNodes();
	iNbrOfHEdges = MyLambda.size();
	if(MyParent != NULL)
		iNbrOfHEdges += MyParent->MyLambda.size();

	Lambda = new CompSet(MyLambda.size());
	Nodes = new CompSet(iNbrOfNodes);
	HEdges = new CompSet(iNbrOfHEdges);
	if((Lambda == NULL) || (Nodes == NULL) || (HEdges == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::reduceLambdaTopDown");

	// Reset the labels of the nodes in the actual hyperedges
	for(LambdaIter=MyLambda.begin(); LambdaIter != MyLambda.end(); LambdaIter++) {
		Lambda->insComp(*LambdaIter);
		HEdges->insComp(*LambdaIter);
		for(i=0; i < (*LambdaIter)->getNbrOfNodes(); i++)
			(*LambdaIter)->getNode(i)->setLabel(0);
	}
	MyLambda.clear();

	// Label all nodes in the actual hyperedges that occur in the hyperedges of at least two children by -1
	for(i=1, ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); i++, ChildIter++)
		for(LambdaIter=(*ChildIter)->MyLambda.begin(); LambdaIter != (*ChildIter)->MyLambda.end(); LambdaIter++)
			for(j=0; j < (*LambdaIter)->getNbrOfNodes(); j++)
				if((*LambdaIter)->getNode(j)->getLabel() >= 0) {
					if(((*LambdaIter)->getNode(j)->getLabel() > 0) && ((*LambdaIter)->getNode(j)->getLabel() < i))
						(*LambdaIter)->getNode(j)->setLabel(-1);
					else
						(*LambdaIter)->getNode(j)->setLabel(i);
				}

	if(MyParent != NULL)
		// Label all nodes in the actual hyperedges that occur in the hyperedges of the parent node and at least one child node by -1
		for(LambdaIter=MyParent->MyLambda.begin(); LambdaIter != MyParent->MyLambda.end(); LambdaIter++) {
			HEdges->insComp(*LambdaIter);
			for(i=0; i < (*LambdaIter)->getNbrOfNodes(); i++)
				if((*LambdaIter)->getNode(i)->getLabel() > 0)
					(*LambdaIter)->getNode(i)->setLabel(-1);
		}

	for(i=0; i < Lambda->size(); i++) {
		// Check whether the hyperedge does not occur in any child
		bFinalOccurrence = true;
		for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
			if((*ChildIter)->MyLambda.find((Hyperedge *)((*Lambda)[i])) != (*ChildIter)->MyLambda.end()) {
				bFinalOccurrence = false;
				break;
			}

		// If so, the hyperedge must not be removed
		if(bFinalOccurrence) {
			MyLambda.insert((Hyperedge *)((*Lambda)[i]));
			for(j=0; j < ((Hyperedge *)((*Lambda)[i]))->getNbrOfNodes(); j++)
				((Hyperedge *)((*Lambda)[i]))->getNode(j)->setLabel(0);
		}
	}

	// Select all nodes in the actual hyperedges that are marked: they must be covered
	for(i=0; i < Lambda->size(); i++) {
		for(j=0; j < ((Hyperedge *)((*Lambda)[i]))->getNbrOfNodes(); j++)
			if(((Hyperedge *)((*Lambda)[i]))->getNode(j)->getLabel() == -1)
				Nodes->insComp(((Hyperedge *)((*Lambda)[i]))->getNode(j));
	}

	// Apply set covering
	CovEdges = SC.cover(Nodes, HEdges);

	// Copy all covering hyperedges into the lambda-set
	for(i=0; i < CovEdges->size(); i++)
		MyLambda.insert((Hyperedge *)((*CovEdges)[i]));

	// If the size of the lambda-set has been increased
	if((int)MyLambda.size() >= Lambda->size()) {
		MyLambda.clear();
		for(i=0; i < Lambda->size(); i++)
			MyLambda.insert((Hyperedge *)((*Lambda)[i]));
	}

	delete Lambda;
	delete Nodes;
	delete HEdges;
	delete CovEdges;

	// Reduce the lambda-sets of all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->reduceLambdaTopDown();
}


/*
***Description***
The method reduces the number of labellings in the lambda-sets in such a way that lambda
labellings are replaced by lambda labellings of the child node if the hypertree conditions
remain satisfied and the hypertree-width becomes smaller.

INPUT:
OUTPUT:

***History***
Written: (22.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::reduceLambdaBottomUp()
{
	int iNbrOfNodes, iNbrOfHEdges, i, j;
	bool bFinalOccurrence, bChildFound;
	CompSet *Nodes, *HEdges, *Lambda, *CovEdges;
	set<Hyperedge *>::iterator LambdaIter;
	list<Hypertree *>::iterator ChildIter;
	SetCover SC;
	Hypertree *Child = this;

	resetLabels();
	while(Child != NULL) {
		do {
			bChildFound = false;
			// Check whether there are unprocessed child nodes
			for(ChildIter=Child->MyChildren.begin(); ChildIter != Child->MyChildren.end(); ChildIter++)
				if((*ChildIter)->getLabel() == 0) {
					bChildFound = true;
					Child = (*ChildIter);
					break;
				}
		} while(bChildFound);

		// Compute an upper bound for the memory needed in this method
		iNbrOfNodes = 0;
		for(LambdaIter=Child->MyLambda.begin(); LambdaIter != Child->MyLambda.end(); LambdaIter++)
			iNbrOfNodes += (*LambdaIter)->getNbrOfNodes();
		iNbrOfHEdges = Child->MyLambda.size();
		for(ChildIter=Child->MyChildren.begin(); ChildIter != Child->MyChildren.end(); ChildIter++)
			iNbrOfHEdges += (*ChildIter)->MyLambda.size();

		Lambda = new CompSet(Child->MyLambda.size());
		Nodes = new CompSet(iNbrOfNodes);
		HEdges = new CompSet(iNbrOfHEdges);
		if((Lambda == NULL) || (Nodes == NULL) || (HEdges == NULL))
			writeErrorMsg("Error assigning memory.", "Hypertree::reduceLambdaBottomUp");

		// Reset the labels of the nodes in the actual hyperedges
		for(LambdaIter=Child->MyLambda.begin(); LambdaIter != Child->MyLambda.end(); LambdaIter++) {
			Lambda->insComp(*LambdaIter);
			HEdges->insComp(*LambdaIter);
			for(i=0; i < (*LambdaIter)->getNbrOfNodes(); i++)
				(*LambdaIter)->getNode(i)->setLabel(0);
		}
		Child->MyLambda.clear();

		// Label all nodes in the actual hyperedges that occur in the hyperedges of at least two children by -1
		for(i=1, ChildIter=Child->MyChildren.begin(); ChildIter != Child->MyChildren.end(); i++, ChildIter++)
			for(LambdaIter=(*ChildIter)->MyLambda.begin(); LambdaIter != (*ChildIter)->MyLambda.end(); LambdaIter++) {
				HEdges->insComp(*LambdaIter);
				for(j=0; j < (*LambdaIter)->getNbrOfNodes(); j++)
					if((*LambdaIter)->getNode(j)->getLabel() >= 0) {
						if(((*LambdaIter)->getNode(j)->getLabel() > 0) && ((*LambdaIter)->getNode(j)->getLabel() < i))
							(*LambdaIter)->getNode(j)->setLabel(-1);
						else
							(*LambdaIter)->getNode(j)->setLabel(i);
					}
			}

		if(Child->MyParent != NULL)
			// Label all nodes in the actual hyperedges that occur in the hyperedges of the parent node and at least one child node by -1
			for(LambdaIter=Child->MyParent->MyLambda.begin(); LambdaIter != Child->MyParent->MyLambda.end(); LambdaIter++) {
				for(i=0; i < (*LambdaIter)->getNbrOfNodes(); i++)
					if((*LambdaIter)->getNode(i)->getLabel() > 0)
						(*LambdaIter)->getNode(i)->setLabel(-1);
			}

		for(i=0; i < Lambda->size(); i++) {
			// Check whether the hyperedge does not occur in the parent
			bFinalOccurrence = true;
			if((Child->MyParent != NULL) && (Child->MyParent->MyLambda.find((Hyperedge *)((*Lambda)[i])) != Child->MyParent->MyLambda.end()))
				bFinalOccurrence = false;

			// If so, the hyperedge must not be removed
			if(bFinalOccurrence) {
				Child->MyLambda.insert((Hyperedge *)((*Lambda)[i]));
				for(j=0; j < ((Hyperedge *)((*Lambda)[i]))->getNbrOfNodes(); j++)
					((Hyperedge *)((*Lambda)[i]))->getNode(j)->setLabel(0);
			}
		}

		// Select all nodes in the actual hyperedges that are marked: they must be covered
		for(i=0; i < Lambda->size(); i++)
			for(j=0; j < ((Hyperedge *)((*Lambda)[i]))->getNbrOfNodes(); j++)
				if(((Hyperedge *)((*Lambda)[i]))->getNode(j)->getLabel() == -1)
					Nodes->insComp(((Hyperedge *)((*Lambda)[i]))->getNode(j));

		// Apply set covering
		CovEdges = SC.cover(Nodes, HEdges);

		// Copy all covering hyperedges into the lambda-set
		for(i=0; i < CovEdges->size(); i++)
			Child->MyLambda.insert((Hyperedge *)((*CovEdges)[i]));

		// If the size of the lambda-set has been increased
		if((int)Child->MyLambda.size() >= Lambda->size()) {
			Child->MyLambda.clear();
			for(i=0; i < Lambda->size(); i++)
				Child->MyLambda.insert((Hyperedge *)((*Lambda)[i]));
		}

		delete Lambda;
		delete Nodes;
		delete HEdges;
		delete CovEdges;

		// Label the node and go up to the parent node
		Child->iMyLabel = 1;
		Child = Child->MyParent;
	}
}


/*
***Description***
The method writes hypertree nodes into a GML file.

INPUT:	GMLFile: Output file stream
OUTPUT:

***History***
Written: (03.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::writeGMLNodes(ofstream &GMLFile)
{
	int i, *iOrder;
	set<Hyperedge *>::iterator LambdaIter;
	set<Node *>::iterator ChiIter;
	list<Hypertree *>::iterator ChildIter;
	Node **Chi;
	Hyperedge **Lambda;

	Lambda = new Hyperedge*[MyLambda.size()];
	iOrder = new int[MyLambda.size()];
	if((Lambda == NULL) || (iOrder == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::writeGMLNodes");
	// Copy the lambda-set into an array
	for(i=0, LambdaIter = MyLambda.begin(); LambdaIter != MyLambda.end(); i++, LambdaIter++) {
		Lambda[i] = *LambdaIter;
		iOrder[i] = (*LambdaIter)->getLabel();
	}
	sortPointers((void **)Lambda, iOrder, 0, MyLambda.size()-1);

	GMLFile << "  node [" << endl;
	GMLFile << "    id " << iMyLabel << endl;
	GMLFile << "    label \"{";

	// Write lambda-set
	if(MyLambda.size() > 0)
		GMLFile << Lambda[0]->getName();
	for(i=1; i < (int)MyLambda.size(); i++)
		GMLFile << ", " << Lambda[i]->getName();

	delete [] Lambda;
	delete [] iOrder;

	GMLFile << "}    {";

	Chi = new Node*[MyChi.size()];
	iOrder = new int[MyChi.size()];
	if((Chi == NULL) || (iOrder == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::writeGMLNodes");
	// Copy the chi-set into an array
	for(i=0, ChiIter = MyChi.begin(); ChiIter != MyChi.end(); i++, ChiIter++) {
		Chi[i] = *ChiIter;
		iOrder[i] = (*ChiIter)->getLabel();
	}
	sortPointers((void **)Chi, iOrder, 0, MyChi.size()-1);

	// Write chi-set
	if(MyChi.size() > 0)
		GMLFile << Chi[0]->getName();
	for(i=1; i < (int)MyChi.size(); i++)
		GMLFile << ", " << Chi[i]->getName();

	delete [] Chi;
	delete [] iOrder;
	
	GMLFile << "}\"" << endl;
	GMLFile << "    vgj [" << endl;
	GMLFile << "      labelPosition \"in\"" << endl;
	GMLFile << "      shape \"Rectangle\"" << endl;
	GMLFile << "    ]" << endl;
	GMLFile << "  ]" << endl << endl;

	// Write GML nodes for all subtrees
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->writeGMLNodes(GMLFile);
}


/*
***Description***
The method writes hypertree edges into a GML file.

INPUT:	GMLFile: Output file stream
OUTPUT:

***History***
Written: (03.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::writeGMLEdges(ofstream &GMLFile)
{
	set<Hyperedge *>::iterator LambdaIter;
	set<Node *>::iterator ChiIter;
	list<Hypertree *>::iterator ChildIter;

	if(MyParent != NULL) {
		GMLFile << "  edge [" << endl;
		GMLFile << "    source " << MyParent->getLabel() << endl;
		GMLFile << "    target " << iMyLabel << endl;
		GMLFile << "  ]" << endl << endl;
	}

	// Write GML edges for all subtrees
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->writeGMLEdges(GMLFile);
}


/*
***Description***
The method writes the hypertree into a GML format file.

INPUT:	cNameOfFile: Name of the file to which the GML output has to be written
OUTPUT: 

***History***
Written: (03.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::outputToGML(Hypergraph *HGraph, char *cNameOfFile)
{
	int i;
	ofstream GMLFile;
	
	// Set labels to write the output in a uniform order
	setIDLabels();
	for(i=0; i < HGraph->getNbrOfNodes(); i++)
		HGraph->getNode(i)->setLabel(i);
	for(i=0; i < HGraph->getNbrOfEdges(); i++)
		HGraph->getEdge(i)->setLabel(i);

	GMLFile.open(cNameOfFile, ios::out);

	// Check if file opening was successful
	if(!GMLFile.is_open())
		writeErrorMsg("Error opening file.", "Hypertree::outputToGML");

	GMLFile << "graph [" << endl << endl;
	GMLFile << "  directed 0" << endl << endl;

	writeGMLNodes(GMLFile);  // Write hypertree nodes in GML format
	writeGMLEdges(GMLFile);  // Write hypertree edges in GML format

	GMLFile << "]" << endl;

	GMLFile.close();
}


/*
***Description***
The method writes the hypertree into a text format file.

INPUT:	cNameOfFile: Name of the file to which the text output has to be written
OUTPUT: 

***History***
Written: (date, author)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::outputToText(char *cNameOfFile)
{
	writeErrorMsg("Not implemented yet.", "Hypertree::outputToText");
}


// The method inserts a node into the chi-set.
void Hypertree::insChi(Node *Node)
{
	MyChi.insert(Node);
}


// The method inserts a hyperedge into the lambda-set.
void Hypertree::insLambda(Hyperedge *Edge)
{
	MyLambda.insert(Edge);
}


// The method sets the pointer to the parent node.
void Hypertree::setParent(Hypertree *Parent)
{
	MyParent = Parent;
}

Hypertree *Hypertree::getParent()
{
	return this->MyParent;
}

const Hypertree *Hypertree::getParent() const
{
	return this->MyParent;
}

list<Hypertree *> *Hypertree::getChildren()
{
	return &this->MyChildren;
}

const list<Hypertree *> *Hypertree::getChildren() const
{
	return &this->MyChildren;
}

// The method inserts a child node.
void Hypertree::insChild(Hypertree *Child, bool bSetParent)
{
	MyChildren.push_back(Child);
	if(bSetParent)
		Child->setParent(this);
}


/*
***Description***
The method removes a child from the current node. It returns true if the child
was found; otherwise false.

INPUT:	Child: Child node in the hypertree
OUTPUT:	return: true if the child was found; otherwise false

***History***
Written: (13.01.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

bool Hypertree::remChild(Hypertree *Child)
{
	list<Hypertree *>::iterator ChildIter;

	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		if((*ChildIter) == Child)
			break;
	if(ChildIter != MyChildren.end()) {
		// If the child was found, remove it
		MyChildren.erase(ChildIter);
		return true;
	}

	return false;
}


/*
***Description***
The method removes all children from the current node.

INPUT:
OUTPUT:

***History***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::remChildren()
{
	list<Hypertree *>::iterator ChildIter;

	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->setParent(NULL);
	MyChildren.clear();
}


// Inserts a pointer into the pointer set;
// such pointers are sometimes useful for the construction of a hypertree
void Hypertree::insPointer(void *Ptr)
{
	MyPointers.insert(Ptr);
}


// Inserts an ID into the ID set;
// such IDs are sometimes useful for the construction of a hypertree
void Hypertree::insID(int iID)
{
	MyIDs.insert(iID);
}


/*
***Description***
The method searches for a tree node that contains Ptr in its pointer set. It returns this node
if it is found; otherwise it returns NULL.

INPUT:	Ptr: Pointer that has to be searched in the subtree
OUTPUT:	return: Pointer to the hypertree node containing Ptr; NULL if such a node was not found

***History***
Written: (13.01.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Hypertree *Hypertree::getHTNode(void *Ptr)
{
	Hypertree *HTree = NULL;
	set<void *>::iterator SetIter;
	list<Hypertree *>::iterator ChildIter;

	// Search for Ptr in the actual node
	SetIter = MyPointers.find(Ptr);
	if(SetIter != MyPointers.end())
		return this;  // Ptr was found in the actual node

	// Search for Ptr in the subtrees rooted at the child nodes
	for(ChildIter = MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		if((HTree = (*ChildIter)->getHTNode(Ptr)) != NULL)
			return HTree;

	return NULL;
}


/*
***Description***
The method searches for a tree node that contains iID in its ID set. It returns this node
if it is found; otherwise it returns NULL.

INPUT:	iID: ID that has to be searched in the subtree
OUTPUT:	return: Pointer to the hypertree node containing iID; NULL if such a node was not found

***History***
Written: (09.05.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Hypertree *Hypertree::getHTNode(int iID)
{
	Hypertree *HTree = NULL;
	set<int>::iterator SetIter;
	list<Hypertree *>::iterator ChildIter;

	// Search for Ptr in the actual node
	SetIter = MyIDs.find(iID);
	if(SetIter != MyIDs.end())
		return this;  // Ptr was found in the actual node

	// Search for Ptr in the subtrees rooted at the child nodes
	for(ChildIter = MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		if((HTree = (*ChildIter)->getHTNode(iID)) != NULL)
			return HTree;

	return NULL;
}


/*
***Description***
The method converts the hypertree such that the actual tree node becomes the root.

INPUT:
OUTPUT: 

***History***
Written: (13.01.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::setRoot()
{
	if(MyParent != NULL) {
		// Make parent node to the root
		MyParent->setRoot();

		// Remove actual node from the child set of the parent node
		if(!MyParent->remChild(this))
			writeErrorMsg("Invalid parent/child relation.", "Hypertree::setRoot");

		// Consider parent node as child of the actual node
		insChild(MyParent);

		// Make actual node to the root
		MyParent = NULL;
	}
}


// Returns true iff the hypertree node does not have a parent
bool Hypertree::isRoot()
{
	return MyParent == NULL;
}


/*
***Description***
The method returns the hypertree width, i.e., the maximum cardinality of the lambda-sets.

INPUT:
OUTPUT: return: Hypertree width

***History***
Written: (13.01.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

int Hypertree::getHTreeWidth()
{
	list<Hypertree *>::iterator ChildIter;
	int iTmp, iTreeWidth;

	// Set treewidth to the cardinality of the actual lambda-set
	iTreeWidth = MyLambda.size();  
	
	// Compute maximum between treewidths of all subtrees and the actual cardinality
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++) {
		iTmp = (*ChildIter)->getHTreeWidth();
		if(iTmp > iTreeWidth)
			iTreeWidth = iTmp;
	}

	return iTreeWidth;
}


/*
***Description***
The method returns the tree width, i.e., the maximum cardinality of the chi-sets.

INPUT:
OUTPUT: return: Tree width

***History***
Written: (02.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

int Hypertree::getTreeWidth()
{
	list<Hypertree *>::iterator ChildIter;
	int iTmp, iTreeWidth;

	// Set treewidth to the cardinality of the actual lambda-set
	iTreeWidth = MyChi.size();  
	
	// Compute maximum between treewidths of all subtrees and the actual cardinality
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++) {
		iTmp = (*ChildIter)->getTreeWidth();
		if(iTmp > iTreeWidth)
			iTreeWidth = iTmp;
	}

	return iTreeWidth - 1;
}


// The method returns the chi-set labelling the actual tree node.
set<Node *> *Hypertree::getChi()
{
	return &MyChi;
}


// The method returns the lambda-set labelling the actual tree node.
set<Hyperedge *> *Hypertree::getLambda()
{
	return &MyLambda;
}


// Sets the label of the hypertree-node
void Hypertree::setLabel(int iLabel)
{
	iMyLabel = iLabel;
}


// Returns the label of the hypertree-node
int Hypertree::getLabel()
{
	return iMyLabel;
}


/*
***Description***
The method sets the labels of all hypertree-nodes in the subtree to zero.

INPUT:
OUTPUT:

***History***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::resetLabels()
{
	list<Hypertree *>::iterator ChildIter;

	iMyLabel = 0;
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->resetLabels();
}


/*
***Description***
The method sets the labels of all hypertree-nodes in the subtree to a unique ID.

INPUT:	iStartID : Least ID labelling a hypertree-node in the subtree
OUTPUT:	return: Largest ID labelling a hypertree-node in the subtree

***History***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

int Hypertree::setIDLabels(int iStartID)
{
	list<Hypertree *>::iterator ChildIter;

	iMyLabel = iStartID;
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		iStartID = (*ChildIter)->setIDLabels(iStartID + 1);

	return iStartID;
}


/*
***Description***
The method checks whether the hypertree contains cycles of the form A->C, B->C, where A != B.

INPUT:
OUTPUT:	return: true if the hypertree contains cycles; otherwise false

***History***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

bool Hypertree::isCyclic()
{
	resetLabels();
	return isCyclic_pvt();
}


/*
***Description***
The method removes redundant hypertree nodes. Hypertree nodes can be removed if their chi-set 
is a subset of the chi-set of their parent or of one of their children.

INPUT:	bLambdaUnion: Indicates if the union of the lambda-labels should be built.
OUTPUT:

***History***
Written: (08.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::shrink(bool bLambdaUnion)
{
	bool bChildFound;
	Hypertree *Child = this;
	list<Hypertree *>::iterator ChildIter, ChildIter2;
	set<Node *>::iterator ChiIter;
	set<Hyperedge *>::iterator LambdaIter;
	set<void *>::iterator PtrIter;

	resetLabels();
	while(Child != NULL) {
		do {
			bChildFound = false;
			// Check whether there are unprocessed child nodes
			for(ChildIter=Child->MyChildren.begin(); ChildIter != Child->MyChildren.end(); ChildIter++)
				if((*ChildIter)->getLabel() == 0) {
					bChildFound = true;
					Child = (*ChildIter);
					break;
				}
		} while(bChildFound);

		// Remove the actual node if its chi-set is a subset of the chi-set of one of its children
		for(ChildIter=Child->MyChildren.begin(); ChildIter != Child->MyChildren.end();)
			if(isSubset(&Child->MyChi, (*ChildIter)->getChi())) {

				// Move the chi-set from the child to the actual node
				for(ChiIter=(*ChildIter)->MyChi.begin(); ChiIter != (*ChildIter)->MyChi.end(); ChiIter++)
					Child->MyChi.insert(*ChiIter);
			
				// Move the lambda-set from the child to the actual node
				if(!bLambdaUnion)
					Child->MyLambda.clear();
				for(LambdaIter=(*ChildIter)->MyLambda.begin(); LambdaIter != (*ChildIter)->MyLambda.end(); LambdaIter++)
					Child->MyLambda.insert(*LambdaIter);
			
				// Move the pointers from the child to the actual node
				for(PtrIter=(*ChildIter)->MyPointers.begin(); PtrIter != (*ChildIter)->MyPointers.end(); PtrIter++)
					Child->MyPointers.insert(*PtrIter);

				// Remove the child node (its contents is now stored in the actual node)
				for(ChildIter2=(*ChildIter)->MyChildren.begin(); ChildIter2 != (*ChildIter)->MyChildren.end(); ChildIter2++)
					Child->insChild(*ChildIter2);
				(*ChildIter)->MyChildren.clear();
				delete (*ChildIter);
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Remove all child nodes whose chi-sets are subsets of the chi-set of the actual node
		for(ChildIter=Child->MyChildren.begin(); ChildIter != Child->MyChildren.end();)
			if(isSubset((*ChildIter)->getChi(), &Child->MyChi)) {
			
				// Move the lambda-set from the child to the actual node
				if(bLambdaUnion)
					for(LambdaIter=(*ChildIter)->MyLambda.begin(); LambdaIter != (*ChildIter)->MyLambda.end(); LambdaIter++)
						Child->MyLambda.insert(*LambdaIter);

				// Move the pointers from the child to the actual node
				for(PtrIter=(*ChildIter)->MyPointers.begin(); PtrIter != (*ChildIter)->MyPointers.end(); PtrIter++)
					Child->MyPointers.insert(*PtrIter);

				for(ChildIter2=(*ChildIter)->MyChildren.begin(); ChildIter2 != (*ChildIter)->MyChildren.end(); ChildIter2++)
					Child->insChild(*ChildIter2);
				(*ChildIter)->MyChildren.clear();
				delete (*ChildIter);
				ChildIter = Child->MyChildren.erase(ChildIter);
			}
			else
				++ChildIter;

		// Label the node and go up to the parent node
		Child->iMyLabel = 1;
		Child = Child->MyParent;
	}
}


/*
***Description***
The method swaps all chi- and lambda-sets labelling the nodes of the subtree.

INPUT:
OUTPUT:

***History***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::swapChiLambda()
{
	list<Hypertree *>::iterator ChildIter;
	set<Hyperedge *> Tmp;
	set<Node *>::iterator ChiIter;
	set<Hyperedge *>::iterator LambdaIter;

	// Swap chi- and lambda set labelling the actual node
	for(LambdaIter=MyLambda.begin(); LambdaIter != MyLambda.end(); LambdaIter++)
		Tmp.insert(*LambdaIter);
	MyLambda.clear();
	for(ChiIter=MyChi.begin(); ChiIter != MyChi.end(); ChiIter++)
		MyLambda.insert((Hyperedge *)(*ChiIter));
	MyChi.clear();
	for(LambdaIter=Tmp.begin(); LambdaIter != Tmp.end(); LambdaIter++)
		MyChi.insert((Node *)(*LambdaIter));
	Tmp.clear();

	// Swap chi- and lambda set for all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->swapChiLambda();
}


/*
***Description***
The method eliminates hyperedges in the lambda-set whose nodes occurring in the chi-set are 
already covered by other hyperedges in the lambda-set, i.e., the method reduces the lambda-set
to the minimal set of hyperedges necessary to cover the chi-set.

INPUT:
OUTPUT:

***History***
Written: (03.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::elimCovEdges()
{
	int i;
	set<Hyperedge *>::iterator LambdaIter;
	set<Node *>::iterator ChiIter;
	CompSet *Nodes, *HEdges, *CovEdges;
	list<Hypertree *>::iterator ChildIter;
	SetCover SC;

	Nodes = new CompSet(MyChi.size());
	HEdges = new CompSet(MyLambda.size());
	if((Nodes == NULL) || (HEdges == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::elimCovEdges");

	// Copy the chi-set into a component set
	for(ChiIter=MyChi.begin(); ChiIter != MyChi.end(); ChiIter++)
		Nodes->insComp(*ChiIter);

	// Copy the lambda-set into a component set
	for(LambdaIter=MyLambda.begin(); LambdaIter != MyLambda.end(); LambdaIter++)
		HEdges->insComp(*LambdaIter);

	// Apply set covering
	CovEdges = SC.cover(Nodes, HEdges);

	// Copy the covering hyperedges into the lambda-set
	MyLambda.clear();
	for(i=0; i < CovEdges->size(); i++)
		MyLambda.insert((Hyperedge *)((*CovEdges)[i]));

	delete Nodes;
	delete HEdges;
	delete CovEdges;

	// Eliminate covered hyperedges in all subtrees
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->elimCovEdges();
}


/*
***Description***
The method reduces the number of labellings in the lambda-sets in such a way that lambda
labellings are replaced by lambda labellings of the parent and child nodes if the hypertree 
conditions remain satisfied and the hypertree-width becomes smaller.

INPUT:
OUTPUT:

***History***
Written: (22.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::reduceLambda()
{
	reduceLambdaTopDown();
	reduceLambdaBottomUp();
}


/*
***Description***
The method sets the chi-sets of the hypertree according to the lambda-sets in such a way that 
each hyperedge is covered and the chi-labellings are connected (provided the lambda-labellings
are connected).

INPUT:	HGraph: Underlying hypergraph
OUTPUT:

***History***
Written: (01.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::setChi(Hypergraph *HGraph)
{
	int i, j;
	Hypertree **CovNodes;

	CovNodes = new Hypertree*[HGraph->getNbrOfEdges()];
	if(CovNodes == NULL)
		writeErrorMsg("Error assigning memory.", "Hypertree::setChi");
	for(i=0; i < HGraph->getNbrOfEdges(); i++) {
		HGraph->getEdge(i)->setLabel(i);
		CovNodes[i] = NULL;
	}

	selCovHTNodes(CovNodes);
	for(i=0; i < HGraph->getNbrOfEdges(); i++)
		if(CovNodes[i] != NULL)
			for(j=0; j < HGraph->getEdge(i)->getNbrOfNodes(); j++)
				CovNodes[i]->getChi()->insert(HGraph->getEdge(i)->getNode(j));
	
	setChi_Conn(HGraph);
}


/*
***Description***
The method sets the lambda-sets of the hypertree according to the chi-sets in such a way that 
set covering algorithms are applied in order to cover the nodes in the chi-sets by the hyperedges
in the lambda-sets.

INPUT:	HGraph: Underlying hypergraph
OUTPUT:

***History***
Written: (02.03.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/
void Hypertree::setLambda(Hypergraph *HGraph)
{
	int i, j;
	set<Node *>::iterator ChiIter;
	set<Hyperedge *>::iterator LambdaIter;
	list<Hypertree *>::iterator ChildIter;
	CompSet *Nodes, *HEdges, *CovEdges;
	SetCover SC;

	Nodes = new CompSet(MyChi.size());
	HEdges = new CompSet(HGraph->getNbrOfEdges());
	if((Nodes == NULL) || (HEdges == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::setLambda");

	// Mark all nodes that are already covered by hyperedges in the lambda-set
	HGraph->resetNodeLabels();
	for(LambdaIter=MyLambda.begin(); LambdaIter != MyLambda.end(); LambdaIter++)
		for(i=0; i < (*LambdaIter)->getNbrOfNodes(); i++)
			(*LambdaIter)->getNode(i)->setLabel(1);

	// Choose all nodes in the chi-set that are not covered
	for(ChiIter=MyChi.begin(); ChiIter != MyChi.end(); ChiIter++)
		if((*ChiIter)->getLabel() == 0) {
			Nodes->insComp(*ChiIter);
			for(j=0; j < (*ChiIter)->getNbrOfEdges(); j++)
				HEdges->insComp((*ChiIter)->getEdge(j));
		}

	// Apply set covering
	CovEdges = SC.cover(Nodes, HEdges);

	// Copy all covering hyperedges into the lambda-set
	for(i=0; i < CovEdges->size(); i++)
		MyLambda.insert((Hyperedge *)((*CovEdges)[i]));

	delete Nodes;
	delete HEdges;
	delete CovEdges;

	// Set the lambda-sets for all subtrees
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->setLambda(HGraph);
}


/*
***Description***
The method resets the lambda-set to a possibly smaller set.

INPUT:	HGraph: Underlying hypergraph
OUTPUT:

***History***
Written: (05.04.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::resetLambda(Hypergraph *HGraph)
{
	int i;
	set<Node *>::iterator ChiIter;
	set<Hyperedge *>::iterator LambdaIter;
	list<Hypertree *>::iterator ChildIter;
	CompSet *Nodes, *HEdges, *CovEdges;
	SetCover SC;

	Nodes = new CompSet(MyChi.size());
	HEdges = new CompSet(HGraph->getNbrOfEdges());
	if((Nodes == NULL) || (HEdges == NULL))
		writeErrorMsg("Error assigning memory.", "Hypertree::resetLambda");

	// Choose all nodes in the chi-set to be covered
	for(ChiIter=MyChi.begin(); ChiIter != MyChi.end(); ChiIter++) {
		Nodes->insComp(*ChiIter);
		for(i=0; i < (*ChiIter)->getNbrOfEdges(); i++)
			HEdges->insComp((*ChiIter)->getEdge(i));
	}

	// Apply set covering
	CovEdges = SC.cover(Nodes, HEdges);

	if(CovEdges->size() < (int)MyLambda.size()) {
		MyLambda.clear();
		// Copy all covering hyperedges into the lambda-set
		for(i=0; i < CovEdges->size(); i++)
			MyLambda.insert((Hyperedge *)((*CovEdges)[i]));
	}

	delete Nodes;
	delete HEdges;
	delete CovEdges;

	// Set the lambda-sets for all subtrees
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->setLambda(HGraph);
}


/*
***Description***
The method checks the first hypertee decomposition condition. It returns a witness hyperedge
if the conditions is violated; otherwise the pointer NULL.

INPUT:	HGraph: Underlying hypergraph
OUTPUT: return: Witness hyperedge if the first condition is violated; otherwise NULL

***History***
Written: (25.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Hyperedge *Hypertree::checkCond1(Hypergraph *HGraph)
{
	HGraph->resetEdgeLabels();
	HGraph->resetNodeLabels();

	// Label all hyperedges covered by some chi-set
	labelCovEdges(HGraph);

	// Search for hyperedges that are not labeled
	for(int i=0; i < HGraph->getNbrOfEdges(); i++)
		if(HGraph->getEdge(i)->getLabel() == 0)
			return HGraph->getEdge(i);

	return NULL;
}


/*
***Description***
The method checks the second hypertee decomposition condition. It returns a witness node if the 
condition is violated; otherwise the pointer NULL.

INPUT:
OUTPUT:	return: Witness node if the second condition is violated; otherwise NULL

***History***
Written: (25.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Node *Hypertree::checkCond2(Hypergraph *HGraph)
{
	HGraph->resetNodeLabels();
	return checkCond2();
}


/*
***Description***
The method checks the third hypertee decomposition condition. It returns a witness hypertree-node 
if the condition is violated; otherwise the pointer NULL.

INPUT:
OUTPUT:	return: Witness hypertree node if the third condition is violated; otherwise NULL

***History***
Written: (25.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Hypertree *Hypertree::checkCond3(Hypergraph *HGraph)
{
	HGraph->resetNodeLabels();
	return checkCond3();
}


/*
***Description***
The method checks the fourth hypertee decomposition condition. It returns a witness hypertree-node 
if the condition is violated; otherwise the pointer NULL.

INPUT:	HGraph: Underlying hypergraph
OUTPUT: return: Witness hypertree node if the fourth condition is violated; otherwise NULL

***History***
Written: (25.02.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

Hypertree *Hypertree::checkCond4(Hypergraph *HGraph)
{
	int i;
	set<Hyperedge *>::iterator EdgeIter;
	set<Node *>::iterator NodeIter;
	list<Hypertree *>::iterator ChildIter;
	Hypertree *Witness;

	// Label all variables occurring in some chi-set of the subtree
	HGraph->resetNodeLabels();
	labelChiSets();

	// Compute the intersection with the variables of hyperedges in the lambda-set
	for(EdgeIter=MyLambda.begin(); EdgeIter != MyLambda.end(); EdgeIter++)
		for(i=0; i < (*EdgeIter)->getNbrOfNodes(); i++)
			if((*EdgeIter)->getNode(i)->getLabel() != 0)
				(*EdgeIter)->getNode(i)->setLabel(2);

	// Compute the set difference between the intersection and the chi-set
	for(NodeIter=MyChi.begin(); NodeIter != MyChi.end(); NodeIter++)
		(*NodeIter)->decLabel();
	
	// Check whether the set difference is empty
	for(i=0; i < HGraph->getNbrOfNodes(); i++)
		if(HGraph->getNode(i)->getLabel() > 1)
			return this;

	// Check fourth condition for all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		if((Witness = (*ChildIter)->checkCond4(HGraph)) != NULL)
			return Witness;

	return NULL;
}


/*
***Description***
The method collects labels of hypertree-nodes with given width.

INPUT:	ConflictVars: Set to collect elements of the chi-sets resp. lambda-set
		iWidth: Width of the hypertree-nodes (size of the lambda-sets)
OUTPUT:	ConflictVars: Set of collected elements

***History***
Written: (21.04.05, MS)
------------------------------------------------------------------------------------------------
Changed: (date, author)
Comments:
------------------------------------------------------------------------------------------------
*/

void Hypertree::getConflictVars(set<Node *> *ConflictVars, int iWidth, bool bDual)
{
	set<Node *>::iterator ChiIter;
	set<Hyperedge *>::iterator LambdaIter;
	list<Hypertree *>::iterator ChildIter;

	// Collect elements of the actual chi-set
	if((int)MyLambda.size() == iWidth) {
		if(bDual)
			for(LambdaIter=MyLambda.begin(); LambdaIter != MyLambda.end(); LambdaIter++)
				ConflictVars->insert((Node *)(*LambdaIter));
		else
			for(ChiIter=MyChi.begin(); ChiIter != MyChi.end(); ChiIter++)
				ConflictVars->insert(*ChiIter);
	}

	// Collect elements of all children
	for(ChildIter=MyChildren.begin(); ChildIter != MyChildren.end(); ChildIter++)
		(*ChildIter)->getConflictVars(ConflictVars, iWidth, bDual);
}

double Hypertree::getAverageJoinOverhead()
{
	return (double)getJoinOverheadSum()/getNumberOfJoinNodes();
}

double Hypertree::getAverageChiSetChange()
{
	return (double)getChiSetChangeSum()/(getNumberOfDescendants() - getNumberOfJoinNodes() - getNumberOfLeafNodes());
}

unsigned int Hypertree::getJoinOverheadSum()
{
	unsigned int sum = 0;
	bool isJoin = false;
	vector<Node *> isect(MyChi.begin(), MyChi.end());

	if(MyChildren.size() >= (unsigned int)2)
		isJoin = true;
	else
		isect.clear();

	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
	{
		if(isJoin)
		{
			set<Node *> tmp(isect.begin(), isect.end()); isect.clear();
			set_intersection(tmp.begin(), tmp.end(), (*i)->getChi()->begin(), (*i)->getChi()->end(), inserter(isect, isect.begin()));
		}

		sum += (*i)->getJoinOverheadSum();
	}
	sum += isect.size();
	return sum;
}

unsigned int Hypertree::getChiSetChangeSum()
{
	unsigned int sum = 0;

	if(MyChildren.size() == (unsigned int)1)
	{
		vector<Node *> sdiff;
		set_symmetric_difference(MyChi.begin(), MyChi.end(),
				(*(MyChildren.begin()))->getChi()->begin(),
				(*(MyChildren.begin()))->getChi()->end(), inserter(sdiff, sdiff.begin()));
		sum += sdiff.size();
	}

	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		sum += (*i)->getChiSetChangeSum();
	return sum;
}

unsigned int Hypertree::getNumberOfDescendants()
{
	unsigned int count = 1;
	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		count += (*i)->getNumberOfDescendants();
	return count;
}

unsigned int Hypertree::getNumberOfLeafNodes()
{
	unsigned int count = 0;
	if(MyChildren.size() == (unsigned int)0) ++count;
	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		count += (*i)->getNumberOfLeafNodes();
	return count;
}

unsigned int Hypertree::getNumberOfJoinNodes()
{
	unsigned int count = 0;
	if(MyChildren.size() > (unsigned int)1) ++count;
	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		count += (*i)->getNumberOfJoinNodes();
	return count;
}


