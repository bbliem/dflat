1 { map(A,true), map(A,false) } 1 :- atom(A).

sat(C) :- pos(C,A), map(A,true).
sat(C) :- neg(C,A), map(A,false).

:- clause(C), not sat(C).

#hide.
#show map/2.

error :- pos(C,_), not clause(C).
error :- neg(C,_), not clause(C).
error :- pos(_,A), not atom(A).
error :- neg(_,A), not atom(A).
#show error/0.
