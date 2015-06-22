%dflat: -e head -e pos -e neg -d td --no-empty-leaves --depth 1
% Each answer set encodes a top-level item set and a subsidiary one
length(2).
or(0).
and(1).

% Make explicit when an atom is false or a rule is unsatisfied
false(S,X) :- sub(_,S), atNode(S,N), bag(N,X), not childItem(S,X).

% Guess a branch in the item tree for every child node
1 { extend(0,R) : rootOf(R,N) } 1 :- childNode(N).
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), L < 2.

% Only join child item sets that coincide on common atoms
:- extend(L,X), extend(L,Y), atom(A), childItem(X,A), false(Y,A).

% A child item set cannot be extended if a removed rule is unsatisfied by it
:- extend(_,S), rule(X), removed(X), false(S,X).

% True atoms and satisfied rules remain so unless removed
item(L,X) :- extend(L,S), childItem(S,X), current(X).

% Guess truth value of introduced atoms
{ item(1,A;2,A) : atom(A), introduced(A) }.

% Through the guess, rules may become satisfied
item(1,R) :- current(R), current(A), head(R,A),     item(1,A).
item(1,R) :- current(R), current(A), pos(R,A),  not item(1,A).
item(1,R) :- current(R), current(A), neg(R,A),      item(1,A).

item(2,R) :- current(R), current(A), head(R,A),     item(2,A).
item(2,R) :- current(R), current(A), pos(R,A),  not item(2,A).
% If a negative body atom is true on the top level, the rule disappears from reduct (w.r.t. the top level)
item(2,R) :- current(R), current(A), neg(R,A),      item(1,A).

% Level 2 interpretation must not be bigger than level 1
:- atom(A), item(2,A), not item(1,A).

% Inherit (or extend) markers indicating that the level 2 interpretation is smaller
item(2,smaller) :- extend(2,S), childItem(S,smaller).
item(2,smaller) :- atom(A), item(1,A), not item(2,A).

% Make sure that eventually only minimal models of the reduct survive
reject :- final, item(2,smaller).
accept :- final, not reject.



#show extend/2.
#show length/1.
#show item/2.
#show or/1.
#show and/1.
#show accept/0.
#show reject/0.
