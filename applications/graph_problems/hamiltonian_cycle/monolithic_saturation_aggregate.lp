1 { p(X,Y) : edge(X,Y) } 1 :- vertex(X).
1 { p(X,Y) : edge(X,Y) } 1 :- vertex(Y).

% For all partitions into two sets there must be a crossing edge.
s1(X) | s2(X) :- vertex(X).
s1(X) :- saturate, vertex(X).
s2(X) :- saturate, vertex(X).
:- not saturate.

% Found crossing edge?
saturate :- p(X,Y), s1(X), s2(Y).

% Not a partition due to a set being empty?
% This is the case if the other set contains all elements.
numVertices(N) :- N = #count{ X : vertex(X) }.
saturate :- N #count{ X : s1(X) }, numVertices(N).
saturate :- N #count{ X : s2(X) }, numVertices(N).

#show p/2.
