color(red;green;blue).

1 { chosenChildTuple(T) : childTuple(T,N) } 1 :- childNode(N).
:- chosenChildTuple(A), chosenChildTuple(B), mapped(A,X,VA), mapped(B,X,VB), VA != VB.
% Or, equivalently... (which avoids the inequality but this probably does not matter since already the grounder can figure everything out)
%:- chosenChildTuple(A), childTuple(A,NA), chosenChildTuple(B), childTuple(B,NB), childBag(NA,X), childBag(NB,X), mapped(A,X,V), not mapped(B,X,V).

map(X,C) :- chosenChildTuple(I), mapped(I,X,C), current(X).

1 { map(X,C) : color(C) } 1 :- introduced(X).
:- edge(X,Y), map(X,C), map(Y,C).

#hide.
#show map/2.
#show chosenChildTuple/1.
