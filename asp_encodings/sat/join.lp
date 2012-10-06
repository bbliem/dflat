atom(A) :- pos(_,A).
atom(A) :- neg(_,A).

1 { chosenChildTupleL(I) : childTupleL(I) } 1.
1 { chosenChildTupleR(I) : childTupleR(I) } 1.

:- mapped(L,A,true), mapped(R,A,false), atom(A), chosenChildTupleL(L), chosenChildTupleR(R).
:- mapped(L,A,false), mapped(R,A,true), atom(A), chosenChildTupleL(L), chosenChildTupleR(R).

map(X,true) :- chosenChildTupleL(I), mapped(I,X,true).
map(X,true) :- chosenChildTupleR(I), mapped(I,X,true).
map(X,false) :- current(X), not map(X,true).



#hide.
#show map/2.
#show chosenChildTupleL/1.
#show chosenChildTupleR/1.
