%dflat: -d td -n semi --no-empty-leaves --no-empty-root --tables -e vertex -e edge
edge(X,Y) :- current(X), current(Y), edge(Y,X).

1 #count { R : extend(R) : childRow(R,N) } 1 :- childNode(N).

0 #count { X : item(selected(X)) } 1 :- introduced(X).

item(selected(X)) :- current(X), extend(R), childItem(R,selected(X)).

item(observed(X)) :- current(X), extend(R), childItem(R,observed(X)).

item(observed(Y)) :- current(X), current(Y), item(selected(X)), edge(X,Y), not item(selected(Y)), X != Y.

item(connected(X,Y)) :- current(X), current(Y), item(selected(X)), item(selected(Y)), edge(X,Y), X < Y.

item(connected(X,Y)) :- current(X), current(Y), extend(R), childItem(R,connected(X,Y)).

item(connected(X,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(Y,Z)), X < Y, Y < Z, X < Z.
item(connected(X,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(Z,Y)), X < Y, Z < Y, X < Z.

item(connected(X,Y)) :- current(X), current(Y), current(Z), item(connected(X,Z)), item(connected(Y,Z)), X < Y, X < Z, Y < Z.
item(connected(Y,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(X,Z)), X < Y, X < Z, Y < Z.

item(establishConnection(X,Y)) :- current(X), current(Y), item(selected(X)), item(selected(Y)), not item(connected(X,Y)), X < Y.

item(establishConnection(X,Y)) :- current(X), current(Y), extend(R), childItem(R,establishConnection(X,Y)), not item(connected(X,Y)).

ok(X) :- current(Y), extend(R), removed(X), childItem(R,connected(X,Y)).
ok(X) :- current(Y), extend(R), removed(X), childItem(R,connected(Y,X)).

item(stop) :- extend(R), childItem(R,selected(X)), removed(X), not ok(X).

item(stop) :- extend(R), childItem(R,stop).

:- current(X), item(stop), item(selected(X)).

:- removed(X), extend(R), not childItem(R,selected(X)), not childItem(R,observed(X)).

:- removed(X); extend(R); childItem(R,establishConnection(X,Y)); not childItem(R,connected(X,Z)) : current(Z); not childItem(R,connected(Z,X)) : current(Z).
:- removed(X); extend(R); childItem(R,establishConnection(Y,X)); not childItem(R,connected(X,Z)) : current(Z); not childItem(R,connected(Z,X)) : current(Z).

:- extend(R), extend(S), childItem(R,stop), childItem(S,stop), R != S.

:- current(X), extend(R), extend(S), childItem(R,selected(X)), not childItem(S,selected(X)), R != S.

relevantChildCost(R,C) :- extend(R), childCost(R,C).

cost(NC) :- NC = #count{ X : item(selected(X)), introduced(X) }, 0 #count { N : childNode(N) } 0.

cost(CC + NC) :- extend(R), childCost(R,CC), NC = #count{ X : item(selected(X)), introduced(X) }, 1 #count { N : childNode(N) } 1.

cost(CC1 + CC2 - LC) :- extend(R), extend(S), childCost(R,CC1), childCost(S,CC2), LC = #count{ X : item(selected(X)) }, R < S, 2 #count { N : childNode(N) } 2.

cost(CC - (LC * (EP - 1))) :- LC = #count { X : item(selected(X)) }, CC = #sum { C,R : relevantChildCost(R,C) }, EP = #count { N : childNode(N) }, EP > 2.

#show extend/1.
#show item/1.
#show cost/1.
