%dflat: --lazy --tables -e vertex -e edge  --no-empty-leaves --no-empty-root --no-optimization -n semi --default-join
#external childItem(map(V,C)) : childNode(N), bag(N,V), color(C).
#show childItem/1. % FIXME: This should not be necessary, but otherwise childItem(_) atoms are not in clasp's symbol table and I do not know how to assign the value of external atoms otherwise.

color(red;green;blue).

item(map(X,C)) :- childItem(map(X,C)), current(X).
% Only join matching colorings
:- item(map(V,C0)), item(map(V,C1)), C0 != C1.

1 { item(map(X,C)) : color(C) } 1 :- introduced(X).
:- edge(X,Y), item(map(X,C)), item(map(Y,C)).

#show item/1.
