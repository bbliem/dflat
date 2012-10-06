oldM(M,A) :- oldMAtom(M,A).
oldM(M,C) :- oldMClause(M,C).

%removed(X) :- before(X), not current(X).
before(X) :- removed(X).
before(X) :- current(X), not introduced(X).

guessable(A) :- current(A), atom(A).
guessable(C) :- current(C), before(C), clause(C).
0 {guess(X)} 1 :- guessable(X).

% Add enforced clauses made true by the guess
mClause(C) :- pos(C,A), current(C), guess(A).
mClause(C) :- neg(C,A), current(C), guessable(A), not guess(A).

mAtom(A) :- guess(A), atom(A).
mClause(C) :- guess(C), clause(C).

noMatch(M) :- childTuple(M), removed(C), clause(C), not oldMClause(M,C). % Killed tuples do not match
noMatch(M) :- before(X), guess(X), childTuple(M), not oldM(M,X). % What was guessed to true cannot have been false before
noMatch(M) :- guessable(X), not guess(X), childTuple(M), oldM(M,X). % What guessed to false cannot have been true before

%%%% Version 1:
%% lt, succ, inf, sup
%lt(M,N) :- childTuple(M), childTuple(N), M < N.
%not_succ(M,O) :- lt(M,N), lt(N,O).
%succ(M,N) :- lt(M,N), not not_succ(M,N).
%not_sup(M) :- lt(M,N).
%sup(M) :- childTuple(M), not not_sup(M).
%not_inf(N) :- lt(M,N).
%inf(M) :- childTuple(M), not not_inf(M).
%
%% noMatchUpTo(I) is true iff among the first I tuples there is no valid predecessor of the current guess
%noMatchUpTo(M) :- inf(M), noMatch(M).
%noMatchUpTo(N) :- succ(M,N), noMatchUpTo(M), noMatch(N).
%:- sup(M), noMatchUpTo(M).

%%% Version 2:
nonLeaf :- childTuple(M).
match :- childTuple(M), not noMatch(M).
:- nonLeaf, not match.



% TODO: Count aggregate instead of loop more efficient?
% Performance on test instance: [obsolete]
% Version 1: 8.43 s
% Version 2: 0.24 s

% Another (bigger) test instance:
% Version 2: 7.51 s
% Version 3: 7.57 s

% Conclusion: Looping much worse, no real difference between version 2 and 3.

% Version 2 on test instance: 7.71 s
% Enumeration/counting encoding on same instance: 35 s



#hide.
#show mAtom/1.
#show mClause/1.
%#show matchUpTo/1.
%#show sup/1.
