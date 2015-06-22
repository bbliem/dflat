edge(X,Y) :- edge(Y,X).

1 #count { X,p1 : partition(X,p1); X,p2 : partition(X,p2) } 1 :- vertex(X).

connected(X,Y) :- edge(X,Y), X != Y, partition(X,p1), partition(Y,p1).

connected(X,Z) :- connected(X,Y), edge(Y,Z), X != Z, Y != Z, partition(X,p1), partition(Y,p1), partition(Z,p1).
connected(X,Z) :- connected(Y,Z), edge(X,Y), X != Z, Y != Z, partition(X,p1), partition(Y,p1), partition(Z,p1).

:- partition(X,p1), partition(Y,p1), X < Y, not connected(X,Y).

ok :- vertex(X), vertex(Y), partition(X,p1), partition(Y,p2), edge(X,Y).
:- not ok.

cut(X,Y) :- edge(X,Y), partition(X,p1), partition(Y,p2), X < Y.
cut(X,Y) :- edge(X,Y), partition(X,p2), partition(Y,p1), X < Y.

cuts(V) :- V = #count { X,Y : cut(X,Y) }.

#maximize { 1,X,Y : cut(X,Y) }.

#show cuts/1.
#show partition/2.
