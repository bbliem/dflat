%dflat: --tables -e vertex -e edge -d td -n semi --default-join
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).

{ item(in(X)) : introduced(X) }.
item(in(X)) :- extend(R), not removed(X), childItem(R,in(X)).

counter(Y,N) :- introduced(Y), not item(in(Y)), N = #count { X : item(in(X)), edge(X,Y) }.
counter(Y,N0+N1) :- extend(R), current(Y), childCounter(R,Y,N1), N0 = #count { X : item(in(X)), edge(X,Y), introduced(X) }.
currentCounter(Y,N) :- current(Y), not item(in(Y)), N = #count { X: item(in(X)), edge(X,Y) }.
counterRem(Y) :- removed(Y).

:- extend(R), removed(X), not childItem(R,in(X)), childCounter(R,X,N), N<lowerBound(X,B).
:- extend(R), removed(X), not childItem(R,in(X)), childCounter(R,X,N), N>upperBound(X,B).

counterInc(cost,W,X) :- introduced(X), item(in(X)), weight(X,W).
currentCounterInc(cost,W,X) :- introduced(X), item(in(X)), weight(X,W).
currentCounterInc(cost,-W,X) :- extend(R), removed(X), childItem(R,in(X)), weight(X,W).