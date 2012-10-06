%order(5,1,2).
%order(1,2,4).
%order(7,4,3).
%order(3,2,8).

%order(1,2,3).
%order(2,3,4).
%order(1,3,4).

%order(1,2,3).
%order(2,3,4).
%order(2,3,5).
%order(4,5,6).
%order(6,7,8).

vertex(V) :- order(V,_,_).
vertex(V) :- order(_,V,_).
vertex(V) :- order(_,_,V).

1 { map(V,1..N) } 1 :- vertex(V), N = #count { vertex(_) }.
:- map(V1,K), map(V2,K), V1 < V2. % 1-to-1 mapping

lt(V1,V2) :- map(V1,K1), map(V2,K2), K1 < K2.
sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).

:- order(A,B,C), not sat(A,B,C).

#hide.
#show map/2.

% The following rules instead of the ones that derive sat/3 don't use lt/2 and are much slower:
%sat(A,B,C) :- order(A,B,C), map(A,KA), map(B,KB), map(C,KC), KA < KB, KB < KC.
%sat(A,B,C) :- order(A,B,C), map(A,KA), map(B,KB), map(C,KC), KB < KC, KC < KA.
%sat(A,B,C) :- order(A,B,C), map(A,KA), map(B,KB), map(C,KC), KC < KA, KA < KB.

% The following version is also much slower
%1 { sup(V) : vertex(V) } 1.
%1 { succ(A,B) : vertex(B) } 1 :- vertex(A), not sup(A).
%:- succ(V,V).
%:- succ(A,V), succ(B,V), A != B.
%lt(A,B) :- succ(A,B).
%lt(A,B) :- lt(A,V), succ(V,B).
%sat(A,B,C) :- order(A,B,C), lt(A,B), lt(B,C).
%sat(A,B,C) :- order(A,B,C), lt(B,C), lt(C,A).
%sat(A,B,C) :- order(A,B,C), lt(C,A), lt(A,B).
