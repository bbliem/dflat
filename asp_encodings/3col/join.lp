% This is just for demonstration. Use the default implementation instead.
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(A;B), childItem(A,I), not childItem(B,I).
item(I) :- extend(R), childItem(R,I).

#hide.
#show item/1.
#show extend/1.
