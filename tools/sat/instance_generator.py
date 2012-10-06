#!/usr/bin/python
# Generates more or less random SAT problems
import random
import sys

dimacs = len(sys.argv) == 5

if len(sys.argv) not in (3, 4, 5) or (dimacs and sys.argv[4] != "--dimacs"):
	sys.stderr.write("Usage: " + sys.argv[0] + " numClauses numVars [seed] [--dimacs]\n")
	sys.exit(1)

numClauses = int(sys.argv[1])
numVars = int(sys.argv[2])

if len(sys.argv) >= 4:
	random.seed(int(sys.argv[3]))

if dimacs:
	print("p cnf {} {}".format(numVars, numClauses))

for i in range(numClauses):
	atoms = random.sample(range(numVars), random.randint(1,numVars))

	for j in atoms:
		negative = random.random() < 0.5
		if dimacs:
			if negative:
				sys.stdout.write("-")
			sys.stdout.write('{} '.format(j+1))
		else:
			if negative:
				pred = "neg"
			else:
				pred = "pos"
			print('{}(c{},a{}).'.format(pred,i,j))
	if dimacs:
		print("0")
