%dflat: -e vertex -e edge --tables -d td -n semi --no-empty-leaves --no-empty-root
% edges are undirected
edge(X,Y) :- edge(Y,X).

% propagate in nodes and known paths
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(in(X)) :- extend(R), childItem(R,in(X)), current(X).
item(path(X,Y)) :- extend(R), childItem(R, path(X,Y)), current(X), current(Y).

% propagate "in" counter
% (increase, if introduced vertices are guessed "in")
% (subtract 1 for each vertex that has been guessed "in" in multiple subtrees - otherwise it is counted twice)
subCounted(X,R1) :- extend(R1), extend(R2), childItem(R1,in(X)), childItem(R2,in(X)), R1 < R2.
subCount(SC) :- SC = #count{X,R1 : subCounted(X,R1)}.
item(inCount(SUM+C-SC)) :- SUM = #sum{COUNT,R : extend(R), childItem(R,inCount(COUNT))}, localInCount(C), subCount(SC).

% prevent joins where a current vertex is only "in" in one child node
:- extend(R1), extend(R2), R1 < R2, current(X), childItem(R1, in(X)), not childItem(R2, in(X)).

% count the vertices that have been introduced and which are guessed into the remove partition
localInCount(C) :- C = #count {X : item(in(X)), introduced(X)}.

% guess for introduced vertices if they are in
0 {item(in(X))} 1 :- introduced(X).

% infer new paths for current in vertices
item(path(X,Y)) :- edge(X,Y), item(in(X)), item(in(Y)), X != Y.
item(path(X,Z)) :- item(path(X,Y)), item(path(Y,Z)), item(in(X)), item(in(Y)), item(in(Z)), X != Y, Y != Z.
item(path(X,Y)) :- item(path(Y,X)).

% infer toConnect(X,Y), if two vertices X and Y are both in and not connected
item(toConnect(X,Y)) :- current(X), item(in(X)), item(in(Y)), not item(path(X,Y)), X != Y.

% kill partial solutions in which the last previously guessed "in" vertex has been forgotten and only newly introduced vertices are guessed "in" -> a new connected component is started
:- introduced(X), extend(R), CIN = #count {Y : childItem(in(Y))}, IN = #count {Y : item(in(Y))}, ININ = #count {Y : item(in(Y)), introduced(Y)}, IN-CIN = ININ.
% kill partial solutions in which for two current vertices with a connecting edge none has been guessed in
:- current(X), current(Y), edge(X,Y), not item(in(X)), not item(in(Y)).
% kill partial solutions in which a vertex has been removed for which the following holds: It still needs to be connected to a certain other vertex and there is no other vertex that is in the bag to which it is connected
:- removed(X), extend(R), childItem(R, toConnect(X,Y)), not childItem(R, path(X,Z)) : current(Z).


% kill partial solutions in which the cardinality of the subset already exceeds k
:- k(K), item(inCount(C)), C > K.

#show item/1.
#show extend/1.
#show introduced/1.
#show cost/1.
