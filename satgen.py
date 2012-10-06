#!/usr/bin/python
# Generates more or less random SAT problems
import random
import sys

if len(sys.argv) not in (4, 5):
	sys.stderr.write("Usage: " + sys.argv[0] + " numClauses numVars 'lp'|'dimacs' [seed]\n")
	sys.exit(1)

numClauses = int(sys.argv[1])
numVars = int(sys.argv[2])
outFormat = sys.argv[3]

if len(sys.argv) == 5:
	random.seed(int(sys.argv[4]))

if outFormat == "dimacs":
	print("p cnf {} {}".format(numVars, numClauses))
	for i in range(numClauses):
		atoms = random.sample(range(numVars), random.randint(1,numVars))

		for j in atoms:
			if random.random() >= 0.5:
				sys.stdout.write("-")
			sys.stdout.write('{} '.format(j+1))
		print("0")
elif outFormat == "lp":
	for i in range(numClauses):
		sys.stdout.write(":-")

		atoms = random.sample(range(numVars), random.randint(1,numVars))

		sep = " "
		for j in atoms:
			sys.stdout.write(sep)
			if random.random() < 0.5:
				sys.stdout.write("not ")
			sys.stdout.write('v{}'.format(j+1))
			sep = ", "
		print(".")
else:
	sys.stderr.write("Invalid output format\n")
	sys.exit(2)
