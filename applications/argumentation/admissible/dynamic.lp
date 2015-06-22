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
auxItem(def(A)) :- current(A), current(B), att(B,A), item(in(B)), not item(in(A)).
auxItem(att(A)) :- extend(R), childAuxItem(R, att(A)), not auxItem(def(A)).
auxItem(att(A)) :- current(A), current(B), att(A,B), item(in(B)), not item(in(A)), not auxItem(def(A)).

% Attacking but not defeated
:- extend(R), childAuxItem(R, att(A)), removed(A).

% Conflict-free
:- item(in(A)), item(in(B)), att(A,B).

#show item/1.
#show auxItem/1.
#show extend/1.

