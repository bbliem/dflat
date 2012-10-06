#include <config.h>

#include <ExtendedHypertree.hpp>

#include <Global.hpp>
#include <Node.hpp>
using namespace sharp;

#include <cstdlib>
#include <algorithm>
#include <iostream>
using namespace std;

ExtendedHypertree::ExtendedHypertree(Hypertree *node) : Hypertree()
{
	this->type = General;
	this->MyParent = node->getParent();

	for(list<Hypertree *>::const_iterator i = node->getChildren()->begin(); i != node->getChildren()->end(); ++i)
		this->insChild(new ExtendedHypertree(*i));

	for(set<Node *>::iterator i = node->getChi()->begin(); i != node->getChi()->end(); ++i)
	{
		this->vertices.insert((*i)->getName());
	}

	if(!this->MyParent) delete node;
}

ExtendedHypertree::ExtendedHypertree(const VertexSet &vertices) : Hypertree()
{
	this->MyParent = NULL;
	this->vertices = vertices;
	this->type = General;
}

ExtendedHypertree::~ExtendedHypertree() { }

ExtendedHypertree *ExtendedHypertree::createChild(ExtendedHypertree *child, const VertexSet &vertices, Vertex difference, TreeNodeType type)
{
	ExtendedHypertree *parent = child->parent();
	ExtendedHypertree *newChild = new ExtendedHypertree(vertices);

	VertexSet diff; diff.insert(difference);

	parent->remChild(child);
	newChild->insChild(child);
	parent->insChild(newChild);

	parent->type = type;
	if(type == Introduction) parent->introduced = diff;
	else parent->removed = diff;

	return child;
}

TreeNodeType ExtendedHypertree::getType() const
{
	return this->type;
}

int ExtendedHypertree::getTreeWidth()
{
	int tw = this->vertices.size() - 1;
	for(list<Hypertree *>::iterator it = getChildren()->begin(); it != getChildren()->end(); ++it)
		tw = max(tw, (*it)->getTreeWidth());
	return tw;
}

Vertex ExtendedHypertree::getDifference() const
{
	if(this->introduced.size() != 0) return *this->introduced.begin();
	else if(this->removed.size() != 0) return *this->removed.begin();
	else return VERTEXNOTFOUND;
}

const VertexSet &ExtendedHypertree::getIntroducedVertices() const
{
	return this->introduced;
}

const VertexSet &ExtendedHypertree::getRemovedVertices() const
{
	return this->removed;
}

bool ExtendedHypertree::isRoot() const
{
	return this->MyParent == NULL;
}

ExtendedHypertree *ExtendedHypertree::normalize(NormalizationType normalization)
{
	ExtendedHypertree *current = new ExtendedHypertree(this->vertices);

	if(this->MyChildren.size() == 0)
	{
		ExtendedHypertree *empty = NULL;

		switch(normalization)
		{
		case NoNormalization:
		case DefaultNormalization:
		case SemiNormalization:
			current->type = Leaf;
			current->introduced = current->vertices;
			break;
		case StrongNormalization:
			empty = new ExtendedHypertree(VertexSet());
			empty->type = Leaf;
			current->insChild(empty);
			empty->adapt();
			break;
		default:
			CHECK0(0, "undefined normalization type"); return NULL;
		}
	}
	else if(this->MyChildren.size() == 1)
	{
		ExtendedHypertree *child = ((ExtendedHypertree *)*this->MyChildren.begin())->normalize(normalization);
		current->insChild(child);

		switch(normalization)
		{
		case NoNormalization:
		case SemiNormalization:
			current->type = Permutation;
			set_difference(current->vertices.begin(), current->vertices.end(),
					child->vertices.begin(), child->vertices.end(),
					inserter(current->introduced, current->introduced.begin()));
			set_difference(child->vertices.begin(), child->vertices.end(),
					current->vertices.begin(), current->vertices.end(),
					inserter(current->removed, current->removed.begin()));
			break;
		case DefaultNormalization:
		case StrongNormalization:
			child->adapt();
			break;
		default:
			CHECK0(0, "undefined normalization type"); return NULL;
		}
	}
	else
	{
		if(normalization == NoNormalization)
		{
			for(list<Hypertree *>::const_iterator i = this->MyChildren.begin(); i != this->MyChildren.end(); ++i)
				current->insChild(((ExtendedHypertree *)*i)->normalize(normalization));
		}
		else
		{
			vector<ExtendedHypertree *> childlist;
			vector<ExtendedHypertree *> newchildren;

			for(list<Hypertree *>::const_iterator i = this->MyChildren.begin(); i != this->MyChildren.end(); ++i)
			{
				newchildren.push_back(((ExtendedHypertree *)*i)->normalize(normalization));
			}

			while(newchildren.size() > 1)
			{
				childlist.swap(newchildren); newchildren.clear();

				for(unsigned int i = 0; i < childlist.size(); ++i)
					if(++i < childlist.size())
						newchildren.push_back(createNormalizedJoinNode(childlist[i-1], childlist[i], current->vertices, normalization));
					else
						newchildren.push_back(childlist[i-1]);
			}

			ExtendedHypertree *child = newchildren[0];

			if(child->vertices == current->vertices)
			{
				delete current;
				current = child;
			}
			else
			{
				current->insChild(child);

				switch(normalization)
				{
				case SemiNormalization:
					current->type = Permutation;
					set_difference(current->vertices.begin(), current->vertices.end(),
							child->vertices.begin(), child->vertices.end(),
							inserter(current->introduced, current->introduced.begin()));
					set_difference(child->vertices.begin(), child->vertices.end(),
							current->vertices.begin(), current->vertices.end(),
							inserter(current->removed, current->removed.begin()));
					break;
				case DefaultNormalization:
				case StrongNormalization:
					child->adapt();
					break;
				default:
					CHECK0(0, "undefined normalization type"); return NULL;
				}
			}
		}
	}

	return current;
}

