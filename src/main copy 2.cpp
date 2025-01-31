#include <singular/Singular/libsingular.h>
#include <iostream>
#include <cstring>  // For strdup

// Function to convert a polynomial to a Singular list of monomials
lists pOLY_List(poly f0) 
{
    if (f0 == NULL) return NULL; // Handle NULL input

    int r = pLength(f0);
    lists S = (lists)omAlloc0Bin(slists_bin);
    S->Init(r);

    for (int k = 0; k < r; k++) 
    {
        S->m[k].rtyp = POLY_CMD;  // Ensure correct type

        poly headTerm = pHead(f0);
        if (headTerm != NULL) 
        {
            S->m[k].data = (void*) p_Copy(headTerm, currRing); // Deep copy
        } 
        else 
        {
            std::cout << "Warning: pHead(f0) returned NULL at index " << k << std::endl;
        }

        f0 = pNext(f0);
        if (f0 == NULL) break;
    }

    return S;
}

int main()
{
  // Initialize Singular
  siInit((char *)"/home/atraore/singular-gpispace/spack/opt/spack/linux-ubuntu22.04-skylake/gcc-11.3.0/singular-snapshot_22_03-5jvwtprazqirywu2triw6rprjazzi3so/lib/libSingular.so");

  // Construct the ring Z/32003[x,y,z]
  // The variable names
  char **n = (char**)omalloc(3 * sizeof(char*));
  n[0] = omStrDup("x");
  n[1] = omStrDup("y");
  n[2] = omStrDup("z2");

  ring R = rDefault(32003, 3, n);
  // Make R the default ring
  rChangeCurrRing(R);

  // Create the polynomial 1
  poly p1 = p_ISet(1, R);

  // Create the polynomial 2*x^3*z^2
  poly p2 = p_ISet(2, R);
  pSetExp(p2, 1, 3);
  pSetExp(p2, 2, 4);
  pSetExp(p2, 3, 2);
  pSetm(p2);

  // Print p1 + p2
  pWrite(p1); 
  printf(" + \n");
  pWrite(p2); 
  printf("\n");

  // Compute p1 + p2
  p1 = p_Add_q(p1, p2, R);
  p2 = NULL;
  pWrite(p1);

  // Convert p1 to a list of monomials
  lists monomialList = pOLY_List(p1);

  // Loop through the list and print each monomial
  if (monomialList != NULL) 
  {
      for (int i = 0; i < pLength(p1); i++) 
      {
          std::cout << "Monomial " << i << ": ";
          pWrite((poly)monomialList->m[i].data);  // Print the monomial
          std::cout << std::endl;
      }
  }

  // Clean up
  pDelete(&p1);
  rKill(R);

  // Finish up with Singular interpreter
  currentVoice = feInitStdin(NULL);
  int err = iiAllStart(NULL, "int ver=system(\"version\");\n", BT_proc, 0);
  if (err) 
      errorreported = 0; // Reset error handling
  printf("Interpreter returns %d\n", err);

  return 0;
}
