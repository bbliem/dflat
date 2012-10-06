% Auxiliary predicates computed by the grounder

toOrTowards(S,X,Y) :- sub(_,S), mapped(S,X,to(Y)).
toOrTowards(S,X,Y) :- sub(_,S), mapped(S,X,towards(Y)).

% reachesViaRemoved(S,X,Y): In "subtuple" S, there is a (possibly empty) "to"/"towards" path from X to Y s.t. X and all vertices between X and Y are removed. Reflexive.
reachesViaRemoved(S,X,X) :- childTuple(T,N), childBag(N,X), removed(X), sub(T,S).
reachesViaRemoved(S,X,Z) :- sub(_,S), reachesViaRemoved(S,X,Y), removed(Y), mapped(S,Y,to(Z)).
reachesViaRemoved(S,X,Z) :- sub(_,S), reachesViaRemoved(S,X,Y), removed(Y), mapped(S,Y,towards(Z)).

% nextCurrent(S,X,Y): In "subtuple" S, Y is the next current vertex reachable from the removed X via "to" or "towards"
nextCurrent(S,X,Y) :- reachesViaRemoved(S,X,Y), current(Y).

% commonVertex(S0,S1,X): X is contained in both bags of the "subtuples" S0 and S1
commonVertex(S0,S1,X) :- sub(T0,S0), sub(T1,S1), childTuple(T0,N0), childTuple(T1,N1), childBag(N0,X), childBag(N1,X).

%%%

1 { chosenChildTuple(T) : childTuple(T,N) } 1 :- childNode(N).
1 { chosenSubTuple(S) : sub(T,S) } 1 :- chosenChildTuple(T), sub(T,_). % Last conjunct required because dummy leaf has no "subtuple"
% XXX: Is there always exactly one subtuple (except in leafs)? If so, no need for the choice.

oldTo(X,Y) :- chosenSubTuple(S), mapped(S,X,to(Y)).
oldTowards(X,Y) :- chosenSubTuple(S), mapped(S,X,towards(Y)).

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
:- chosenSubTuple(S0;S1), S0 != S1, mapped(S0,X,towards(_)), mapped(S1,X,towards(_)).

% When joining tuple T, corresponding to bag B, with tuple T', corresponding to bag B', it must hold: For each edge (x,y) in the subgraph induced by B \cap B': (x,y) is in T iff (x,y) is in T'.
:- chosenSubTuple(S0;S1), S0 != S1, commonVertex(S0,S1,X;Y), mapped(S0,X,to(Y)), not mapped(S1,X,to(Y)).

% If X is removed, it occurs only in one child (by connectedness). We require that the tuple of this child fully integrates X into a path.
:- removed(X), not hadPredecessor(X).
:- removed(X), not hadSuccessor(X).

%%%

towards(X,Y) :- oldTowards(X,Y), current(X;Y).
to(X,Y)      :- oldTo(X,Y),      current(X;Y).

towards(X,Z) :- chosenSubTuple(S), toOrTowards(S,X,Y), current(X), removed(Y), nextCurrent(S,Y,Z).

% Edges can be added to the paths because of introduced vertices or by connecting ends to starts of paths in children

{ newEdge(X,Y) : edge(X,Y) : current(X;Y) }.
to(X,Y) :- newEdge(X,Y).

% A guessed edge is invalid if the bag of a preceding child tuple contains both endpoints
:- newEdge(X,Y), chosenChildTuple(T), childTuple(T,N), childBag(N,X;Y).
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

map(1,X,to(Y)) :- to(X,Y).
map(1,X,towards(Y)) :- towards(X,Y).

% XXX: Remove requirement to assign something to each vertex
hasSuccessor(X) :- map(1,X,to(_)).
hasSuccessor(X) :- map(1,X,towards(_)).
map(1,X,null) :- current(X), not hasSuccessor(X).

%%%

map(0,X,to(Y)) :- newEdge(X,Y).
map(0,X,to(Y)) :- chosenChildTuple(T), mapped(T,X,to(Y)), current(X).
map(0,X,null) :- current(X), not hasSuccessor(X).



#hide.
#show chosenChildTuple/1.
#show map/3.
