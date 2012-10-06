1 { p(X,Y) : edge(X,Y) } 1 :- vertex(X).
1 { p(X,Y) : edge(X,Y) } 1 :- vertex(Y).

pTrans(X,Y) :- p(X,Y).
pTrans(X,Z) :- pTrans(X,Y), p(Y,Z).

:- vertex(X), vertex(Y), not pTrans(X,Y).

#hide. #show p/2.
