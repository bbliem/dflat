% Suppose that the top level quantifier is \exists
itemSet(S,0,N) :- childRow(S,N).
itemSet(S,L+1,N) :- itemSet(R,L,N), sub(R,S).

false(S,A) :- atom(L,A), itemSet(S,L,N), childBag(N,A), not childItem(S,A).
-sat(S,C) :- clause(C), levels(L), itemSet(S,L-1,N), childBag(N,C), not childItem(S,C).

forallItemSet(S) :- childRow(S,_),              sub(S,_).
forallItemSet(S) :- existsItemSet(R), sub(R,S), sub(S,_).
existsItemSet(S) :- forallItemSet(R), sub(R,S), sub(S,_).

bad(S) :- childItem(S,bad). % Has already been bad
bad(S) :- levels(L), itemSet(S,L-1,_), removed(C), clause(C), -sat(S,C). % Becomes bad due to clause removal
-bad(S) :- existsItemSet(S), sub(S,T), not bad(T).
bad(S) :- existsItemSet(S), not -bad(S).
bad(S) :- forallItemSet(S), sub(S,T), bad(T).

% Actual solving part...

% Guess a path
1 { extend(L-1,S) : itemSet(S,L-1,N) } 1 :- childNode(N), levels(L).
extend(L-1,R) :- extend(L,S), sub(R,S).

%item(L,X) :- extend(L,S), childItem(S,X), not removed(X).
item(L,X) :- extend(L,S), childItem(S,X), current(X).
item(L,bad) :- extend(L,S), childItem(S,bad), not root.

% Only join root-to-leaf paths that coincide on common atoms
:- extend(L,X;Y), atom(L,A), childItem(X,A), false(Y,A).

% The leaves (and only they) can contain "bad"
%item(L,bad) :- levels(L+1), extend(L,S), bad(S).
item(L,bad) :- levels(L+1), extend(L,S), bad(S), not root.

:- extend(0,S), bad(S).

% Guess truth value of introduced atoms
{ item(L,A) : atom(L,A) : introduced(A) }.

% Through the guess, clauses may become satisfied
item(L-1,C) :- levels(L), pos(C,A), atom(LA,A), current(C;A), item(LA,A).
item(L-1,C) :- levels(L), neg(C,A), atom(LA,A), current(C;A), not item(LA,A).



#hide.
#show levels/1.
#show item/2.
#show extend/2.
