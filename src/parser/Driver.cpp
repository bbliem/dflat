/*{{{
Copyright 2012-2016, Bernhard Bliem
WWW: <http://dbai.tuwien.ac.at/research/project/dflat/>.

This file is part of D-FLAT.

D-FLAT is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

D-FLAT is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with D-FLAT.  If not, see <http://www.gnu.org/licenses/>.
*/
//}}}
#include <cassert>
#include <stdexcept>

#include "Driver.h"
#include "../Instance.h"
#include "../util.h"

namespace parser {

Driver::Driver(std::istream& input)
	: input(input)
	, seenNumVertices(false)
{
}

Driver::~Driver()
{
}

Instance Driver::parse()
{
	Instance instance;
	Scanner scanner(&input);
	Parser parser(scanner, *this, instance);
	int res = parser.parse();
	if(res != 0)
		throw std::runtime_error("Parse error");
	return instance;
}

void Driver::processFact(Instance& instance, const std::string& predicate, const std::vector<std::string>& arguments)
{
	if(predicate == "numVertices") {
		if(arguments.size() != 1)
			throw std::runtime_error("numVertices predicate must be unary");
		const int numVertices = util::strToInt(arguments[0], "Invalid number of vertices");
		std::vector<unsigned> vertexNames;
		vertexNames.reserve(numVertices);
		for(int i = 1; i <= numVertices; ++i)
			vertexNames.push_back(i);
		instance.setVertexNames(std::move(vertexNames));
		seenNumVertices = true;
	}
	else
	{
		if(!seenNumVertices)
			throw std::runtime_error("First fact must be over predicate numVertices/1");

		if(predicate == "edge") {
			if(arguments.size() != 3)
				throw std::runtime_error("edge predicate must be ternary");
			const int x = util::strToInt(arguments[0], "Invalid vertex") - 1;
			const int y = util::strToInt(arguments[1], "Invalid vertex") - 1;
			const int w = util::strToInt(arguments[2], "Invalid weight");
			instance.addEdge(x, y, w);
		}
		else if(predicate == "terminal") {
			if(arguments.size() != 1)
				throw std::runtime_error("terminal predicate must be unary");
			instance.setTerminal(util::strToInt(arguments[0], "Invalid terminal") - 1);
		}
		else {
			std::ostringstream msg;
			msg << "Unexpected predicate " << predicate;
			throw std::runtime_error(msg.str());
		}
	}
}

} // namespace parser
