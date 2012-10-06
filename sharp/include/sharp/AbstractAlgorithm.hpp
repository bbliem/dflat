#ifndef ABSTRACTALGORITHM_H_
#define ABSTRACTALGORITHM_H_

//FIXME: use unordered_set/unordered_map when c++0x is released
#include <map>
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

		// When adding this Row into the Row and it turns out there is
		// already such a Row "old" (i.e. *this == old), the old row is
		// deleted, this->unify(old) is called, and then *this is added (with
		// the corresponding Plan). In some cases it is required that the new
		// row be updated depending on the one it replaces. In these cases,
		// one is to override this method. Of course, the changes to *this
		// should leave the equality to old invariant.
		virtual void unify(const Row& old);
	};

	class Solution
	{
	public:
		virtual ~Solution();
	};

	class Plan
	{
	public:
		virtual ~Plan();

		Solution* materialize() const;

		enum Operation {
			LEAF,
			UNION,
			JOIN
		};

		Operation getOperation() const { return operation; }

	protected:
		Plan(Operation operation);

		virtual Solution* materializeLeaf() const = 0;
		virtual Solution* materializeUnion() const = 0;
		virtual Solution* materializeJoin() const = 0;

		Operation operation;
	};

	class PlanFactory
	{
	public:
		virtual ~PlanFactory();
		virtual Plan* leaf(const Row&) const = 0; // Plan for a singleton solution
		virtual Plan* unify(const Plan* left, const Plan* right) const = 0; // "Adds" solutions
		virtual Plan* join(const Row& extension, const Plan* left, const Plan* right = 0) const = 0; // "Multiplies" solutions by joining and extending them
	};

	template <typename PlanT, typename RowT = Row>
	class GenericPlanFactory : public PlanFactory
	{
	public:
		virtual Plan* leaf(const Row& row) const
		{
			return PlanT::leaf(dynamic_cast<const RowT&>(row));
		}

		virtual Plan* unify(const Plan* left, const Plan* right) const
		{
			return PlanT::unify(dynamic_cast<const PlanT*>(left), dynamic_cast<const PlanT*>(right));
		}

		virtual Plan* join(const Row& extension, const Plan* left, const Plan* right) const
		{
			return PlanT::join(dynamic_cast<const RowT&>(extension), dynamic_cast<const PlanT*>(left), dynamic_cast<const PlanT*>(right));
		}
	};


	//FIXME: use a hash_map, hash_function, etc...
	//typedef __gnu_cxx::hash_map<Row, Solution *> Table;
	typedef std::map<Row*, Plan*, std::less<Row *> > Table;

	class AbstractHTDAlgorithm
	{
	public:
		AbstractHTDAlgorithm(Problem *problem, const PlanFactory& planFactory);
		virtual ~AbstractHTDAlgorithm();
	
	private:
		Problem *prob;

	protected:
		virtual ExtendedHypertree *prepareHypertreeDecomposition(ExtendedHypertree *root);

	public:
		Plan *evaluate(ExtendedHypertree *root);
		const PlanFactory& getPlanFactory() const { return planFactory; }
		void addRowToTable(Table&, Row*, Plan*) const;
	
	protected:
		virtual Problem *problem();
		const PlanFactory& planFactory;
		virtual Plan *selectPlan(Table *rows, const ExtendedHypertree *root) = 0;
		virtual Table *evaluateNode(const ExtendedHypertree *node) = 0;
	};
	
	class AbstractSemiNormalizedHTDAlgorithm : public AbstractHTDAlgorithm
	{
	public:
		AbstractSemiNormalizedHTDAlgorithm(Problem *problem, const PlanFactory& planFactory);
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
		AbstractNormalizedHTDAlgorithm(Problem *problem, const PlanFactory& planFactory);
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
		virtual Plan *selectPlan(Table *rows, const ExtendedHypertree *root);
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
