#ifndef ABSTRACTALGORITHM_H_
#define ABSTRACTALGORITHM_H_

//FIXME: use unordered_set/unordered_map when c++0x is released
#include <set>
#include <vector>

#include <sharp/Global.hpp>

#include <sharp/ExtendedHypertree.hpp>



namespace sharp
{
	class Problem;

	class Row
	{
	public:
		virtual ~Row();
	
		virtual bool operator<(const Row &other) const = 0;
		virtual bool operator==(const Row &other) const = 0;

		// When adding this Row into the Table and it turns out there is
		// already such a Row "other" (i.e. *this == other), the other row is
		// deleted, this->unify(other) is called, and then *this is added. In
		// some cases it is required that the new row be updated depending on
		// the one it replaces. In these cases, one is to override this method.
		// Of course, the changes to *this should leave the equality to other
		// invariant.
		virtual void unify(Row& other);
	};

	class Solution
	{
	public:
		virtual ~Solution();
	};


	//FIXME: use a hash_map, hash_function, etc...
	//typedef __gnu_cxx::hash_map<Row, Solution *> Table;
	typedef std::set<Row*, std::less<Row *> > Table;

	class AbstractHTDAlgorithm
	{
	public:
		AbstractHTDAlgorithm(Problem *problem);
		virtual ~AbstractHTDAlgorithm();
	
	private:
		Problem *prob;

	protected:
		virtual ExtendedHypertree *prepareHypertreeDecomposition(ExtendedHypertree *root);

	public:
		Table *evaluate(ExtendedHypertree *root);
		void addRowToTable(Table&, Row*) const;
	
	protected:
		virtual Problem *problem();
		virtual Table *evaluateNode(const ExtendedHypertree *node) = 0;
	};
	
	class AbstractSemiNormalizedHTDAlgorithm : public AbstractHTDAlgorithm
	{
	public:
		AbstractSemiNormalizedHTDAlgorithm(Problem *problem);
		virtual ~AbstractSemiNormalizedHTDAlgorithm();
	
	protected:
		virtual ExtendedHypertree *prepareHypertreeDecomposition(ExtendedHypertree *root);

	protected:
		virtual Table *evaluateNode(const ExtendedHypertree *node);	
		virtual Table *evaluateBranchNode(const ExtendedHypertree *node) = 0;
		virtual Table *evaluatePermutationNode(const ExtendedHypertree *node) = 0;
	};

	class AbstractNormalizedHTDAlgorithm : public AbstractSemiNormalizedHTDAlgorithm
	{
	public:
		AbstractNormalizedHTDAlgorithm(Problem *problem);
		virtual ~AbstractNormalizedHTDAlgorithm();
	
	protected:
		virtual ExtendedHypertree *prepareHypertreeDecomposition(ExtendedHypertree *root);

	protected:
		virtual Table *evaluatePermutationNode(const ExtendedHypertree *node);
		virtual Table *evaluateIntroductionNode(const ExtendedHypertree *node) = 0;
		virtual Table *evaluateRemovalNode(const ExtendedHypertree *node) = 0;
		virtual Table *evaluateLeafNode(const ExtendedHypertree *node) = 0;
	};

	template<class TRow>
	class AbstractStronglyNormalizedHTDAlgorithm: public AbstractNormalizedHTDAlgorithm
	{
	public:
		AbstractStronglyNormalizedHTDAlgorithm(Problem *problem);
		virtual ~AbstractStronglyNormalizedHTDAlgorithm();

	protected:
		virtual ExtendedHypertree *prepareHypertreeDecomposition(ExtendedHypertree *root);

	protected:
		virtual Table *evaluateLeafNode(const ExtendedHypertree *node);
	};
} // namespace sharp

namespace std
{
	template<>
	class less<sharp::Row *>
	{
	public:
		bool operator()(const sharp::Row *r1, const sharp::Row *r2) { return *r1 < *r2; }
	};

} // namespace std

#endif /*ABSTRACTALGORITHM_H_*/
