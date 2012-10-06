1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(A;B), childItem(A,X), childRow(B,N), childBag(N,X), not childItem(B,X).

item(X) :- extend(R), childItem(R,X), current(X).

{ item(X) : introduced(X) }.
:- edge(X,Y), current(X;Y), not item(X;Y).

% If a vertex is in more than one child node, it must be in the current node (connectedness).
% If we add the costs of all predecessors, that which we count multiple times is due to vertices which are also in the current node.
% Thus, add costs of all predecessors and subtract for each current vertex how often it was counted.

currentCost(C) :- C = #count { item(_) }.
sumCosts(S) :- S = #sum [ extend(R) : childCost(R,C) = C ].

inExtendedRow(V,R) :- current(V), extend(R), childItem(R,V).
numCounted(V,N) :- current(V), N = #count { inExtendedRow(V,_) }.
subtract(C) :- C = #sum [ numCounted(_,N) = N ].

cost(Sum - Sub + Cur) :- currentCost(Cur), sumCosts(Sum), subtract(Sub).

% XXX: These aggregates are killing us...


#hide.
#show extend/1.
#show cost/1.
#show item/1.
