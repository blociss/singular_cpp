 eliminateVariables(lG1);
[DEBUG] Entering eliminateVariables
polynomial ring, over a field, global ordering
// coefficients: QQ(p(1), p(2), p(3), p(4))
// number of vars : 7
//        block   1 : ordering ip
//                  : names    q(1) q(2) q(3) q(4) q(5) q(6) q(7)
//        block   2 : ordering C
ring RP=G.overpoly: 
polynomial ring, over a field, global ordering
// coefficients: QQ
// number of vars : 11
//        block   1 : ordering ip
//                  : names    p(1) p(2) p(3) p(4) q(1) q(2) q(3) q(4) q(5) q(6) q(7)
//        block   2 : ordering C
ring R=G.over: 
polynomial ring, over a field, global ordering
// coefficients: QQ(p(1), p(2), p(3), p(4))
// number of vars : 7
//        block   1 : ordering ip
//                  : names    q(1) q(2) q(3) q(4) q(5) q(6) q(7)
//        block   2 : ordering C
Balancing ideal computed
q(4)+q(1)+(p(1)),
q(5)-q(1)+(p(2)),
q(7)-q(5)+q(2),
-q(7)+q(3)+(p(4)),
q(6)-q(3)+(p(3)),
-q(6)-q(4)-q(2),
(p(1)+p(2)+p(3)+p(4))
ring RP=setring RP: 
polynomial ring, over a field, global ordering
// coefficients: QQ
// number of vars : 11
//        block   1 : ordering ip
//                  : names    p(1) p(2) p(3) p(4) q(1) q(2) q(3) q(4) q(5) q(6) q(7)
//        block   2 : ordering C
ideal I before mapping
`I`
ideal I after mapping
q(4)+q(1)+p(1),
q(5)-q(1)+p(2),
q(7)-q(5)+q(2),
-q(7)+q(3)+p(4),
q(6)-q(3)+p(3),
-q(6)-q(4)-q(2),
p(4)+p(3)+p(2)+p(1)
computing Std ideal
p(4)+p(3)+p(2)+p(1),
q(3)+q(2)-q(1)-p(3)-p(1),
q(4)+q(1)+p(1),
q(5)-q(1)+p(2),
q(6)+q(2)-q(1)-p(1),
q(7)+q(2)-q(1)+p(2)
 labelgraph G1 computed
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => q(3), [1, 6] => q(4), [2, 3] => q(5), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (p(4))]
 computing ld and ta
p(4)+p(3)+p(2)+p(1)
p(4)
-p(3)-p(2)-p(1)
 computing after mapping
(p(4))
(-p(1)-p(2)-p(3))
G1 before substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => q(3), [1, 6] => q(4), [2, 3] => q(5), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (p(4))]
G1 after substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => q(3), [1, 6] => q(4), [2, 3] => q(5), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
 computing ld and ta
q(3)+q(2)-q(1)-p(3)-p(1)
q(3)
-q(2)+q(1)+p(3)+p(1)
 computing after mapping
q(3)
-q(2)+q(1)+(p(1)+p(3))
G1 before substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => q(3), [1, 6] => q(4), [2, 3] => q(5), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
G1 after substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => q(4), [2, 3] => q(5), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
 computing ld and ta
q(4)+q(1)+p(1)
q(4)
-q(1)-p(1)
 computing after mapping
q(4)
-q(1)+(-p(1))
G1 before substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => q(4), [2, 3] => q(5), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
G1 after substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(5), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
 computing ld and ta
q(5)-q(1)+p(2)
q(5)
q(1)-p(2)
 computing after mapping
q(5)
q(1)+(-p(2))
G1 before substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(5), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
G1 after substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
 computing ld and ta
q(6)+q(2)-q(1)-p(1)
q(6)
-q(2)+q(1)+p(1)
 computing after mapping
q(6)
-q(2)+q(1)+(p(1))
G1 before substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => q(6), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
G1 after substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => -q(2)+q(1)+(p(1)), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
 computing ld and ta
q(7)+q(2)-q(1)+p(2)
q(7)
-q(2)+q(1)-p(2)
 computing after mapping
q(7)
-q(2)+q(1)+(-p(2))
G1 before substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => -q(2)+q(1)+(p(1)), [3, 4] => q(7), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
G1 after substituteGraph
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => -q(2)+q(1)+(p(1)), [3, 4] => -q(2)+q(1)+(-p(2)), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => -q(2)+q(1)+(p(1)), [3, 4] => -q(2)+q(1)+(-p(2)), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]
