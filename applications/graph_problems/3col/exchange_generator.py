#!/usr/bin/python
# Generates random input to the exchange node program
import random
import sys

if len(sys.argv) != 6:
	sys.stderr.write("Usage: " + sys.argv[0] + " current introduced removed childTuples probEdge\n")
	sys.exit(1)

numCurrent = int(sys.argv[1])
numIntroduced = int(sys.argv[2])
numRemoved = int(sys.argv[3])
numChildTuples = int(sys.argv[4])
probEdge = float(sys.argv[5])

if numIntroduced > numCurrent:
	sys.stderr.write("Invalid parameters\n")
	sys.exit(2)

nodes = [("n"+str(v)) for v in range(numCurrent + numRemoved)]

# Print current/1, introduced/1 and removed/1 predicates
for v in nodes[:numCurrent]:
	print("current("+v+").")
for v in nodes[:numIntroduced]:
	print("introduced("+v+").")
for v in nodes[numCurrent:]:
	print("removed("+v+").")

# Print edges
for i1 in range(len(nodes)):
	for i2 in range(i1+1,len(nodes)):
		if random.random() < probEdge:
			print("edge("+nodes[i1]+","+nodes[i2]+").")

nodesBefore = nodes[numIntroduced:]

# Print child tuples
for c in range(numChildTuples):
	childTupleName = "c"+str(c)
	print("childTuple("+childTupleName+").")

	random.shuffle(nodesBefore)
	x = random.randrange(numCurrent - numIntroduced)
	y = random.randrange(x, numCurrent - numIntroduced)
	# Let nodes[:x] be red, nodes[x:y] be green and nodes[y:numCurrent-numIntroduced] be blue
	for r in nodesBefore[:x]:
		print("oldR("+childTupleName+","+r+").")
	for g in nodesBefore[x:y]:
		print("oldG("+childTupleName+","+g+").")
	for b in nodesBefore[y:numCurrent-numIntroduced]:
		print("oldB("+childTupleName+","+b+").")
