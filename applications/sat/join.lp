1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(A;B), childItem(A,I), not childItem(B,I), atom(I).
item(I) :- extend(R), childItem(R,I).
auxItem(I) :- extend(R), childAuxItem(R,I).

#hide.
#show item/1.
#show auxItem/1.
#show extend/1.
