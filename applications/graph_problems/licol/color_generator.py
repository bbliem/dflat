#!/usr/bin/python
# Produce a random list of permitted colors for each vertex.
# Use in conjunction with a graph generator.
import random
import sys

if len(sys.argv) not in (3, 4):
	sys.stderr.write("Usage: " + sys.argv[0] + " numVertices maxColors [seed]\n")
	sys.exit(1)

numVertices = int(sys.argv[1])
maxColors = int(sys.argv[2])

colors = range(maxColors)

if len(sys.argv) == 4:
	random.seed(int(sys.argv[3]))

for i in range(numVertices):
	numColors = 1 + random.randrange(maxColors)
	for c in random.sample(colors,numColors):
		print('color({},{}).'.format(i,c))
