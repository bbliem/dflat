%dflat: -d td -n semi --no-empty-leaves --no-empty-root --tables -e vertex -e edge
edge(X,Y) :- current(X), current(Y), edge(Y,X).

1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

0 { item(selectedEdge(X,Y)) } 1 :- current(X), current(Y), introduced(X), edge(X,Y), X < Y.
0 { item(selectedEdge(X,Y)) } 1 :- current(X), current(Y), introduced(Y), edge(X,Y), X < Y.

item(selectedEdge(X,Y)) :- current(X), current(Y), extend(R), childItem(R,selectedEdge(X,Y)).

item(observed(X)) :- current(X), extend(R), childItem(R,observed(X)).

item(observed(X)) :- current(X), current(Y), terminalVertex(X), item(selectedEdge(X,Y)).
item(observed(X)) :- current(X), current(Y), terminalVertex(X), item(selectedEdge(Y,X)).
item(observed(Y)) :- current(X), current(Y), terminalVertex(Y), item(selectedEdge(X,Y)).
item(observed(Y)) :- current(X), current(Y), terminalVertex(Y), item(selectedEdge(Y,X)).

item(steinerVertex(X)) :- current(X), extend(R), childItem(R,steinerVertex(X)).

item(steinerVertex(X)) :- current(X), current(Y), item(selectedEdge(X,Y)), not terminalVertex(X).
item(steinerVertex(Y)) :- current(X), current(Y), item(selectedEdge(X,Y)), not terminalVertex(Y).
item(steinerVertex(X)) :- current(X), current(Y), item(selectedEdge(Y,X)), not terminalVertex(X).
item(steinerVertex(Y)) :- current(X), current(Y), item(selectedEdge(Y,X)), not terminalVertex(Y).

item(connected(X,Y)) :- current(X), current(Y), item(selectedEdge(X,Y)), X < Y.

item(connected(X,Y)) :- current(X), current(Y), extend(R), childItem(R,connected(X,Y)).

item(connected(X,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(Y,Z)), X < Y, Y < Z, X < Z.
item(connected(X,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(Z,Y)), X < Y, Z < Y, X < Z.

item(connected(X,Y)) :- current(X), current(Y), current(Z), item(connected(X,Z)), item(connected(Y,Z)), X < Y, X < Z, Y < Z.
item(connected(Y,Z)) :- current(X), current(Y), current(Z), item(connected(X,Y)), item(connected(X,Z)), X < Y, X < Z, Y < Z.

item(establishConnection(X,Y)) :- current(X), current(Y), terminalVertex(X), terminalVertex(Y), not item(connected(X,Y)), X < Y.

item(establishConnection(X,Y)) :- current(X), current(Y), terminalVertex(X), item(steinerVertex(Y)), not item(connected(X,Y)), X < Y.
item(establishConnection(X,Y)) :- current(X), current(Y), terminalVertex(Y), item(steinerVertex(X)), not item(connected(X,Y)), X < Y.

item(establishConnection(X,Y)) :- current(X), current(Y), item(steinerVertex(X)), item(steinerVertex(Y)), not item(connected(X,Y)), X < Y.

ok(X) :- current(Y), extend(R), removed(X), childItem(R,connected(X,Y)).
ok(X) :- current(Y), extend(R), removed(X), childItem(R,connected(Y,X)).

item(stop) :- extend(R), childItem(R,steinerVertex(X)), removed(X), not ok(X).
item(stop) :- extend(R), childItem(R,observed(X)), removed(X), not ok(X).

item(stop) :- extend(R), childItem(R,stop).

relevantWeight(V1,V2,W) :- item(selectedEdge(V1,V2)), weight(V1,V2,W), V1 < V2.
relevantWeight(V1,V2,W) :- item(selectedEdge(V1,V2)), weight(V2,V1,W), V1 < V2.

newWeight(V1,V2,W) :- relevantWeight(V1,V2,W), introduced(V1).
newWeight(V1,V2,W) :- relevantWeight(V1,V2,W), introduced(V2).

localCost(C) :- C = #sum { W,X,Y : relevantWeight(X,Y,W) }.

additionalCost(C) :- C = #sum { W,X,Y : newWeight(X,Y,W) }.

relevantChildCost(R,C) :- extend(R), childCost(R,C).

cost(NC) :- additionalCost(NC), 0 #count { N : childNode(N) } 0.

cost(CC + NC) :- extend(R), childCost(R,CC), additionalCost(NC), 1 #count { N : childNode(N) } 1.

cost(CC - (LC * (EP - 1))) :- localCost(LC), CC = #sum { CC1,R : relevantChildCost(R,CC1) }, EP = #count { N : childNode(N) }, EP >= 2.

:- current(X), current(Y), extend(R), extend(S), childItem(R,selectedEdge(X,Y)), not childItem(S,selectedEdge(X,Y)), R != S.

:- removed(X); extend(R); childItem(R,establishConnection(X,Y)); not childItem(R,connected(X,Z)) : current(Z); not childItem(R,connected(Z,X)) : current(Z).
:- removed(X); extend(R); childItem(R,establishConnection(Y,X)); not childItem(R,connected(X,Z)) : current(Z); not childItem(R,connected(Z,X)) : current(Z).

:- extend(R), removed(X), terminalVertex(X), not childItem(R,observed(X)).

:- extend(R), extend(S), childItem(R,stop), childItem(S,stop), R < S.

:- current(X), item(stop), item(selectedEdge(X,Y)).
:- current(X), item(stop), item(selectedEdge(Y,X)).

:- current(X), item(stop), item(steinerVertex(X)).

:- current(X), item(stop), terminalVertex(X).

#show extend/1.
#show item/1.
#show cost/1.
