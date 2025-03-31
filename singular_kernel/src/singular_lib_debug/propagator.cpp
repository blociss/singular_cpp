
    >  graph G = makeGraph(list(1,2,3,4,5,6),list(list(1,2),list(3,6),list(4,5),list(1,6),list(2,3),list(5,6),list(3,4),list(1),list(2),list(5),list(4)));
    labeledgraph lG = labelGraph(G,0);
    labeledgraph G1 = eliminateVariables(lG);
    > G1;
    [[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
    Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

    Edgeterms: 
    [[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => -q(2)+q(1)+(p(1)), [3, 4] => -q(2)+q(1)+(-p(2)), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]

    >   labeledgraph G2 = removeElimVars(G1);
    [DEBUG] Starting removeElimVars
    [DEBUG] R =G.over = (0,p(1),p(2),p(3),p(4)),(q(1),q(2),q(3),q(4),q(5),q(6),q(7)),(ip(7),C)
    [DEBUG] RP =G.overpoly = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4),q(5),q(6),q(7)),(ip(11),C)
    [DEBUG] el =G.elimvars = (p(4)),q(3),q(4),q(5),q(6),q(7)
    [DEBUG] lb =G.labels = q(1),q(2),-q(2)+q(1)+(p(1)+p(3)),-q(1)+(-p(1)),q(1)+(-p(2)),-q(2)+q(1)+(p(1)),-q(2)+q(1)+(-p(2)),(p(1)),(p(2)),(p(3)),(-p(1)-p(2)-p(3))
    [DEBUG] el[i] = (p(4))
    [DEBUG] rvar(el[i]) = 3
    [DEBUG] rvar(el[i]) = 4
    [DEBUG] rvar(el[i]) = 5
    [DEBUG] rvar(el[i]) = 6
    [DEBUG] rvar(el[i]) = 7
    [DEBUG] iv = 3,4,5,6,7
    [DEBUG] ip = 4
    [DEBUG] R1 after removing variable 7 = (0,p(1),p(2),p(3),p(4)),(q(1),q(2),q(3),q(4),q(5),q(6)),(ip(6),C)
    [DEBUG] R1 after removing variable 6 = (0,p(1),p(2),p(3),p(4)),(q(1),q(2),q(3),q(4),q(5)),(ip(5),C)
    [DEBUG] R1 after removing variable 5 = (0,p(1),p(2),p(3),p(4)),(q(1),q(2),q(3),q(4)),(ip(4),C)
    [DEBUG] R1 after removing variable 4 = (0,p(1),p(2),p(3),p(4)),(q(1),q(2),q(3)),(ip(3),C)
    [DEBUG] R1 after removing variable 3 = (0,p(1),p(2),p(3),p(4)),(q(1),q(2)),(ip(2),C)
    [DEBUG] R1 after removing parameter 4 = (0,p(1),p(2),p(3)),(q(1),q(2)),(ip(2),C)
    [DEBUG] el[i] = p(4)
    [DEBUG] el[i] = q(3)
    [DEBUG] el[i] = q(4)
    [DEBUG] el[i] = q(5)
    [DEBUG] el[i] = q(6)
    [DEBUG] el[i] = q(7)
    [DEBUG] RP1 before removing variable 11 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4),q(5),q(6),q(7)),(ip(11),C)
    [DEBUG] RP1 after removing variable 11 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4),q(5),q(6)),(ip(10),C)
    [DEBUG] RP1 before removing variable 10 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4),q(5),q(6)),(ip(10),C)
    [DEBUG] RP1 after removing variable 10 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4),q(5)),(ip(9),C)
    [DEBUG] RP1 before removing variable 9 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4),q(5)),(ip(9),C)
    [DEBUG] RP1 after removing variable 9 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4)),(ip(8),C)
    [DEBUG] RP1 before removing variable 8 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4)),(ip(8),C)
    [DEBUG] RP1 after removing variable 8 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3)),(ip(7),C)
    [DEBUG] RP1 before removing variable 7 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3)),(ip(7),C)
    [DEBUG] RP1 after removing variable 7 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2)),(ip(6),C)
    [DEBUG] RP1 before removing variable 4 = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2)),(ip(6),C)
    [DEBUG] RP1 after removing variable 4 = (QQ),(p(1),p(2),p(3),q(1),q(2)),(ip(5),C)
    [DEBUG] tr = q(1),q(2),-q(2)+q(1)+(p(1)+p(3)),-q(1)+(-p(1)),q(1)+(-p(2)),-q(2)+q(1)+(p(1)),-q(2)+q(1)+(-p(2)),(p(1)),(p(2)),(p(3)),(-p(1)-p(2)-p(3))
    > G2;
    [[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
    Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

    Edgeterms: 
    [[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => -q(2)+q(1)+(p(1)), [3, 4] => -q(2)+q(1)+(-p(2)), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]

    >   ring R= G2.over;
    setring R;
    > R;
    // coefficients: QQ(p(1), p(2), p(3))
    // number of vars : 2
    //        block   1 : ordering ip
    //                  : names    q(1) q(2)
    //        block   2 : ordering C
    > 