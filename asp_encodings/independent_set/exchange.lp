1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(in(X)) :- extend(R), childItem(R,in(X)), not removed(X).

{ item(in(X)) : introduced(X) }.
:- edge(X,Y), item(in(X;Y)).

% We use negative costs since this is a maximization problem and D-FLAT currently only minimizes.
cost(CC - IC) :- extend(R), childCost(R,CC), IC = #count{ item(in(X)) : introduced(X) }.

#hide.
#show extend/1.
#show item/1.
#show cost/1.
