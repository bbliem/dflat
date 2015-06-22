%dflat: --tables -e vertex -e edge --no-empty-leaves --no-empty-root
color(red;green;blue).

1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(map(X,C)) :- extend(R), childItem(R,map(X,C)), current(X).
% Only join matching colorings
:- item(map(V,C0)), item(map(V,C1)), C0 != C1.

1 { item(map(X,C)) : color(C) } 1 :- introduced(X).
:- edge(X,Y), item(map(X,C)), item(map(Y,C)).



#show item/1.
#show extend/1.
