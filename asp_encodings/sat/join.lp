1 { chosenChildTupleL(I) : childTupleL(I) } 1.
1 { chosenChildTupleR(I) : childTupleR(I) } 1.

:- mapped(L,A,X), mapped(R,A,Y), atom(A), chosenChildTupleL(L), chosenChildTupleR(R), X != Y.

map(X,true) :- chosenChildTupleL(I), mapped(I,X,true).
map(X,true) :- chosenChildTupleR(I), mapped(I,X,true).
map(X,false) :- current(X), not map(X,true).



#hide.
#show map/2.
#show chosenChildTupleL/1.
#show chosenChildTupleR/1.
