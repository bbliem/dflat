% This is just for demonstration. Use the default implementation instead.
1 { chosenChildTupleL(I) : childTupleL(I) } 1.
1 { chosenChildTupleR(I) : childTupleR(I) } 1.
:- chosenChildTupleL(L), chosenChildTupleR(R), mapped(L,X,A), not mapped(R,X,A).
map(X,V) :- chosenChildTupleL(L), mapped(L,X,V).

#hide.
#show map/2.
#show chosenChildTupleL/1.
#show chosenChildTupleR/1.
