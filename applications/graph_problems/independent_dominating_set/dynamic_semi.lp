1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(R1;R2), childItem(R1,in(X)), not childItem(R2,in(X)).
item(in(X)) :- extend(R), childItem(R,in(X)), current(X).
item(dominated(X)) :- extend(R), childItem(R,dominated(X)), current(X).
:- removed(X), extend(R), not childItem(R,in(X)), not childItem(R,dominated(X)).
{ item(in(X)) : introduced(X) }.
item(dominated(X)) :- item(in(Y)), edge(Y,X), current(X).
:- edge(X,Y), item(in(X;Y)).
