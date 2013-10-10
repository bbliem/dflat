color(red;green;blue).
criticalColor(red).

length(2).

:- edge(X,Y), item(L,map(X;Y,C)).

1 { extend(0,R) : rootOf(R,N) } 1 :- childNode(N).
1 { extend(L+1,S) : sub(R,S) } 1 :- extend(L,R), L < 2.

item(L,map(V,C)) :- extend(L,S), childItem(S,map(V,C)), current(V).

1 { item(L,map(V,C)) : color(C) } 1 :- introduced(V), L = 1..2.

% Only join matching colorings
:- item(L,map(V,C0;C1)), C0 != C1.

% Level 1 coloring must not be bigger (w.r.t. critical color and critical vertices) than level 0
:- criticalColor(C), criticalVertex(V), item(2,map(V,C)), not item(1,map(V,C)).

item(2,smaller) :- extend(2,S), childItem(S,smaller).
item(2,smaller) :- criticalColor(C), criticalVertex(V), item(1,map(V,C)), not item(2,map(V,C)).

:- final, extend(1,R), sub(R,S), childItem(S,smaller).



#hide.
#show extend/2.
#show length/1.
#show item/2.
