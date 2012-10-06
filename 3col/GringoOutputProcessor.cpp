#include "GringoOutputProcessor.h"

namespace threeCol {

void GringoOutputProcessor::printSymbolTableEntry(const AtomRef &atom, uint32_t arity, const std::string &name)
{
	::GringoOutputProcessor::printSymbolTableEntry(atom, arity, name);

	if(arity == 0)
		return;

	std::stringstream arg; // The first argument
	ValVec::const_iterator k = vals_.begin() + atom.second;
	k->print(s_, arg);

	if(name == "r") // FIXME: I'm dirty
		r.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first)); // +1 because of the leading "m" of the argument
	else if(name == "g")
		g.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first));
	else if(name == "b")
		b.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first));
	else if(name == "chosenChildTuple")
		chosenChildTuple.push_back(LongAndSymbolTableKey(std::strtol(arg.str().c_str()+1, 0, 0), atom.first));
}

} // namespace threeCol
