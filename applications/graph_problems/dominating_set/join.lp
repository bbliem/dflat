1 { extend(R) : childRow(R,N) } 1 :- childNode(N).
:- extend(R;S), childItem(R,in(X)), not childItem(S,in(X)).
item(in(X)) :- extend(R), childItem(R,in(X)).
item(dominated(X)) :- extend(R), childItem(R,dominated(X)).

sumCosts(CR + CS) :- extend(R;S), R < S, childCost(R,CR), childCost(S,CS).
subtract(C) :- C = #count{ item(in(_)) }.
cost(Sum - Sub) :- sumCosts(Sum), subtract(Sub).

#hide.
#show extend/1.
#show item/1.
#show cost/1.
