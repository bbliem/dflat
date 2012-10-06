#include "GringoOutputProcessor.h"

namespace sat {

void GringoOutputProcessor::printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	::GringoOutputProcessor::printSymbolTableEntry(atom, arity, name);

	if(arity == 0)
		return;

	std::stringstream arg; // The first argument
	ValVec::const_iterator k = vals_.begin() + atom.second;
	k->print(s_, arg);

	if(name == "mAtom") // FIXME: I'm dirty
		mAtom.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first)); // +1 because of the leading "m" of the argument
	else if(name == "mClause")
		mClause.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first));
	else if(name == "chosenChildTuple")
		chosenChildTuple.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first));
}

} // namespace sat
