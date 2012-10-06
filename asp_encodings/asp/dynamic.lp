% Each answer set encodes a top-level item set and a subsidiary one
levels(2).

% Make explicit when an atom is false or a rule is unsatisfied
false(R,X) :- childRow(R,N), childBag(N,X), not childItem(R,X).
false(S,X) :- childRow(R,N), childBag(N,X), sub(R,S), not childItem(S,X).

% A child item set is "killed" when a removed rule is unsatisfied by it
killedRow(R) :- rule(X), removed(X), childRow(R,_), false(R,X).
killedSub(S) :- rule(X), removed(X), sub(_,S), false(S,X).

% Guess a root-to-leaf path in a characteristic for every child node
1 { extend(0,R) : childRow(R,N) } 1 :- childNode(N).
1 { extend(1,S) : sub(R,S) } 1 :- extend(0,R).

% Only join child item sets that coincide on common atoms
:- extend(L,X;Y), atom(A), childItem(X,A), false(Y,A).

% Some child item sets cannot be extended since they are killed
:- extend(0,R), killedRow(R).
:- extend(1,S), killedSub(S).

% True atoms and satisfied rules remain so unless removed
item(L,X) :- extend(L,S), childItem(S,X), current(X).

% Guess truth value of introduced atoms
{ item(0;1,A) : atom(A) : introduced(A) }.

% Through the guess, rules may become satisfied
item(0,R) :- current(R;A), head(R,A), item(0,A).
item(0,R) :- current(R;A), pos(R,A), not item(0,A).
item(0,R) :- current(R;A), neg(R,A), item(0,A).

item(1,R) :- current(R;A), head(R,A), item(1,A).
item(1,R) :- current(R;A), pos(R,A), not item(1,A).
% If a negative body atom is true (on the top level), the rule disappears from reduct (w.r.t. the top level)
item(1,R) :- current(R;A), neg(R,A), item(0,A).

% Level 1 interpretation must not be bigger than level 0
:- atom(A), item(1,A), not item(0,A).

% Inherit (or extend) markers indicating that the level 1 interpretation is smaller
item(1,smaller) :- extend(1,S), childItem(S,smaller).
item(1,smaller) :- atom(A), item(0,A), not item(1,A).

% Make sure that eventually only minimal models of the reduct survive
:- root, extend(0,R), sub(R,S), childItem(S,smaller), not killedSub(S).



#hide.
#show extend/2.
#show levels/1.
#show item/2.
