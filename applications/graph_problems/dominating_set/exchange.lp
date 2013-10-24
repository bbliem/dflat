1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(in(X)) :- extend(R), childItem(R,in(X)), not removed(X).
item(dominated(X)) :- extend(R), childItem(R,dominated(X)), not removed(X).

{ item(in(X)) : introduced(X) }.
item(dominated(Y)) :- item(in(X)), edge(X,Y), current(X;Y).
:- removed(X), extend(R), not childItem(R,in(X)), not childItem(R,dominated(X)).

cost(CC + IC) :- extend(R), childCost(R,CC), IC = #count{ item(in(X)) : introduced(X) }.

#hide.
#show extend/1.
#show item/1.
#show cost/1.
