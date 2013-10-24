1 { item(map(V,1..N)) } 1 :- current(V), N = #count { current(_) }.
:- item(map(V0;V1,K)), V0 != V1.

lt(V0,V1) :- item(map(V0,K0)), item(map(V1,K1)), K0 < K1.
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), current(A;B;C), not sat(A,B,C).

% Now check if we can find a compatible predecessor in each child table
gtChild(R,V0,V1) :- childItem(R,map(V0,K0);map(V1,K1)), current(V0;V1), K0 > K1.
incompatible(R) :- lt(V0,V1), gtChild(R,V0,V1).
match(N) :- childRow(R,N), not incompatible(R).

:- childNode(N), not match(N).



#hide.
#show item/1.
