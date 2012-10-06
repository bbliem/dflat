#include "GringoOutputProcessor.h"

namespace asdp {

GringoOutputProcessor::GringoOutputProcessor(const sharp::Problem& problem)
	: problem(problem)
{
}

void GringoOutputProcessor::printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	::GringoOutputProcessor::printSymbolTableEntry(atom, arity, name);

	if(arity == 3 && name == "map") {
		std::stringstream firstArg; // First argument
		std::ostringstream args[2]; // The last two arguments
		ValVec::const_iterator k = vals_.begin() + atom.second;
		(k++)->print(s_, firstArg);
		(k++)->print(s_, args[0]);
		k->print(s_, args[1]);

		unsigned int firstArgNum;
		firstArg >> firstArgNum;

		mapAtoms.push_back(MapAtom(firstArgNum, args[0].str(), args[1].str(), atom.first));
	}
	else if(name == "chosenChildTuple") {
		assert(arity == 1);
		std::stringstream firstArg; // First argument
		ValVec::const_iterator k = vals_.begin() + atom.second;
		k->print(s_, firstArg);
		chosenChildTupleAtoms[std::strtol(firstArg.str().c_str()+1, 0, 0)] = atom.first;
	}
}

} // namespace asdp
