#!/usr/bin/python
import random
import sys

if len(sys.argv) not in (3,4):
	sys.stderr.write("Usage: " + sys.argv[0] + " n o [seed]\nn = number of vertices = number of edges + 1; o = number of random edges\n")
	sys.exit(1)

def edge(a,b):
	print("edge(v{},v{}).".format(a,b))

n = int(sys.argv[1])
o = int(sys.argv[2])

if len(sys.argv) == 4:
	random.seed(int(sys.argv[3]))

for i in range(n):
	print("vertex(v{}).".format(i))

for i in range(n-1):
	edge(i, i+1)

for i in range(o):
	l = random.sample(range(n), 2)
	edge(l[0], l[1])
