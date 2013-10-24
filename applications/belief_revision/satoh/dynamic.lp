% Top-level items correspond to bag assignments, second-level items are 
% either counter-examples (of beiing subset minimal) or equivalent/equal
levels(2).

% Make explicit that an atom or clause is not satisified in a child
false(R,ma(X)) :- childRow(R,N), childBag(N,X), not childItem(R,ma(X)).
false(R,mb(X)) :- childRow(R,N), childBag(N,X), not childItem(R,mb(X)).
false(R,cl(X)) :- childRow(R,N), childBag(N,X), not childItem(R,cl(X)).
false(S,ma(X)) :- childRow(R,N), sub(R,S), childBag(N,X), not childItem(S,ma(X)).
false(S,mb(X)) :- childRow(R,N), sub(R,S), childBag(N,X), not childItem(S,mb(X)).
false(S,cl(X)) :- childRow(R,N), sub(R,S), childBag(N,X), not childItem(S,cl(X)).

1 { extend(0,R) : childRow(R,N) } 1 :- childNode(N).
1 { extend(1,S) : sub(R,S) } 1 :- extend(0,R).

% Only join child item sets that coincide on common atoms in both ma and mb
:- extend(L,RS1;RS2), atom(A), childItem(RS1,ma(A)), false(RS2,ma(A)).
:- extend(L,RS1;RS2), atom(A), childItem(RS1,mb(A)), false(RS2,mb(A)).

% Do not extend row in case an unsatisfied clause is removed
killed(RS) :- clausea(C), removed(C), false(RS,cl(C)).
killed(RS) :- clauseb(C), removed(C), false(RS,cl(C)).
:- extend(_,RS), killed(RS).

% Do not extend row in case there exists the same row as subset counter-example
diff(S) :- childRow(R,N), sub(R,S), childItem(R,X), not childItem(S,X).
diff(S) :- childRow(R,N), sub(R,S), not childItem(R,X), childItem(S,X), X != subset.
kill(R) :- childRow(R,N), sub(R,S), not diff(S).
:- extend(0,R), kill(R).

% Keep atoms and satisfied clauses if not removed
item(L,ma(X)) :- extend(L,RS), childItem(RS,ma(X)), current(X). 
item(L,mb(X)) :- extend(L,RS), childItem(RS,mb(X)), current(X).
item(L,cl(X)) :- extend(L,RS), childItem(RS,cl(X)), current(X).

% Guess truth value for any introduced atom
{ item(0;1,ma(A)) : atom(A) : introduced(A) }.
{ item(0;1,mb(A)) : atom(A) : introduced(A) }.

% add clause to clause set in case it is satisfied
level(0;1).
item(L,cl(C)) :- clausea(C), current(C;A), pos(A,C),     item(L,ma(A)).
item(L,cl(C)) :- clausea(C), current(C;A), neg(A,C), not item(L,ma(A)), level(L).
item(L,cl(C)) :- clauseb(C), current(C;A), pos(A,C),     item(L,mb(A)).
item(L,cl(C)) :- clauseb(C), current(C;A), neg(A,C), not item(L,mb(A)), level(L).


% Compute delta for models at both levels
deltaContains(L,X) :- item(L,ma(X)), not item(L,mb(X)).
deltaContains(L,X) :- item(L,mb(X)), not item(L,ma(X)).

% Compare delta at both levels
smaller0 :- deltaContains(1,X), not deltaContains(0,X).
smaller1 :- deltaContains(0,X), not deltaContains(1,X).
:- smaller0. 

% Store relation information
item(1,subset) :- extend(1,S), childItem(S,subset).
item(1,subset) :- smaller1.
item(1,equal)  :- not item(1,subset).

% If there exists a counter-example for the solution being subset-minimal, remove it
:- root, extend(0,R), sub(R,S), childItem(S,subset), not killed(S).


#hide.
#show item/2.
#show extend/2.
#show levels/1.
