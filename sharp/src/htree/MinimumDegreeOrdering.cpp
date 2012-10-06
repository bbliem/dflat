#include <MinimumDegreeOrdering.hpp>
#include <probSol.hpp>
#include <Hypergraph.hpp>
#include <Node.hpp>

using namespace sharp;

MinimumDegreeOrdering::MinimumDegreeOrdering()
{
}

MinimumDegreeOrdering::~MinimumDegreeOrdering()
{
}

Ordering MinimumDegreeOrdering::getOrdering(Hypergraph *g)
{
	//TODO: possibly implement "better" heuristics below that
	// actually improves matters
	if(g->getNbrOfNodes() > 0) return g->getMIWOrder();
	
	probSol ps; 
	ps.createGraphFromHyp(g);
	ps.MinDegreeOrdering();

	int size = g->getNbrOfNodes();
	Ordering order = new Node*[size+1];

	// Initialize variable order
	for(int i=0; i < size; i++) {
		
		int k = ps.iNodePosInOrdering[i];
			
		order[size-k-1] = g->getNode(i);
		order[size-k-1]->setLabel(0);
	}
	order[size] = 0;

	return order;
}
