color(red;green;blue).
minColor(red).

levels(2).

:- edge(X,Y), item(L,map(X;Y,C)).

1 { extend(0,R) : childRow(R,N) } 1 :- childNode(N).
1 { extend(1,S) : sub(R,S) } 1 :- extend(0,R).

item(L,map(V,C)) :- extend(L,S), childItem(S,map(V,C)), current(V).

1 { item(L,map(V,C)) : color(C) } 1 :- introduced(V), L = 0..1.

% Only join matching colorings
:- item(L,map(V,C0;C1)), C0 != C1.

% Level 1 coloring must not be bigger (w.r.t. minimized color) than level 0
:- minColor(C), item(1,map(V,C)), not item(0,map(V,C)).

item(1,smaller) :- extend(1,S), childItem(S,smaller).
item(1,smaller) :- minColor(C), item(0,map(V,C)), not item(1,map(V,C)).

:- root, extend(0,R), sub(R,S), childItem(S,smaller).



#hide.
#show extend/2.
#show levels/1.
#show item/2.
