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

	class Tuple
	{
	public:
		virtual ~Tuple();
	
		virtual bool operator<(const Tuple &other) const = 0;
		virtual bool operator==(const Tuple &other) const = 0;

		// When adding this Tuple into the TupleTable and it turns out there is
		// already such a Tuple "old" (i.e. *this == old), the old row is
		// deleted, this->unify(old) is called, and then *this is added (with
		// the corresponding Plan). In some cases it is required that the new
		// tuple be updated depending on the one it replaces. In these cases,
		// one is to override this method. Of course, the changes to *this
		// should leave the equality to old invariant.
		virtual void unify(const Tuple& old);
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
		virtual Plan* leaf(const Tuple&) const = 0; // Plan for a singleton solution
		virtual Plan* unify(const Plan* left, const Plan* right) const = 0; // "Adds" solutions
		virtual Plan* join(const Tuple& extension, const Plan* left, const Plan* right = 0) const = 0; // "Multiplies" solutions by joining and extending them
	};

	template <typename PlanT, typename TupleT = Tuple>
	class GenericPlanFactory : public PlanFactory
	{
	public:
		virtual Plan* leaf(const Tuple& tuple) const
		{
			return PlanT::leaf(dynamic_cast<const TupleT&>(tuple));
		}

		virtual Plan* unify(const Plan* left, const Plan* right) const
		{
			return PlanT::unify(dynamic_cast<const PlanT*>(left), dynamic_cast<const PlanT*>(right));
		}

		virtual Plan* join(const Tuple& extension, const Plan* left, const Plan* right) const
		{
			return PlanT::join(dynamic_cast<const TupleT&>(extension), dynamic_cast<const PlanT*>(left), dynamic_cast<const PlanT*>(right));
		}
	};


	//FIXME: use a hash_map, hash_function, etc...
	//typedef __gnu_cxx::hash_map<Tuple, Solution *> TupleTable;
	typedef std::map<Tuple*, Plan*, std::less<Tuple *> > TupleTable;

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
		void addRowToTupleTable(TupleTable&, Tuple*, Plan*) const;
	
	protected:
		virtual Problem *problem();
		const PlanFactory& planFactory;
		virtual Plan *selectPlan(TupleTable *tuples, const ExtendedHypertree *root) = 0;
		virtual TupleTable *evaluateNode(const ExtendedHypertree *node) = 0;
	};
	
	class AbstractSemiNormalizedHTDAlgorithm : public AbstractHTDAlgorithm
	{
	public:
		AbstractSemiNormalizedHTDAlgorithm(Problem *problem, const PlanFactory& planFactory);
		virtual ~AbstractSemiNormalizedHTDAlgorithm();
	
	protected:
		virtual ExtendedHypertree *prepareHypertreeDecomposition(ExtendedHypertree *root);

	protected:
		virtual TupleTable *evaluateNode(const ExtendedHypertree *node);	
		virtual TupleTable *evaluateBranchNode(const ExtendedHypertree *node) = 0;
		virtual TupleTable *evaluatePermutationNode(const ExtendedHypertree *node) = 0;
	};

	class AbstractNormalizedHTDAlgorithm : public AbstractSemiNormalizedHTDAlgorithm
	{
	public:
		AbstractNormalizedHTDAlgorithm(Problem *problem, const PlanFactory& planFactory);
		virtual ~AbstractNormalizedHTDAlgorithm();
	
	protected:
		virtual ExtendedHypertree *prepareHypertreeDecomposition(ExtendedHypertree *root);

	protected:
		virtual TupleTable *evaluatePermutationNode(const ExtendedHypertree *node);
		virtual TupleTable *evaluateIntroductionNode(const ExtendedHypertree *node) = 0;
		virtual TupleTable *evaluateRemovalNode(const ExtendedHypertree *node) = 0;
		virtual TupleTable *evaluateLeafNode(const ExtendedHypertree *node) = 0;
	};

	template<class TTuple>
	class AbstractStronglyNormalizedHTDAlgorithm: public AbstractNormalizedHTDAlgorithm
	{
	public:
		AbstractStronglyNormalizedHTDAlgorithm(Problem *problem);
		virtual ~AbstractStronglyNormalizedHTDAlgorithm();

	protected:
		virtual ExtendedHypertree *prepareHypertreeDecomposition(ExtendedHypertree *root);

	protected:
		virtual Plan *selectPlan(TupleTable *tuples, const ExtendedHypertree *root);
		virtual TupleTable *evaluateLeafNode(const ExtendedHypertree *node);
	};
} // namespace sharp

namespace std
{
	template<>
	class less<sharp::Tuple *>
	{
	public:
		bool operator()(const sharp::Tuple *t1, const sharp::Tuple *t2) { return *t1 < *t2; }
	};

} // namespace std

#endif /*ABSTRACTALGORITHM_H_*/
