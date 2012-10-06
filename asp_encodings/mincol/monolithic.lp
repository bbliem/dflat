color(red;green;blue).
minColor(red).

1 { map(X,C) : color(C) } 1 :- vertex(X).
:- edge(X,Y), map(X,C), map(Y,C).

% Use metasp for subset-minimization
#minimize [ map(X,C) : minColor(C) ].

#hide.
#show map/2.
