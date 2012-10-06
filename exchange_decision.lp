introduced(X) :- current(X), not before(X).
removed(X) :- before(X), not current(X).

guessable(A) :- current(A), atom(A).
guessable(R) :- current(R), before(R), rule(R).
0 {guess(X)} 1 :- guessable(X).

enforced(R) :- head(R,A), current(R), guess(A).
enforced(R) :- neg(R,A), current(R), guess(A).
enforced(R) :- pos(R,A), current(R), guessable(A), not guess(A).

m(X) :- guess(X).
m(R) :- enforced(R).

% lt, succ, inf, sup
lt(M,N) :- oldMi(M), oldMi(N), M < N.
not_succ(M,O) :- lt(M,N), lt(N,O).
succ(M,N) :- lt(M,N), not not_succ(M,N).
not_sup(M) :- lt(M,N).
sup(M) :- oldMi(M), not not_sup(M).
not_inf(N) :- lt(M,N).
inf(M) :- oldMi(M), not not_inf(M).

% matchUpTo(I) is true iff among the first I tuples there is a valid predecessor of the current guess

% TODO: Performance with/without an extra predicate to factor out the common parts of the version with inf and succ?

noMatch(M) :- oldMi(M), removed(R), rule(R), not oldM(M,R). % Killed tuples do not match
noMatch(M) :- oldMi(M), before(X), not oldM(M,X), guess(X).
noMatch(M) :- oldMi(M), guessable(X), not guess(X), oldM(M,X).

matchUpTo(M) :- inf(M), not noMatch(M).
matchUpTo(N) :- succ(M,N), matchUpTo(M).
matchUpTo(N) :- succ(M,N), not matchUpTo(M), not noMatch(N).

:- sup(M), not matchUpTo(M).

% TODO: Count aggregate instead of loop more efficient?



#hide.
#show m/1.
#show matchUpTo/1.
#show sup/1.
