 graph G = makeGraph(list(1,2,3,4,5,6),list(list(1,2),list(3,6),list(4,5),list(1,6),list(2,3),list(5,6),list(3,4),list(1),list(2),list(5),list(4)));
    labeledgraph lG = labelGraph(G,0);
    labeledgraph lGelim = eliminateVariables(lG);
[DEBUG] ct = 4
[DEBUG] anzq = 7
[DEBUG] P = (QQ),(p(1),p(2),p(3),p(4),q(1),q(2),q(3),q(4),q(5),q(6),q(7)),(ip(11),C)
[DEBUG] R = (0,p(1),p(2),p(3),p(4)),(q(1),q(2),q(3),q(4),q(5),q(6),q(7)),(ip(7),C)
[DEBUG] lab = q(1),q(2),q(3),q(4),q(5),q(6),q(7),(p(1)),(p(2)),(p(3)),(p(4))
> lGelim;
[[1, 2], [3, 6], [4, 5], [1, 6], [2, 3], [5, 6], [3, 4], [1], [2], [5], [4]]
Graph with 6 vertices, 7 bounded edges and 4 unbounded edges

Edgeterms: 
[[1, 2] => q(1), [3, 6] => q(2), [4, 5] => -q(2)+q(1)+(p(1)+p(3)), [1, 6] => -q(1)+(-p(1)), [2, 3] => q(1)+(-p(2)), [5, 6] => -q(2)+q(1)+(p(1)), [3, 4] => -q(2)+q(1)+(-p(2)), [1] => (p(1)), [2] => (p(2)), [5] => (p(3)), [4] => (-p(1)-p(2)-p(3))]

>  def R = lGelim.over;
    setring R;
    ideal I = propagators(lGelim);
> I;
I[1]=q(1)^2
I[2]=q(2)^2
I[3]=q(2)^2-2*q(1)*q(2)+(-2*p(1)-2*p(3))*q(2)+q(1)^2+(2*p(1)+2*p(3))*q(1)+(2*p(1)*p(3))
I[4]=q(1)^2+(2*p(1))*q(1)
I[5]=q(1)^2+(-2*p(2))*q(1)
I[6]=q(2)^2-2*q(1)*q(2)+(-2*p(1))*q(2)+q(1)^2+(2*p(1))*q(1)
I[7]=q(2)^2-2*q(1)*q(2)+(2*p(2))*q(2)+q(1)^2+(-2*p(2))*q(1)
> 