#!/usr/bin/python
# Generates more or less random SAT problems
import random
import sys

def usage():
	sys.stderr.write('args: ' + str(sys.argv) + '\n')
	sys.stderr.write("Usage: " + sys.argv[0] + " numVars numExtraClauses [seed] [--allow-unsat] [--dimacs]\n")
	sys.stderr.write("If you provide --allow-unsat, adding the extra clauses may result in an unsatisfiable instance.\n")
	sys.exit(1)

ratio = 1
allowUnsat = False
dimacs = False

if len(sys.argv) not in range(3,6):
	usage()

numVars = int(sys.argv[1])
numExtraClauses = int(sys.argv[2])
numClauses = numVars * ratio + numExtraClauses

for arg in sys.argv[3:]:
	if arg.isdigit():
		random.seed(int(arg))
	elif arg == '--dimacs':
		dimacs = True
	elif arg == '--allow-unsat':
		allowUnsat = True
	else:
		usage()

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
	numVarsInThisClause = 2

	if not allowUnsat:
		# At least one var must be satisfied
		satisfiedVar = random.randrange(numVars)
		clauses[c].append({'atom': satisfiedVar, 'negative': not model[satisfiedVar]})
		numVarsInThisClause -= 1

	# Choose other random variables
	for var in random.sample(range(numVars), numVarsInThisClause):
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
