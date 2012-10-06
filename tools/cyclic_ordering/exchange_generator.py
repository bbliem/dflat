#!/usr/bin/python
# Generates random input to the exchange node program
import random
import sys

if len(sys.argv) != 6:
	sys.stderr.write("Usage: " + sys.argv[0] + " current introduced removed childTuples numOrderings\n")
	sys.exit(1)

numCurrent = int(sys.argv[1])
numIntroduced = int(sys.argv[2])
numRemoved = int(sys.argv[3])
numChildTuples = int(sys.argv[4])
numOrderings = int(sys.argv[5])

if numIntroduced > numCurrent:
	sys.stderr.write("Invalid parameters\n")
	sys.exit(2)

elements = [("v"+str(v)) for v in range(numCurrent + numRemoved)]

# Print current/1, introduced/1 and removed/1 predicates
for v in elements[:numCurrent]:
	print("current("+v+").")
for v in elements[:numIntroduced]:
	print("introduced("+v+").")
for v in elements[numCurrent:]:
	print("removed("+v+").")

# Print orderings
for i in range(numOrderings):
	o = random.sample(elements, 3)
	print("order_({},{},{}).".format(o[0], o[1], o[2]))

elementsBefore = elements[numIntroduced:]

# Print child tuples
for c in range(numChildTuples):
	childTupleName = "c"+str(c)
	print("childTuple_("+childTupleName+").")

	random.shuffle(elementsBefore)
	predecessor = elementsBefore[0]
	for v in elementsBefore[1:]:
		print("succ_({},{},{}).".format(childTupleName, predecessor, v))
		predecessor = v
