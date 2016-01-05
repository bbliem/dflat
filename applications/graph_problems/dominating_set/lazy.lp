%dflat: -d td --lazy --tables -e vertex -e edge --no-empty-leaves -n semi --default-join
% TODO Costs (right now D-FLAT internally does item set cardinality minimization)
#external childItem(in(X)) : childNode(N), bag(N,X).
#external childAuxItem(dominated(X)) : childNode(N), bag(N,X).
#show childItem/1. % FIXME: This should not be necessary, but otherwise childItem(_) atoms are not in clasp's symbol table and I do not know how to assign the value of external atoms otherwise.
#show childAuxItem/1.
item(in(X)) :- childItem(in(X)), not removed(X).
auxItem(dominated(X)) :- childAuxItem(dominated(X)), not removed(X).

{ item(in(X)) : introduced(X) }.
auxItem(dominated(Y)) :- item(in(X)), edge(X,Y), current(X), current(Y).
:- removed(X), not childItem(in(X)), not childAuxItem(dominated(X)).

#show item/1.
#show auxItem/1.
