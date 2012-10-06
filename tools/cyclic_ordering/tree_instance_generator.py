#!/usr/bin/python
import random
import sys

if len(sys.argv) not in (3,4):
	sys.stderr.write("Usage: " + sys.argv[0] + " n o [seed]\n")
	sys.exit(1)

n = int(sys.argv[1])
o = int(sys.argv[2])

if len(sys.argv) == 4:
	random.seed(int(sys.argv[3]))

def order(a,b,c):
	print("order(v{},v{},v{}).".format(a,b,c))

order(0,1,2)

for i in range(1,n):
	order(i, i+1, i+2)

for i in range(o):
	l = random.sample(range(n+2), 3)
	order(l[0], l[1], l[2])
