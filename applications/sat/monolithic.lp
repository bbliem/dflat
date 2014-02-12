1 { map(A,true); map(A,false) } 1 :- atom(A).

sat(C) :- pos(C,A), map(A,true).
sat(C) :- neg(C,A), map(A,false).

:- clause(C), not sat(C).



#show map/2.