ExtendedHypertree *ExtendedHypertree::createNormalizedJoinNode(ExtendedHypertree *left, ExtendedHypertree *right, const VertexSet &top, NormalizationType normalization)
{
	VertexSet intersection;

	set_intersection(top.begin(), top.end(),
			left->vertices.begin(), left->vertices.end(),
			inserter(intersection, intersection.end()));
	set_intersection(top.begin(), top.end(),
			right->vertices.begin(), right->vertices.end(),
			inserter(intersection, intersection.end()));

	ExtendedHypertree *branch = new ExtendedHypertree(intersection);
	branch->type = Branch;

	if(intersection == left->vertices)
	{
		branch->insChild(left);
	}
	else
	{
		ExtendedHypertree *newleft = new ExtendedHypertree(intersection);
		branch->insChild(newleft);
		newleft->insChild(left);
		switch(normalization)
		{
		case SemiNormalization:
			newleft->type = Permutation;
			set_difference(newleft->vertices.begin(), newleft->vertices.end(),
					left->vertices.begin(), left->vertices.end(),
					inserter(newleft->introduced, newleft->introduced.begin()));
			set_difference(left->vertices.begin(), left->vertices.end(),
					newleft->vertices.begin(), newleft->vertices.end(),
					inserter(newleft->removed, newleft->removed.begin()));
			break;
		case DefaultNormalization:
		case StrongNormalization:
			left->adapt();
			break;
		default:
			CHECK0(0, "undefined normalization type"); return NULL;
		}
	}

	if(intersection == right->vertices)
	{
		branch->insChild(right);
	}
	else
	{
		ExtendedHypertree *newright = new ExtendedHypertree(intersection);
		branch->insChild(newright);
		newright->insChild(right);
		switch(normalization)
		{
		case SemiNormalization:
			newright->type = Permutation;
			set_difference(newright->vertices.begin(), newright->vertices.end(),
					right->vertices.begin(), right->vertices.end(),
					inserter(newright->introduced, newright->introduced.begin()));
			set_difference(right->vertices.begin(), right->vertices.end(),
					newright->vertices.begin(), newright->vertices.end(),
					inserter(newright->removed, newright->removed.begin()));
			break;
		case DefaultNormalization:
		case StrongNormalization:
			right->adapt();
			break;
		default:
			CHECK0(0, "undefined normalization type"); return NULL;
		}
	}

	return branch;
}

void ExtendedHypertree::adapt()
{
	if(this->MyParent == NULL) return;

	unsigned int changes;

	list<Vertex> currentVertices(this->parent()->vertices.begin(), this->parent()->vertices.end());

	vector<Vertex> redVertices(currentVertices.size());
	vector<Vertex> greenVertices(this->vertices.size());

	vector<Vertex>::iterator it
		= set_difference(this->parent()->vertices.begin(), this->parent()->vertices.end(), 
				this->vertices.begin(), this->vertices.end(), redVertices.begin());
	redVertices.resize(it - redVertices.begin());

	it = set_difference(this->vertices.begin(), this->vertices.end(), this->parent()->vertices.begin(),
		       		this->parent()->vertices.end(), greenVertices.begin());
	greenVertices.resize(it - greenVertices.begin());

	changes = redVertices.size() + greenVertices.size();

	for(it = redVertices.begin(); changes > 1 && it != redVertices.end(); ++it)
	{
		currentVertices.remove(*it);
		createChild(this, VertexSet(currentVertices.begin(), currentVertices.end()), *it, Introduction);
		--changes;
	}

	for(it = greenVertices.begin(); changes > 1 && it != greenVertices.end(); ++it)
	{
		currentVertices.push_back(*it);
		createChild(this, VertexSet(currentVertices.begin(), currentVertices.end()), *it, Removal);
		--changes;
	}

	if(greenVertices.size() != 0)
	{
		VertexSet rem; rem.insert(*greenVertices.rbegin());
		this->parent()->type = Removal;
		this->parent()->removed = rem;
	}
	else
	{
		VertexSet intro; intro.insert(*redVertices.rbegin());
		this->parent()->type = Introduction;
		this->parent()->introduced = intro;
	}
}

