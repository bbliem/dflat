#!/usr/bin/python
import random
import sys

if len(sys.argv) not in (3, 4):
	sys.stderr.write("Usage: " + sys.argv[0] + " numElements numOrderings [seed]\n")
	sys.exit(1)

n = int(sys.argv[1])
o = int(sys.argv[2])

if len(sys.argv) == 4:
	random.seed(int(sys.argv[3]))

if o < n:
	sys.stderr.write("Must have at least as many orderings as elements.\n")
	sys.exit(2)

#n = 12
# = 18

for i in range(n):
	print("vertex(v{}).".format(i))

for i in range(n):
	l = random.sample(range(n-1), 2)
	if l[0] >= i:
		l[0] = l[0] + 1
	if l[1] >= i:
		l[1] = l[1] + 1
	print("order(v{},v{},v{}).".format(i, l[0], l[1]))

for i in range(o-n):
	l = random.sample(range(n), 3)
	print("order(v{},v{},v{}).".format(l[0], l[1], l[2]))
