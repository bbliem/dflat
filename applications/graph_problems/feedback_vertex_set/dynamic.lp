%dflat: -e vertex -e edge --tables -d td -n semi --no-empty-leaves --no-empty-root
% propagate out nodes and known paths
1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
item(out(X)) :- extend(R), childItem(R,out(X)), current(X).
item(path(X,Y)) :- extend(R), childItem(R, path(X,Y)), current(X), current(Y).

% propagate "out" counter
% (increase, if introduced vertices are guessed "out")
% (subtract 1 for each vertex that has been guessed "out" in multiple subtrees - otherwise it is counted twice)
subCounted(X,R1) :- extend(R1), extend(R2), childItem(R1,out(X)), childItem(R2,out(X)), R1 < R2.
subCount(SC) :- SC = #count{X,R1 : subCounted(X,R1) }.
item(outCount(SUM+C-SC)) :- SUM = #sum{COUNT,R : extend(R), childItem(R,outCount(COUNT))}, localOutCount(C), subCount(SC).

% prevent joins where a vertex is only out in one child node
:- extend(R1), extend(R2), R1 < R2, current(X), childItem(R1, out(X)), not childItem(R2, out(X)).

% count the vertices that have been introduced and which are guessed into the remove partition
localOutCount(C) :- C = #count {X : item(out(X)), introduced(X)}.

% guess, if introduced vertices are out
0 {item(out(X))} 1 :- introduced(X).

% infer new paths for current vertices (the second and third rule should not be necessary)
item(path(X,Y)) :- current(X), current(Y), edge(X,Y), not item(out(X)), not item(out(Y)).
item(path(X,Z)) :- current(X), current(Y), current(Z), item(path(X,Y)), edge(Y,Z), not item(out(X)), not item(out(Y)), not item(out(Z)).
item(path(X,Z)) :- current(X), current(Y), current(Z), edge(X,Y), item(path(Y,Z)), not item(out(X)), not item(out(Y)), not item(out(Z)).
item(path(X,Z)) :- current(X), current(Y), current(Z), item(path(X,Y)), item(path(Y,Z)), not item(out(X)), not item(out(Y)), not item(out(Z)).

% check current vertices for cycles
cycle :- item(path(X,X)).

% kill partial solutions in which the remaining vertices contain a cycle
:- cycle.

% kill partial solutions in which the cardinality of the subset already exceeds k
:- k(K), item(outCount(C)), C > K.

#show item/1.
#show extend/1.
#show introduced/1.
#show cost/1.