ExtendedHypertree *ExtendedHypertree::parent() const
{
	return dynamic_cast<ExtendedHypertree *>(this->MyParent);
}

ExtendedHypertree *ExtendedHypertree::firstChild() const
{
	return dynamic_cast<ExtendedHypertree *>(*this->MyChildren.begin());
}

ExtendedHypertree *ExtendedHypertree::secondChild() const
{
	return dynamic_cast<ExtendedHypertree *>(*++this->MyChildren.begin());
}

const VertexSet &ExtendedHypertree::getVertices() const { return this->vertices; }


#ifdef DEBUG
////////// PRINTING FUNCTIONS
void ExtendedHypertree::print()
{
	ExtendedHypertree *eht = this;
        if(eht->getParent() == NULL) cout << "root: " << eht << endl;
        int type = eht->getType();

        cout << "node " << eht << ", parent = " << eht->getParent() << ", type = " << type << flush;
        if(type == Branch) cout << ", children: " << eht->firstChild() << " - " << eht->secondChild();
        else if(type != Leaf) cout << ", child: " << eht->firstChild();

        cout << ", vertices: ";
        for(VertexSet::const_iterator it = eht->getVertices().begin(); it != eht->getVertices().end(); ++it) cout << *it << ", ";
        cout << "difference: " << eht->getDifference() << ", ";
        cout << "END" << endl;

        if(type == Branch) { eht->firstChild()->print(); eht->secondChild()->print(); }
        else if(type == Leaf) {}
        else eht->firstChild()->print();
}

#endif

double ExtendedHypertree::getAverageWidth()
{
	return (double)getWidthSum()/getNumberOfDescendants();
}

unsigned int ExtendedHypertree::getWidthSum()
{
	unsigned int sum = MyChi.size();
	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		sum += ((ExtendedHypertree *)*i)->getWidthSum();
	return sum;
}

unsigned int ExtendedHypertree::getJoinNodeWidthSum()
{
	unsigned int sum = 0;
	if(getType() == Branch) sum += MyChi.size();
	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		sum += ((ExtendedHypertree *)*i)->getJoinNodeWidthSum();
	return sum;
}

double ExtendedHypertree::getAverageJoinNodeWidth()
{
	unsigned int count = getNumberOfJoinNodes();
	if(count == 0) return 0;
	return (double)getJoinNodeWidthSum()/getNumberOfJoinNodes();
}

double ExtendedHypertree::getJoinNodePercentage()
{
	return (double)getNumberOfJoinNodes()/getNumberOfDescendants();
}

unsigned int ExtendedHypertree::getNodeCountWithWidth(unsigned int width)
{
	unsigned int count = 0;
	if(MyChi.size() == width) ++count;

	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		count += ((ExtendedHypertree *)*i)->getNodeCountWithWidth(width);
	return count;
}

double ExtendedHypertree::getAverageJoinLeafDistance()
{
	return (double)getJoinLeafDistanceSum()/getNumberOfJoinNodes();
}

unsigned int ExtendedHypertree::getDistanceToNearestLeaf()
{
	if(getType() == Leaf) return 0;
	unsigned int ldist = firstChild()->getDistanceToNearestLeaf();
	if(getType() != Branch) return ldist;
	unsigned int rdist = secondChild()->getDistanceToNearestLeaf();
	return 1 + (ldist < rdist ? ldist : rdist);
}

unsigned int ExtendedHypertree::getJoinLeafDistanceSum()
{
	unsigned int sum = 0;
	if(getType() == Branch) sum += getDistanceToNearestLeaf();
	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		sum += ((ExtendedHypertree *)*i)->getJoinLeafDistanceSum();
	return sum;
}

double ExtendedHypertree::getAverageJoinJoinDistance()
{
	if(getNumberOfJoinNodes() <= 1) return 0;
	return (double)getJoinJoinDistanceSum()/(getNumberOfJoinNodes()-1);
}

unsigned int ExtendedHypertree::getDistanceToNearestJoinAncestor()
{
	ExtendedHypertree *p = parent();
	unsigned int i = 1;
	while(p) if(p->getType() == Branch) return i; else {++i; p = p->parent(); }
	return 0;
}

unsigned int ExtendedHypertree::getJoinJoinDistanceSum()
{
	unsigned int sum = 0;
	if(getType() == Branch) sum += getDistanceToNearestJoinAncestor();
	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		sum += ((ExtendedHypertree *)*i)->getJoinJoinDistanceSum();
	return sum;
}

unsigned int ExtendedHypertree::getNumberOfJoinNodes()
{
	unsigned int count = 0;
	if(getType() == Branch) ++count;

	for(list<Hypertree *>::iterator i = MyChildren.begin(); i != MyChildren.end(); ++i)
		count += (*i)->getNumberOfJoinNodes();
	return count;
}
