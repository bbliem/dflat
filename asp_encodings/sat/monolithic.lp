clause(C) :- pos(C,A).
clause(C) :- neg(C,A).
atom(A) :- pos(C,A).
atom(A) :- neg(C,A).

1 { true(A), false(A) } 1 :- atom(A).

sat(C) :- pos(C,A), true(A).
sat(C) :- neg(C,A), false(A).

:- clause(C), not sat(C).

#hide.
#show true/1.
