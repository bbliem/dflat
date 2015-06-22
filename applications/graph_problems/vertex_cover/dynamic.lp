%dflat: -d td --no-empty-leaves --no-empty-root --tables -e vertex -e edge
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(A), extend(B), childItem(A,X), childRow(B,N), childNode(N), bag(N,X), not childItem(B,X).

item(X) :- extend(R), childItem(R,X), current(X).

{ item(X) : introduced(X) }.
:- edge(X,Y), current(X), current(Y), not item(X), not item(Y).

% If a vertex is in more than one child node, it must be in the current node (connectedness).
% If we add the costs of all predecessors, that which we count multiple times is due to vertices which are also in the current node.
% Thus, add costs of all predecessors and subtract for each current vertex how often it was counted.

currentCost(C) :- C = #count { X : item(X) }.
sumCosts(S) :- S = #sum { C,R : extend(R), childCost(R,C) }.

inExtendedRow(V,R) :- current(V), extend(R), childItem(R,V).
numCounted(V,N) :- current(V), N = #count { R : inExtendedRow(V,R) }.
subtract(C) :- C = #sum { N,V : numCounted(V,N) }.

cost(Sum - Sub + Cur) :- currentCost(Cur), sumCosts(Sum), subtract(Sub).



#show currentCost/1.
#show extend/1.
#show cost/1.
#show item/1.
