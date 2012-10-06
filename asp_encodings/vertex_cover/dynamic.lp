1 { chosenChildTuple(T) : childTuple(T,N) } 1 :- childNode(N).
:- chosenChildTuple(A), chosenChildTuple(B), mapped(A,X,VA), mapped(B,X,VB), VA != VB.

map(X,Y) :- chosenChildTuple(I), mapped(I,X,Y), current(X).

1 { in(X), out(X) } 1 :- introduced(X).
map(X,in) :- in(X).
map(X,out) :- out(X).
:- edge(X,Y), map(X,out), map(Y,out).

% If a vertex is in more than one child node, it must be in the current node (connectedness).
% If we add the costs of all predecessors, that which we count multiple times is due to vertices which are also in the current node.
% Thus, add costs of all predecessors and subtract for each current vertex how often it was counted.

currentCost(C) :- C = #count { map(_,in) }.
sumCosts(S) :- S = #sum [ chosenChildTuple(I) : childCost(I,C) = C ].

inChosenChild(V,I) :- current(V), chosenChildTuple(I), mapped(I,V,in).
numCounted(V,N) :- current(V), N = #count { inChosenChild(V,_) }.
subtract(C) :- C = #sum [ numCounted(_,N) = N ].

cost(Sum - Sub + Cur) :- currentCost(Cur), sumCosts(Sum), subtract(Sub).

% XXX: These aggregates are killing us...


#hide.
#show chosenChildTuple/1.
#show cost/1.
#show map/2.
