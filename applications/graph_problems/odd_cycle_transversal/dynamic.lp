%dflat: -e vertex -e edge --tables -d td -n semi --no-empty-leaves --no-empty-root
% edges are undirected
edge(X,Y) :- edge(Y,X), current(X), current(Y).

% there are three partitions, two for the bipartite graph and one for the vertices that are cut
number(1;2;3).

% propagate partitions
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(part(X,N)) :- extend(R), childItem(R,part(X,N)), current(X), number(N).

% propagate removed counter
% (increase, if introduced vertices are  added to the remove partition)
% (subtract 1 for each vertex that is in the remove partition in multiple subtrees - otherwise it is counted twice)
subCounted(X,R1) :- extend(R1), extend(R2), childItem(R1,part(X,3)), childItem(R2,part(X,3)), R1 < R2.
subCount(SC) :- SC = #count{X,R1 : subCounted(X,R1)}.
item(removedCount(SUM+C-SC)) :- SUM = #sum{COUNT,R : extend(R), childItem(R,removedCount(COUNT))}, removeCount(C), subCount(SC).

% prevent joins where a vertex is only part of a partition in one child node
:- extend(R1), extend(R2), R1 < R2, current(X), childItem(R1, part(X,N)), not childItem(R2, part(X,N)).
% prevent join where a vertex is part of two different partitions in the child nodes
:- extend(R1), extend(R2), R1 < R2, current(X), childItem(R1, part(X,N1)), childItem(R2, part(X,N2)), N1 != N2.

% count the vertices that have been introduced and which are guessed into the remove partition
removeCount(C) :- C = #count {X : item(part(X,3)), introduced(X)}.

% guess the partition for introduced vertices
1 {item(part(X,N)) : number(N)} 1 :- introduced(X).

% kill (partial) solutions where two vertices from the same partition (1 and 2) are connected
:- current(X), current(Y), edge(X,Y), item(part(X,N)), item(part(Y,N)), N != 3.
% [OPT]: kill (partial) solutions where the removed counter already exceeds the given value k
:- item(removedCount(N)), k(K), N > K.

% kill final solutions where the removed counter does not equal the given number k
:- root, extend(R), childItem(R, removedCount(N)), k(K), N != K.

#show item/1.
#show extend/1.
#show introduced/1.
#show cost/1.
