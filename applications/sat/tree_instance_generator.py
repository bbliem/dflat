#!/usr/bin/python
# Generates more or less random SAT problems
import random
import sys

ratio = 1
dimacs = len(sys.argv) == 5

if len(sys.argv) not in (3, 4, 5) or (dimacs and sys.argv[4] != "--dimacs"):
	sys.stderr.write("Usage: " + sys.argv[0] + " numVars numExtraClauses [seed] [--dimacs]\n")
	sys.exit(1)

numVars = int(sys.argv[1])
numExtraClauses = int(sys.argv[2])
numClauses = numVars * ratio + numExtraClauses

if len(sys.argv) >= 4:
	random.seed(int(sys.argv[3]))

# Generate some model randomly
model = [random.randrange(2) == 0 for i in range(numVars)]

clauses = [[] for c in range(numClauses)]

for i in range(ratio * numVars):
#	# At least one var must be satisfied
#	satisfiedVar = random.randrange(numVars)
#	clauses[i].append({'atom': satisfiedVar, 'negative': not model[satisfiedVar]})
#
#	# Choose other random variables
#	for var in random.sample(range(numVars), 2):
#		negative = random.random() < 0.5
#		clauses[i].append({'atom': var, 'negative': negative})
	satisfiedVar = i % numVars
	clauses[i].append({'atom': satisfiedVar, 'negative': not model[satisfiedVar]})
	n = random.randint(2,4) # additional vars in this clause
	for var in [(i+j+1) % numVars for j in range(n)]:
		negative = random.random() < 0.5
		clauses[i].append({'atom': var, 'negative': negative})

for i in range(numExtraClauses):
	c = numVars * ratio + i
	# At least one var must be satisfied
	satisfiedVar = random.randrange(numVars)
	clauses[c].append({'atom': satisfiedVar, 'negative': not model[satisfiedVar]})

	# Choose other random variables
	for var in random.sample(range(numVars), random.randint(1,1)):
		negative = random.random() < 0.5
		clauses[c].append({'atom': var, 'negative': negative})

# Print generated instance
if dimacs:
	print("p cnf {} {}".format(numVars, numClauses))
else:
	for i in range(numClauses):
		print("clause(c{}).".format(i))
	for i in range(numVars):
		print("atom(a{}).".format(i))

for i in range(numClauses):
	c = clauses[i]
	if dimacs:
		for a in c:
			if a['negative']:
				sys.stdout.write('-')
			sys.stdout.write('{} '.format(a['atom']))
	else:
		for a in c:
			if a['negative']:
				pred = "neg"
			else:
				pred = "pos"
			print('{}(c{},a{}).'.format(pred,i,a['atom']))

if dimacs:
	print("0")

#for i in range(numClauses):
#	atoms = random.sample(range(numVars), random.randint(1,numVars))
#
#	for j in atoms:
#		negative = random.random() < negprob
#		if dimacs:
#			if negative:
#				sys.stdout.write("-")
#			sys.stdout.write('{} '.format(j+1))
#		else:
#			if negative:
#				pred = "neg"
#			else:
#				pred = "pos"
#			print('{}(c{},a{}).'.format(pred,i,j))
#	if dimacs:
#		print("0")
