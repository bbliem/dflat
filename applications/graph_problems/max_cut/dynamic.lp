%dflat: -d td -n semi --no-empty-leaves --tables -e vertex -e edge
edge(X,Y) :- current(X), current(Y), edge(Y,X).

1 #count { R : extend(R) : childRow(R,N) } 1 :- childNode(N).

1 #count { X,p1: item(partition(X,p1)); X,p2 : item(partition(X,p2)) } 1 :- introduced(X).

item(connected(X,Y)) :- current(X), current(Y), extend(R), childItem(R,connected(X,Y)).

item(partition(X,p1)) :- current(X), extend(R), childItem(R,partition(X,p1)).
item(partition(X,p2)) :- current(X), extend(R), childItem(R,partition(X,p2)).

item(observed(p1)) :- extend(R), childItem(R,observed(p1)).
item(observed(p2)) :- extend(R), childItem(R,observed(p2)).

item(observed(p1)) :- extend(R); childItem(R,partition(X,p1)); removed(X); not item(partition(Y,p1)) : current(Y), not introduced(Y).
item(observed(p2)) :- extend(R); childItem(R,partition(X,p2)); removed(X); not item(partition(Y,p2)) : current(Y), not introduced(Y).

item(required(p1)) :- current(X); item(partition(X,p2)); not item(partition(Y,p1)) : current(Y), X != Y; not item(observed(p1)).
item(required(p2)) :- current(X); item(partition(X,p1)); not item(partition(Y,p2)) : current(Y), X != Y; not item(observed(p2)).

item(connected(X,Y)) :- current(X), current(Y), extend(R), childItem(R,connected(X,Y)).

item(connected(X,Y)) :- current(X), current(Y), edge(X,Y), X < Y, item(partition(X,p1)), item(partition(Y,p1)).

item(connected(X,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(Y,Z)), X < Y, Y < Z, X < Z.
item(connected(X,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(Z,Y)), X < Y, Z < Y, X < Z.

item(connected(X,Y)) :- current(X), current(Y), current(Z), item(connected(X,Z)), item(connected(Y,Z)), X < Y, X < Z, Y < Z.
item(connected(Y,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(X,Z)), X < Y, X < Z, Y < Z.

item(establishConnection(X,Y)) :- current(X), current(Y), item(partition(X,p1)), item(partition(Y,p1)), X < Y, not item(connected(X,Y)).

item(establishConnection(X,Y)) :- current(X), current(Y), extend(R), childItem(R,establishConnection(X,Y)), not item(connected(X,Y)).

%ok :- current(X), current(Y), item(partition(X,p1)), item(partition(Y,p2)), edge(X,Y).
ok :- current(X), current(Y), item(partition(X,p1)), item(partition(Y,p2)).
ok :- current(X), item(partition(X,p1)), item(required(p2)).
ok :- current(X), item(partition(X,p2)), item(required(p1)).
ok :- current(X), item(partition(X,p1)), item(observed(p2)).
ok :- current(X), item(partition(X,p2)), item(observed(p1)).
ok :- final, item(observed(p1)), item(observed(p2)).
ok :- 0 #count { R : extend(R) } 0.
:- not ok.

:- current(X), item(partition(X,p1)), item(observed(p1)).

:- current(X), extend(R), extend(S), childItem(R,observed(p1)), childItem(S,observed(p1)), R < S.

:- current(X), extend(R), extend(S), childItem(R,partition(X,p1)), childItem(S,partition(X,p2)), R != S.

:- removed(X); extend(R); childItem(R,establishConnection(X,Y)); not childItem(R,connected(X,Z)) : current(Z); not childItem(R,connected(Z,X)) : current(Z).
:- removed(X); extend(R); childItem(R,establishConnection(Y,X)); not childItem(R,connected(X,Z)) : current(Z); not childItem(R,connected(Z,X)) : current(Z).

:- removed(X); extend(R); childItem(R,partition(X,p1)); introduced(Z); item(partition(Z,p1)); not childItem(R,connected(X,Y)) : childItem(R,partition(Y,p1)); not childItem(R,connected(Y,X)) : childItem(R,partition(Y,p1)).

cut(X,Y) :- current(X), current(Y), edge(X,Y), item(partition(X,p1)), item(partition(Y,p2)), X < Y.
cut(X,Y) :- current(X), current(Y), edge(X,Y), item(partition(X,p2)), item(partition(Y,p1)), X < Y.
cut(X,Y) :- current(X), current(Y), edge(X,Y), item(partition(X,p1)), item(partition(Y,p2)), X < Y.
cut(X,Y) :- current(X), current(Y), edge(X,Y), item(partition(X,p2)), item(partition(Y,p1)), X < Y.

newCut(X,Y) :- current(X), current(Y), cut(X,Y), introduced(X), X < Y.
newCut(X,Y) :- current(X), current(Y), cut(X,Y), introduced(Y), X < Y.

relevantChildCost(R,C) :- extend(R), childCost(R,C).

cost(CC - NC) :- CC = #sum { C,R : relevantChildCost(R,C) }, NC = #count { X,Y : newCut(X,Y) }, 0 #count { N: childNode(N) } 1.

cost(CC1 + CC2 + LC) :- extend(R), extend(S), childCost(R,CC1), childCost(S,CC2), LC = #count{ X,Y : cut(X,Y) }, R < S, 2 #count { N : childNode(N) } 2.

cost(CC - NC + (LC * (EP - 1))) :- CC = #sum { C,R : relevantChildCost(R,C) }, LC = #count { X,Y : cut(X,Y) }, NC = #count { X,Y : newCut(X,Y) }, EP = #count { N : childNode(N) }, EP > 2.

#show extend/1.
#show item/1.
#show cost/1.
