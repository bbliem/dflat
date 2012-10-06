oldM(M,A) :- oldMAtom(M,A).
oldM(M,R) :- oldMRule(M,R).

removed(X) :- before(X), not current(X).

guessable(A) :- current(A), atom(A).
guessable(R) :- current(R), before(R), rule(R).
0 {guess(X)} 1 :- guessable(X).

% Add enforced rules made true by the guess
mRule(R) :- head(R,A), current(R), guess(A).
mRule(R) :- neg(R,A), current(R), guess(A).
mRule(R) :- pos(R,A), current(R), guessable(A), not guess(A).

mAtom(A) :- guess(A), atom(A).
mRule(R) :- guess(R), rule(R).

noMatch(M) :- oldMi(M), removed(R), rule(R), not oldMRule(M,R). % Killed tuples do not match
noMatch(M) :- before(X), guess(X), oldMi(M), not oldM(M,X). % What was guessed to true cannot have been false before
noMatch(M) :- guessable(X), not guess(X), oldMi(M), oldM(M,X). % What guessed to false cannot have been true before

%%%% Version 1:
%% lt, succ, inf, sup
%lt(M,N) :- oldMi(M), oldMi(N), M < N.
%not_succ(M,O) :- lt(M,N), lt(N,O).
%succ(M,N) :- lt(M,N), not not_succ(M,N).
%not_sup(M) :- lt(M,N).
%sup(M) :- oldMi(M), not not_sup(M).
%not_inf(N) :- lt(M,N).
%inf(M) :- oldMi(M), not not_inf(M).
%
%% noMatchUpTo(I) is true iff among the first I tuples there is no valid predecessor of the current guess
%noMatchUpTo(M) :- inf(M), noMatch(M).
%noMatchUpTo(N) :- succ(M,N), noMatchUpTo(M), noMatch(N).
%:- sup(M), noMatchUpTo(M).

%%% Version 2:
nonLeaf :- oldMi(M).
match :- oldMi(M), not noMatch(M).
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
#show mRule/1.
%#show matchUpTo/1.
%#show sup/1.
