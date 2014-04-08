% edges are undirected
edge(X,Y) :- edge(Y,X).

% there are three partitions, two for the bipartite graph and one for the vertices that are cut
number(1;2;3).

% guess the partition of a vertex
1 {part(X,N) : number(N)} 1 :- vertex(X).

% the resulting graph is not bipartite, if there is an edge between two vertices in the same partition
not_bipartite :- part(X,N), part(Y,N), edge(X,Y), N != 3.

% kill all answer sets in which the remaining graph is not bipartite
:- not_bipartite.
% kill all answer sets in which the removed partition does not contain exactly k vertices
:- k(K), C = #count {X : part(X,3)}, K != C.

#show part/2.
