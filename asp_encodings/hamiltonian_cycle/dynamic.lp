1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

:- extend(R;S), childItem(R,path(X,Y)), childItem(S,path(X,Z)), Y != Z.

% If two different preceding rows have a non-direct path from X to Y, the immediate successors of X are different.
:- extend(R;S), R != S, childItem(R;S,path(X,Y)), not childItem(R;S,to(X,Y)).

% When joining row R, corresponding to bag B, with row S, corresponding to bag B', it must hold: For each edge (x,y) in the subgraph induced by B \cap B': (x,y) is in R iff (x,y) is in S.
% Because of connectedness, this is equivalent to: For each child row and each pair of vertices in the current bag and the bag of the child row: There is an edge between these vertices in the current row iff this edge is also in the child row.
:- item(to(X,Y)), extend(R), childRow(R,N), childBag(N,X;Y), not childItem(R,to(X,Y)).

% If X is removed, it occurs only in one child (by connectedness). We require that the row of this child fully integrates X into a path.
integrated(R,X) :- childItem(R,path(X,_)), childItem(R,path(_,X)).
:- extend(R), childRow(R,N), childBag(N,X), removed(X), not integrated(R,X).

reachesViaRemoved(R,X,X) :- childRow(R,N), childBag(N,X).
reachesViaRemoved(R,X,Z) :- reachesViaRemoved(R,X,Y), removed(Y), childItem(R,path(Y,Z)).
item(path(X,Z)) :- extend(R), childItem(R,path(X,Y)), reachesViaRemoved(R,Y,Z), current(X;Z).
item(to(X,Y)) :- extend(R), childItem(R,to(X,Y)), current(X;Y).

% Edges can be added to the paths because of introduced vertices or by connecting ends to starts of paths in children
{ item(to(X,Y)) : edge(X,Y) : current(X;Y) }.
item(path(X,Y)) :- item(to(X,Y)).

% A guessed edge is invalid if it would give a vertex more than one successor or predecessor.
:- item(path(X,Y;Z)), Y != Z.
:- item(path(X;Y,Z)), X != Y.
path(X,Y) :- item(path(X,Y)).
path(X,Z) :- path(X,Y), item(path(Y,Z)).
% Disallow cycles not encompassing all current vertices
:- path(X,X), current(Y), not path(X,Y).



#hide.
#show extend/1.
#show item/1.
