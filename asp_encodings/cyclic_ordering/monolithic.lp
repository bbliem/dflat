1 { map(V,1..N) } 1 :- vertex(V), N = #count { vertex(_) }.
:- map(V1,K), map(V2,K), V1 < V2. % 1-to-1 mapping

lt(V1,V2) :- map(V1,K1), map(V2,K2), K1 < K2.
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), not sat(A,B,C).

#hide.
#show map/2.

error :- order(V,_,_), not vertex(V).
error :- order(_,V,_), not vertex(V).
error :- order(_,_,V), not vertex(V).
#show error/0.
