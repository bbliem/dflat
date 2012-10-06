#!/usr/bin/python
# Generates random input to the exchange node program
import random
import sys

if len(sys.argv) != 8:
	sys.stderr.write("Usage: " + sys.argv[0] + " currentAtoms currentRules introducedAtoms introducedRules removedAtoms removedRules childTuples\n")
	sys.exit(1)

numCurrentAtoms = int(sys.argv[1])
numCurrentRules = int(sys.argv[2])
numIntroducedAtoms = int(sys.argv[3])
numIntroducedRules = int(sys.argv[4])
numRemovedAtoms = int(sys.argv[5])
numRemovedRules = int(sys.argv[6])
numChildTuples = int(sys.argv[7])

bagSize = numCurrentAtoms + numCurrentRules
numChildAtoms = numCurrentAtoms - numIntroducedAtoms + numRemovedAtoms
numChildRules = numCurrentRules - numIntroducedRules + numRemovedRules

if numIntroducedAtoms > numCurrentAtoms or numIntroducedRules > numCurrentRules:
	sys.stderr.write("Invalid parameters\n")
	sys.exit(2)

atoms = [({'name': "a"+str(v)}) for v in range(numCurrentAtoms + numRemovedAtoms)]
rules = []

# Declare all atoms and rules
for a in atoms:
	print("atom("+a['name']+").")

for i in range(numCurrentRules + numRemovedRules):
	ruleAtoms = random.sample(atoms, random.randint(1,numCurrentAtoms + numRemovedAtoms))
	random.shuffle(ruleAtoms)
	sep = random.randrange(len(ruleAtoms)+1)
	rule = {'name': "r"+str(i), 'pos': ruleAtoms[:sep], 'neg': ruleAtoms[sep:]}
	rules.append(rule)
	print("rule("+rule['name']+").")
	for p in rule['pos']:
		print("pos({},{}).".format(rule['name'], p['name']))
	for n in rule['neg']:
		print("neg({},{}).".format(rule['name'], n['name']))

random.shuffle(atoms)
random.shuffle(rules)

# Print current/1 and before/1 predicates
for v in atoms[:numChildAtoms]:
	print("before("+v['name']+").")
for v in rules[:numChildRules]:
	print("before("+v['name']+").")

for v in atoms[numRemovedAtoms:]:
	print("current("+v['name']+").")
for v in rules[numRemovedRules:]:
	print("current("+v['name']+").")

# Print child tuples
for m in range(numChildTuples):
	mName = "m"+str(m)
	print("childTuple("+mName+").")
	# Some rules are already true without having been guessed to true
	for r in random.sample(rules, random.randint(0,len(rules))):
		print("oldMRule("+mName+","+r['name']+").")
	# Print true atoms and thereby satisfied rules
	for a in random.sample(atoms, random.randint(0,len(atoms))):
		print("oldMAtom("+mName+","+a['name']+").")
		for r in rules:
			if a in r['neg']:
				print("oldMRule("+mName+","+r['name']+").")

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
