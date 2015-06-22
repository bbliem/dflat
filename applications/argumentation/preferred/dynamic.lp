%dflat: -e att -e arg

length(2).
or(0).
and(1).

1 #count { R  : extend(0,R)  : rootOf(R,N) } 1 :- childNode(N).
1 #count { S1 : extend(1,S1) : sub(R,S1)   } 1 :- extend(0,R).
1 #count { S2 : extend(2,S2) : sub(S1,S2)  } 1 :- extend(1,S1).

% Only join if tuples agree on 'in' arguments
notIn(S,A) :- childNode(N), atNode(S,N), not atLevel(S,0), bag(N,A), arg(A), not childItem(S,in(A)).
:- extend(L,S1), extend(L,S2), arg(A), childItem(S1,in(A)), notIn(S2,A).

% Guess whether introduced arguments are in the set
{ item(1,in(A)) } :- introduced(A).
{ item(2,in(A)) } :- introduced(A).
item(2,in(A)) :- item(1,in(A)).

% Set colors for arguments in current bag
item(L,in(A))     :- extend(L,S), childItem(S, in(A)),     current(A).
auxItem(L,def(A)) :- extend(L,S), childAuxItem(S, def(A)), current(A).
auxItem(L,def(A)) :- current(A), current(B), att(B,A), item(L,in(B)), not item(L,in(A)).
auxItem(L,att(A)) :- extend(L,S), childAuxItem(S, att(A)), not auxItem(L,def(A)).
auxItem(L,att(A)) :- current(A), current(B), att(A,B), item(L,in(B)), not item(L,in(A)), not auxItem(L,def(A)).

% Attacking but not defeated
:- extend(L,S), childAuxItem(S, att(A)), removed(A).

% Conflict-free
:- item(L,in(A)), item(L,in(B)), att(A,B).

% Certificate, if in set at second level is larger than in set at first level
auxItem(2,cert) :- extend(2,S), childAuxItem(S,cert).
auxItem(2,cert) :- item(2, in(A)), not item(1,in(A)).

% Reject in case we have a counter-example (i.e., a larger admissible set)
reject :- final, auxItem(2,cert).
accept :- final, not reject.

#show length/1.
#show or/1.
#show and/1.
#show reject/0.
#show accept/0.
#show item/2.
#show auxItem/2.
#show extend/2.
