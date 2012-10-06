#!/usr/bin/python
# Generates random input to the exchange node program
import random
import sys

if len(sys.argv) != 8:
	sys.stderr.write("Usage: " + sys.argv[0] + " currentAtoms currentClauses introducedAtoms introducedClauses removedAtoms removedClauses childTuples\n")
	sys.exit(1)

numCurrentAtoms = int(sys.argv[1])
numCurrentClauses = int(sys.argv[2])
numIntroducedAtoms = int(sys.argv[3])
numIntroducedClauses = int(sys.argv[4])
numRemovedAtoms = int(sys.argv[5])
numRemovedClauses = int(sys.argv[6])
numChildTuples = int(sys.argv[7])

bagSize = numCurrentAtoms + numCurrentClauses
numChildAtoms = numCurrentAtoms - numIntroducedAtoms + numRemovedAtoms
numChildClauses = numCurrentClauses - numIntroducedClauses + numRemovedClauses

if numIntroducedAtoms > numCurrentAtoms or numIntroducedClauses > numCurrentClauses:
	sys.stderr.write("Invalid parameters\n")
	sys.exit(2)

atoms = [({'name': "a"+str(v)}) for v in range(numCurrentAtoms + numRemovedAtoms)]
clauses = []

# Declare all atoms and clauses
for a in atoms:
	print("atom("+a['name']+").")

for i in range(numCurrentClauses + numRemovedClauses):
	clauseAtoms = random.sample(atoms, random.randint(1,numCurrentAtoms + numRemovedAtoms))
	random.shuffle(clauseAtoms)
	sep = random.randrange(len(clauseAtoms)+1)
	clause = {'name': "r"+str(i), 'pos': clauseAtoms[:sep], 'neg': clauseAtoms[sep:]}
	clauses.append(clause)
	print("clause("+clause['name']+").")
	for p in clause['pos']:
		print("pos({},{}).".format(clause['name'], p['name']))
	for n in clause['neg']:
		print("neg({},{}).".format(clause['name'], n['name']))

random.shuffle(atoms)
random.shuffle(clauses)

# Print current/1, introduced/1 and removed/1 predicates
for v in atoms[:numCurrentAtoms]:
	print("current("+v['name']+").")
for v in clauses[:numCurrentClauses]:
	print("current("+v['name']+").")
for v in atoms[:numIntroducedAtoms]:
	print("introduced("+v['name']+").")
for v in clauses[:numIntroducedClauses]:
	print("introduced("+v['name']+").")
for v in atoms[numCurrentAtoms:]:
	print("removed("+v['name']+").")
for v in clauses[numCurrentClauses:]:
	print("removed("+v['name']+").")

# Print child tuples
for m in range(numChildTuples):
	mName = "m"+str(m)
	print("childTuple("+mName+").")
	# Some clauses are already true without having been guessed to true
	for r in random.sample(clauses, random.randint(0,len(clauses))):
		print("oldMClause("+mName+","+r['name']+").")
	# Print true atoms and thereby satisfied clauses
	for a in random.sample(atoms, random.randint(0,len(atoms))):
		print("oldMAtom("+mName+","+a['name']+").")
		for r in clauses:
			if a in r['neg']:
				print("oldMClause("+mName+","+r['name']+").")

#for i in range(numClauses):
#	sys.stdout.write(":-")
#
#	atoms = random.sample(range(numVars), random.randint(1,numVars))
#
#	sep = " "
#	for j in atoms:
#		sys.stdout.write(sep)
#		if random.random() < 0.5:
#			sys.stdout.write("not ")
#		sys.stdout.write('v{}'.format(j+1))
#		sep = ", "
#	print(".")
