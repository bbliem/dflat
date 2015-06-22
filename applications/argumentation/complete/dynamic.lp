%dflat: --tables -e att -e arg

% Exactly extend one child row per child node.
1 { extend(R) : childRow(R, N) } 1 :- childNode(N).

% Only join if tuples agree on 'in' arguments
notIn(R,A) :- childNode(N), childRow(R,N), bag(N,A), arg(A), not childItem(R,in(A)).
:- extend(R1), extend(R2), arg(A), childItem(R1,in(A)), notIn(R2,A).
% And on 'def'/'defc' arguments
notDef(R,A) :- childNode(N), childRow(R,N), bag(N,A), arg(A), not childItem(R,def(A)), not childItem(R,defc(A)).
:- extend(R1), extend(R2), arg(A), childItem(R1,def(A)), notDef(R2,A).
:- extend(R1), extend(R2), arg(A), childItem(R1,defc(A)), notDef(R2,A).

% Guess whether introduced arguments are in the set, defeated, or out
1 #count { A,in : item(in(A)); A,defc : defc(A); A,outc : outc(A) } 1 :- introduced(A).
defc(A) :- extend(R), childItem(R, defc(A)), current(A).
outc(A) :- extend(R), childItem(R, outc(A)), current(A).

% Set colors for arguments in current bag
item(in(A))   :- current(A), extend(R), childItem(R, in(A)).
item(def(A))  :- current(A), extend(R), childItem(R, def(A)).
item(def(A))  :- current(A), current(B), att(B,A), item(in(B)), defc(A).
item(defc(A)) :- current(A), defc(A), not item(def(A)).
item(out(A))  :- current(A), extend(R), childItem(R, out(A)).
item(out(A))  :- current(A), current(B), att(B,A), item(out(B)), outc(A).
item(out(A))  :- current(A), current(B), att(B,A), outc(B), outc(A).
item(outc(A)) :- current(A), outc(A), not item(out(A)).

% Not defeated, or not really out
:- extend(R), childItem(R, defc(A)), removed(A).
:- extend(R), childItem(R, outc(A)), removed(A).
:- current(A), current(B), att(B,A), item(in(B)), item(outc(A)).
:- current(A), current(B), att(B,A), item(in(B)), item(out(A)).
:- current(A), current(B), att(B,A), item(out(B)), item(in(A)).
:- current(A), current(B), att(B,A), item(outc(B)), item(in(A)).

% Conflict-free
:- item(in(A)), item(in(B)), att(A,B).

#show item/1.
#show extend/1.
