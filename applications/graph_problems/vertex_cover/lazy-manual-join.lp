%dflat: -d td -n semi --lazy --tables -e vertex -e edge --no-empty-leaves --no-empty-root
% TODO Costs (right now D-FLAT internally does item set cardinality minimization)
#external childItem(in(X)) : childNode(N), bag(N,X).
#external childItem(out(X)) : childNode(N), bag(N,X).
#external childItem(X) : childNode(N), bag(N,X).
#show childItem/1. % FIXME: This should not be necessary, but otherwise childItem(_) atoms are not in clasp's symbol table and I do not know how to assign the value of external atoms otherwise.

% Join condition
:- childItem(in(X)), childItem(out(X)).

item(in(X)) :- childItem(in(X)), current(X).
item(out(X)) :- childItem(out(X)), current(X).
1 { item(in(X)); item(out(X)) } 1 :- introduced(X).
:- edge(X,Y), current(X), current(Y), not item(in(X)), not item(in(Y)).

% Hack to make optimization work
item(X) :- item(in(X)).

#show item/1.
