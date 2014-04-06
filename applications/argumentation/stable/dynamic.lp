%dflat: --tables -e att -e arg

% Exactly extend one child row per child node.
1 { extend(R) : childRow(R, N) } 1 :- childNode(N).

% Only join if tuples agree on 'in' arguments
notIn(R,A) :- childNode(N), childRow(R,N), bag(N,A), arg(A), not childItem(R,in(A)).
:- extend(R1), extend(R2), arg(A), childItem(R1,in(A)), notIn(R2,A).

% Guess whether introduced arguments are in the set
{ item(in(A)) } :- introduced(A).

% Set colors for arguments in current bag
item(in(A))     :- extend(R), childItem(R, in(A)),     current(A).
auxItem(def(A)) :- extend(R), childAuxItem(R, def(A)), current(A).
auxItem(def(A)) :- current(A;B), att(B,A), item(in(B)), not item(in(A)).

% Neither in nor defeated
:- removed(A), not childItem(R, in(A)), not childAuxItem(R, def(A)), extend(R), childRow(R, N), bag(N, A).

% Conflict-free
:- item(in(A)), item(in(B)), att(A,B).

#show item/1.
#show auxItem/1.
#show extend/1.
