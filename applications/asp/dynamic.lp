% Each answer set encodes a top-level item set and a subsidiary one
length(2).

% Make explicit when an atom is false or a rule is unsatisfied
false(R,X) :- childRow(R,N), childBag(N,X), not childItem(R,X).
false(S,X) :- childRow(R,N), childBag(N,X), sub(R,S), not childItem(S,X).

% A child item set is "killed" when a removed rule is unsatisfied by it
killed(S) :- rule(X), removed(X), false(S,X).

% Guess a root-to-leaf path in a characteristic for every child node
1 { extend(0,R) : rootOf(R,N) } 1 :- childNode(N).
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), L < 2.

% Only join child item sets that coincide on common atoms
:- extend(L,X;Y), atom(A), childItem(X,A), false(Y,A).

% Some child item sets cannot be extended since they are killed
:- extend(_,S), killed(S).

% True atoms and satisfied rules remain so unless removed
item(L,X) :- extend(L,S), childItem(S,X), current(X).

% Guess truth value of introduced atoms
{ item(0;1,A) : atom(A) : introduced(A) }.

% Through the guess, rules may become satisfied
item(1,R) :- current(R;A), head(R,A),     item(1,A).
item(1,R) :- current(R;A), pos(R,A),  not item(1,A).
item(1,R) :- current(R;A), neg(R,A),      item(1,A).

item(2,R) :- current(R;A), head(R,A),     item(2,A).
item(2,R) :- current(R;A), pos(R,A),  not item(2,A).
% If a negative body atom is true on the top level, the rule disappears from reduct (w.r.t. the top level)
item(2,R) :- current(R;A), neg(R,A),      item(1,A).

% Level 1 interpretation must not be bigger than level 0
:- atom(A), item(2,A), not item(1,A).

% Inherit (or extend) markers indicating that the level 1 interpretation is smaller
item(2,smaller) :- extend(2,S), childItem(S,smaller).
item(2,smaller) :- atom(A), item(1,A), not item(2,A).

% Make sure that eventually only minimal models of the reduct survive
:- final, extend(1,R), sub(R,S), childItem(S,smaller), not killed(S).



#hide.
#show extend/2.
#show length/1.
#show item/2.
