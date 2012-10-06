#!/usr/bin/python
# Generates random input to the exchange node program
import random
import sys

if len(sys.argv) != 5:
	sys.stderr.write("Usage: " + sys.argv[0] + " bagSize childBagSize numIntroduced numChildTuples\n")
	sys.exit(1)

bagSize = int(sys.argv[1])
childBagSize = int(sys.argv[2])
numIntroduced = int(sys.argv[3])
numChildTuples = int(sys.argv[4])
numCommon = bagSize - numIntroduced
numRemoved = childBagSize - numCommon
numTotal = bagSize + numRemoved
numRules = random.randrange(numTotal)
numAtoms = numTotal - numRules

#print(numCommon)
#print(numRemoved)
#print(numTotal)
#print(numRules)
#print(numAtoms)

if numIntroduced > bagSize or numRemoved > childBagSize or numCommon > bagSize or numCommon > childBagSize:
	sys.stderr.write("Invalid parameters\n")
	sys.exit(2)

atoms = [({'name': "a"+str(v)}) for v in range(numAtoms)]
rules = []

for a in atoms:
	print("atom("+a['name']+").")

for i in range(numRules):
	ruleAtoms = random.sample(atoms, random.randint(1,numAtoms))
	random.shuffle(ruleAtoms)
	sep = random.randrange(len(ruleAtoms)+1)
	rule = {'name': "r"+str(i), 'pos': ruleAtoms[:sep], 'neg': ruleAtoms[sep:]}
	rules.append(rule)
	print("rule("+rule['name']+").")
	for p in rule['pos']:
		print("pos({},{}).".format(rule['name'], p['name']))
	for n in rule['neg']:
		print("neg({},{}).".format(rule['name'], n['name']))

vertices = atoms + rules
random.shuffle(vertices)

# Print current/1 and before/1 predicates
for v in vertices[:childBagSize]:
	print("before("+v['name']+").")
sep = childBagSize-numCommon
for v in vertices[sep:sep+bagSize]:
	print("current("+v['name']+").")

# Print child tuples
#random.shuffle(atoms)
#random.shuffle(rules)
for m in range(numChildTuples):
	mName = "m"+str(m)
	print("oldMi("+mName+").")
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
