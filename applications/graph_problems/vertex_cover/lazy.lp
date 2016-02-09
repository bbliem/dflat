%dflat: -d td -n semi --lazy --tables --default-join -e vertex -e edge --no-empty-leaves --no-empty-root
% TODO Costs (right now D-FLAT internally does item set cardinality minimization)
#external childItem(X) : childNode(N), bag(N,X).
#show childItem/1. % FIXME: This should not be necessary, but otherwise childItem(_) atoms are not in clasp's symbol table and I do not know how to assign the value of external atoms otherwise.

item(X) :- childItem(X), current(X).
{ item(X) : introduced(X) }.
:- edge(X,Y), current(X), current(Y), not item(X), not item(Y).

#show item/1.
