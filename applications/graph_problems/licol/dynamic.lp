%dflat: -d td --no-empty-leaves --no-empty-root --tables -e vertex -e edge
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

item(map(X,C)) :- extend(R), childItem(R,map(X,C)), current(X).
1 { item(map(X,C)) : color(X,C) } 1 :- introduced(X).
:- item(map(V,C0)), item(map(V,C1)), C0 < C1.

:- edge(X,Y), item(map(X,C)), item(map(Y,C)).

#show item/1.
#show extend/1.
