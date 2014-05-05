D-FLAT
======

D-FLAT is a framework that allows you to specify dynamic programming algorithms
that work on a tree decomposition in a declarative way by means of Answer Set
Programming (ASP).

A detailed description of the system is given in our [Technical Report DBAI-TR-2014-86, Technische Universität Wien, 2014] (http://dbai.tuwien.ac.at/research/report/dbai-tr-2014-86.pdf).


Please see our [project page]
(http://dbai.tuwien.ac.at/research/project/dflat/) for more information.

Compiling
---------

For instructions about compiling D-FLAT, please read the INSTALL file.

Running
-------

For getting a usage message that describes the invocation of D-FLAT and all
command-line options, use the *help* option:

    dflat -h

An example invocation of D-FLAT for solving the 3-Colorability problem on a
graph specified in the file `instance.lp`:

    dflat -p applications/graph_problems/3col/dynamic.lp -f instance.lp

Depending on the graph, this may produce the following output.

    Solutions:
    
    ┣━ map(0,red) map(1,green) map(2,blue) map(3,red) 
    ┣━ map(0,red) map(1,blue) map(2,green) map(3,red) 
    ┣━ map(0,green) map(1,red) map(2,blue) map(3,green) 
    ┣━ map(0,blue) map(1,red) map(2,green) map(3,blue) 
    ┣━ map(0,green) map(1,blue) map(2,red) map(3,green) 
    ┗━ map(0,blue) map(1,green) map(2,red) map(3,blue) 

The first line of the 3-Colorability encoding contains the following
*modeline*.

    %dflat: --tables -e vertex -e edge

A modeline such as this one has the effect of automatically adding the string
after the colon to the command-line arguments. It should contain arguments
without which running the encoding does not make sense. In this case,
`--tables` is used to tell D-FLAT's ASP solving module to use predicates like
`childRow/2` to describe a table for each tree decomposition node. The two
occurrences of `-e` specify that in the input graph (`instance.lp` in this
case) the respective predicates denote (hyper-)edges. (`vertex` is supplied
because isolated vertices would otherwise not be included in the tree
decomposition.)

As a further example, if it suffices to just count the number of solutions
without enumerating them, we can use the option `--depth 0`. Solutions (and
candidates for solutions) are represented in D-FLAT using so-called *item
trees*. For the 3-Colorability encoding, such an item tree always has an empty
root, and the children of this root represent the solution candidates.
Supplying `--depth i` has the effect that when D-FLAT has finished computing
the item tree at each decomposition node only item tree nodes up to level i are
materialized. In this case, only the (empty) root of the item tree of solutions
is materialized. Furthermore, the number of child nodes that have thus been
omitted is printed. For 3-Colorability, this number therefore amounts to the
number of solutions. The following is the output for the same input graph when
the argument `--depth 0` is added.

    Solutions:
    [6]

For monitoring what is going on during the computation, it is often useful to
print the generated decomposition using the option `--print-decomposition`.
Human-readable debugging output can be generated using the option `--output
human`.
