#include <config.h>

#include <Problem.hpp>

#include <Node.hpp>
#include <Hyperedge.hpp>
#include <Hypergraph.hpp>
#include <H_BucketElim.hpp>
#include <ExtendedHypertree.hpp>
#include <AbstractAlgorithm.hpp>
#include <AbstractHypertreeDecompositionAlgorithm.hpp>
#include <BucketEliminationAlgorithm.hpp>
#include <MinimumDegreeOrdering.hpp>

#include <cstring>
#include <iostream>
using namespace std;

namespace sharp
{
	Problem::Problem(AbstractHypertreeDecompositionAlgorithm *htdalgorithm, bool collectStatistics)
	{
		this->initialize(htdalgorithm, collectStatistics);
	}

	Problem::Problem(bool collectStatistics)
	{
		this->initialize(NULL, collectStatistics);
	}

	void Problem::initialize(AbstractHypertreeDecompositionAlgorithm *htdalgorithm, bool collectStatistics)
	{
		this->parsed = false;
		this->statistics = collectStatistics;
		this->htdalgorithm = htdalgorithm ? htdalgorithm : new BucketEliminationAlgorithm(new MinimumDegreeOrdering());
	
		// start numbering of vertices at 1 instead of 0
		this->createAuxiliaryVertex();
	}
	
	Problem::~Problem()
	{
	}
	
	Table *Problem::calculateTable(AbstractHTDAlgorithm *algorithm)
	{
		return this->calculateTableFromDecomposition(algorithm, this->calculateHypertreeDecomposition());
	}

	Table *Problem::calculateTableFromDecomposition(AbstractHTDAlgorithm *algorithm, ExtendedHypertree *root)
	{
		return algorithm->evaluate(root);
	}
	
	ExtendedHypertree *Problem::calculateHypertreeDecomposition()
	{
	        if(!this->parsed)
	        {
	                this->parse();
	                this->preprocess();
	                this->parsed = true;
	        }
	
	        Hypergraph *hg = this->buildHypergraphRepresentation();
	
		Hypertree *ht = this->htdalgorithm->decompose(hg);
		ht = new ExtendedHypertree(ht);

		return this->prepareHypertreeDecomposition((ExtendedHypertree *)ht);
	}

	void Problem::preprocess()
	{
	}

	ExtendedHypertree *Problem::prepareHypertreeDecomposition(ExtendedHypertree *root)
	{
		return root;
	}
	
	Vertex Problem::getVertexId(string vertexName)
	{
		ReverseNameMap::iterator it = reverseVertexNames.find(vertexName);
		if(it != reverseVertexNames.end()) return it->second;
		return VERTEXNOTFOUND;
	}
	
	string Problem::getVertexName(Vertex vertexId)
	{
		return this->vertexNames[vertexId];
	}
	
	Hypergraph *Problem::createGraphFromSets(VertexSet vertices, EdgeSet edges)
	{
		return createGraphFromDisjointSets(vertices, VertexSet(), edges);
	}

	Hypergraph *Problem::createGraphFromDisjointSets(VertexSet v1, VertexSet v2, EdgeSet edges)
	{
		vector<Node *> lookup; lookup.resize(v1.size() + v2.size(), NULL);
		int edgeId = 0;
		Hypergraph *hg = new Hypergraph(true /* we create a graph, not a hypergraph */);

		for(VertexSet::iterator it = v1.begin(); it != v1.end(); ++it)
		{
			if((unsigned int)*it >= lookup.capacity()) lookup.resize(*it + 1);
			Node *n; CHECKNULL(n = new Node(*it, *it), "memory allocation failure");
			lookup[*it] = n;
			hg->MyNodes.push_back(n);
		}

		for(VertexSet::iterator it = v2.begin(); it != v2.end(); ++it)
		{
			if((unsigned int)*it >= lookup.capacity()) lookup.resize(*it + 1);
			Node *n; CHECKNULL(n = new Node(*it, *it), "memory allocation failure");
			lookup[*it] = n;
			hg->MyNodes.push_back(n);
		}

		for(EdgeSet::iterator it = edges.begin(); it != edges.end(); ++it)
		{
			Hyperedge *e; CHECKNULL(e = new Hyperedge(edgeId, edgeId), "memory allocation failure"); ++edgeId;
			hg->MyEdges.push_back(e);

			lookup[it->first]->insEdge(e);
			lookup[it->second]->insEdge(e);
			e->insNode(lookup[it->first]);
			e->insNode(lookup[it->second]);
		}

		for(int i = 0; i < (int)hg->MyNodes.size(); ++i)
			hg->MyNodes[i]->updateNeighbourhood();
		
		for(int i = 0; i < (int)hg->MyEdges.size(); ++i)
			hg->MyEdges[i]->updateNeighbourhood();
		
		hg->iMyMaxNbrOfNodes = hg->MyNodes.size();
		hg->iMyMaxNbrOfEdges = hg->MyEdges.size();

		return hg;
	}

	Hypergraph *Problem::createHypergraphFromSets(VertexSet vertices, HyperedgeSet hyperedges)
	{
		vector<Node *> lookup; lookup.resize(vertices.size(), NULL);
		int edgeId = 0;
		Hypergraph *hg = new Hypergraph();

		for(VertexSet::iterator it = vertices.begin(); it != vertices.end(); ++it)
		{
			if((unsigned int)*it >= lookup.capacity()) lookup.resize(*it + 1);
			Node *n; CHECKNULL(n = new Node(*it, *it), "memory allocation failure");
			lookup[*it] = n;
			hg->MyNodes.push_back(n);
		}

		for(HyperedgeSet::iterator it = hyperedges.begin(); it != hyperedges.end(); ++it)
		{
			Hyperedge *e; CHECKNULL(e = new Hyperedge(edgeId, edgeId), "memory allocation failure"); ++edgeId;
			hg->MyEdges.push_back(e);

			for(VertexSet::iterator vit = it->begin(); vit != it->end(); ++vit)
			{
				lookup[*vit]->insEdge(e);
				e->insNode(lookup[*vit]);
			}
		}

		for(int i = 0; i < (int)hg->MyNodes.size(); ++i)
			hg->MyNodes[i]->updateNeighbourhood();
		
		for(int i = 0; i < (int)hg->MyEdges.size(); ++i)
			hg->MyEdges[i]->updateNeighbourhood();
		
		hg->iMyMaxNbrOfNodes = hg->MyNodes.size();
		hg->iMyMaxNbrOfEdges = hg->MyEdges.size();

		return hg;
	}
	
	Vertex Problem::storeVertexName(string name)
	{
		Vertex v;
		if((v = getVertexId(name)) != VERTEXNOTFOUND) return v;
		
		vertexNames.push_back(name);
		reverseVertexNames.insert(ReverseNameMap::value_type(name, vertexNames.size() - 1));
	
		return vertexNames.size() - 1;
	}
	
	unsigned int Problem::getVertexCount()
	{
		return vertexNames.size() - 1;
	}
	
	Vertex Problem::createAuxiliaryVertex()
	{
		vertexNames.push_back(string("__aux"));
		return vertexNames.size() - 1;
	}

	void Problem::clearVertexStore()
	{
		this->vertexNames.clear();
		this->reverseVertexNames.clear();
		this->createAuxiliaryVertex();
	}
	
	void Problem::printVertexNames(ostream &out)
	{
		for(unsigned int i = 0; i < this->vertexNames.size(); ++i)
			out << "(" << i << "=" << this->vertexNames[i] << ")";
		out << endl;
	}

} // namespace sharp
