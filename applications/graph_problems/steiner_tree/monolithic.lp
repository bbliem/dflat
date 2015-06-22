edge(X,Y) :- edge(Y,X).

{ selectedEdge(X,Y) : edge(X,Y), X < Y }.

connected(X,Y) :- selectedEdge(X,Y), X < Y.

connected(X,Z) :- connected(X,Y), selectedEdge(Y,Z), X != Z, Y != Z.
connected(X,Z) :- connected(X,Y), selectedEdge(Z,Y), X != Z, Y != Z.
connected(X,Z) :- connected(Y,Z), selectedEdge(X,Y), X != Z, Y != Z.
connected(X,Z) :- connected(Y,Z), selectedEdge(Y,X), X != Z, Y != Z.

:- terminalVertex(X), terminalVertex(Y), X < Y, not connected(X,Y), not connected(Y,X).

relevantWeight(V1,V2,W) :- selectedEdge(V1,V2), weight(V1,V2,W), V1 < V2.

#minimize { W,V1,V2 : relevantWeight(V1,V2,W) }.

#show selectedEdge/2.
