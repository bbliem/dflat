clause(C) :- pos(C,_).
clause(C) :- neg(C,_).
atom(A) :- pos(_,A).
atom(A) :- neg(_,A).

1 { true(A), false(A) } 1 :- atom(A).

sat(C) :- pos(C,A), true(A).
sat(C) :- neg(C,A), false(A).

:- clause(C), not sat(C).

#hide.
#show true/1.
