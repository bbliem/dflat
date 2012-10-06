levels(2).

% Auxiliary predicates computed by the grounder

toOrTowards(S,X,Y) :- sub(_,S), childItem(S,to(X,Y)).
toOrTowards(S,X,Y) :- sub(_,S), childItem(S,towards(X,Y)).

% reachesViaRemoved(S,X,Y): In "subtuple" S, there is a (possibly empty) "to"/"towards" path from X to Y s.t. X and all vertices between X and Y are removed. Reflexive.
reachesViaRemoved(S,X,X) :- childRow(R,N), childBag(N,X), removed(X), sub(R,S).
reachesViaRemoved(S,X,Z) :- sub(_,S), reachesViaRemoved(S,X,Y), removed(Y), childItem(S,to(Y,Z)).
reachesViaRemoved(S,X,Z) :- sub(_,S), reachesViaRemoved(S,X,Y), removed(Y), childItem(S,towards(Y,Z)).

% nextCurrent(S,X,Y): In "subtuple" S, Y is the next current vertex reachable from the removed X via "to" or "towards"
nextCurrent(S,X,Y) :- reachesViaRemoved(S,X,Y), current(Y).

%%%

1 { extend(0,R) : childRow(R,N) } 1 :- childNode(N).
1 { extend(1,S) : sub(R,S) } 1 :- extend(0,R), sub(R,_). % Last conjunct required because dummy leaf has no "subtuple"
% XXX: Is there always exactly one subtuple (except in leafs)? If so, no need for the choice.

oldTo(X,Y) :- extend(1,S), childItem(S,to(X,Y)).
oldTowards(X,Y) :- extend(1,S), childItem(S,towards(X,Y)).

hadSuccessor(X) :- oldTo(X,_).
hadSuccessor(X) :- oldTowards(X,_).
hadPredecessor(X) :- oldTo(_,X).
hadPredecessor(X) :- oldTowards(_,X).

%%%

:- oldTo(X,Y;Z), Y != Z.
:- oldTo(X;Y,Z), X != Y.

:- oldTo(X,_), oldTowards(X,_).
:- oldTo(_,X), oldTowards(_,X).

% If two different preceding tuples have a towards-edge starting at X, the immediate successors of X are different.
:- extend(1,S0;S1), S0 != S1, childItem(S0,towards(X,_)), childItem(S1,towards(X,_)).

% When joining tuple T, corresponding to bag B, with tuple T', corresponding to bag B', it must hold: For each edge (x,y) in the subgraph induced by B \cap B': (x,y) is in T iff (x,y) is in T'.
% Because of connectedness, this is equivalent to: For each child tuple and each pair of vertices in the current bag and the bag of the child tuple: There is an edge between these vertices in the current tuple iff this edge is also in the child tuple.
:- item(1,to(X,Y)), extend(0,R), childRow(R,N), childBag(N,X;Y), not childItem(R,to(X,Y)).

% If X is removed, it occurs only in one child (by connectedness). We require that the tuple of this child fully integrates X into a path.
:- removed(X), not hadPredecessor(X).
:- removed(X), not hadSuccessor(X).

%%%

towards(X,Y) :- oldTowards(X,Y), current(X;Y).
to(X,Y)      :- oldTo(X,Y),      current(X;Y).

towards(X,Z) :- extend(1,S), toOrTowards(S,X,Y), current(X), removed(Y), nextCurrent(S,Y,Z).

% Edges can be added to the paths because of introduced vertices or by connecting ends to starts of paths in children

{ newEdge(X,Y) : edge(X,Y) : current(X;Y) }.
to(X,Y) :- newEdge(X,Y).

% A guessed edge is invalid if the bag of a preceding child tuple contains both endpoints
:- newEdge(X,Y), extend(0,R), childRow(R,N), childBag(N,X;Y).
% ... or if it would give a vertex more than one successor or predecessor.
e(X,Y) :- to(X,Y).
e(X,Y) :- towards(X,Y).
:- e(X,Y;Z), Y != Z.
:- e(X;Y,Z), X != Y.
eTrans(X,Y) :- e(X,Y).
eTrans(X,Z) :- eTrans(X,Y), e(Y,Z).
% Disallow cycles not encompassing all current vertices
:- eTrans(X,X), current(Y), not eTrans(X,Y).

%%%

item(1,to(X,Y)) :- to(X,Y).
item(1,towards(X,Y)) :- towards(X,Y).

%%%

item(0,to(X,Y)) :- newEdge(X,Y).
item(0,to(X,Y)) :- extend(0,R), childItem(R,to(X,Y)), current(X).



#hide.
#show extend/2.
#show item/2.
#show levels/1.
