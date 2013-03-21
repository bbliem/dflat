1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(R;S), childItem(R,pos(I,V)), vertex(V), not childItem(S,pos(I,V)).
:- extend(R;S), R < S, childItem(R;S,pos(I,_nn)).
item(I) :- extend(R), childItem(R,I).

#hide.
#show extend/1.
#show item/1.
