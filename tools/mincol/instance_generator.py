#!/usr/bin/python
import random
import sys
import os

if len(sys.argv) not in (3, 4):
	sys.stderr.write("Usage: " + sys.argv[0] + " numNodes numEdges [seed]\n")
	sys.exit(1)

numNodes = int(sys.argv[1])
numEdges = int(sys.argv[2])

if len(sys.argv) == 4:
	seed = int(sys.argv[3])
	random.seed(seed)
else:
	seed = ''

ret = os.system('tools/graphs/instance_generator.py {} {} {}'.format(numNodes, numEdges, seed))
ret == 0 or sys.exit(ret >> 8)

for i in random.sample(range(numNodes), random.randrange(numNodes+1)):
	print('criticalVertex({}).'.format(i))
