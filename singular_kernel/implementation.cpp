
std::pair<int, lists> std_gpi(leftv arg1) {
    // Validate input argument
    if (!arg1 || !arg1->Data()) {
        std::cerr << "Error: Invalid input argument!" << std::endl;
        return {0, nullptr};
    }

    // Extract input list
    lists input = (lists)(arg1->Data());
    if (!input || input->nr < 3) {
        std::cerr << "Error: Input list is invalid or too small!" << std::endl;
        return {0, nullptr};
    }

    // Extract tmp list
    lists tmp = (lists)(input->m[3].Data());
    if (!tmp || tmp->nr < 0) {
        std::cerr << "Error: Invalid tmp list!" << std::endl;
        return {0, nullptr};
    }

    // Extract ideal M
    ideal M = (ideal)tmp->m[0].Data();
    if (!M) {
        std::cerr << "Error: Ideal M is null!" << std::endl;
        return {0, nullptr};
    }

    // Check size of M
    int p = IDELEMS(M);
    if (p <= 0) {
        std::cerr << "Error: Ideal M has no elements!" << std::endl;
        return {0, nullptr};
    }

    // Set pr dynamically based on M
    int pr = IDELEMS(M); // Use the size of M to determine pr
    if (pr > IDELEMS(M)) {
        std::cerr << "Error: pr exceeds the size of M!" << std::endl;
        return {0, nullptr};
    }

    // Initialize ideal L
    ideal L = idInit(pr, 1);
    if (!L) {
        std::cerr << "Error: Failed to initialize ideal L!" << std::endl;
        return {0, nullptr};
    }

    // Copy polynomials from M to L
    for (int i = 0; i < pr; i++) {
        if (!M->m[i] || !p_Test((poly)M->m[i], currRing)) {
            std::cerr << "Error: M->m[" << i << "] is invalid!" << std::endl;
            return {0, nullptr};
        }
        L->m[i] = pCopy(M->m[i]);
        std::cout << "L->m[" << i << "]: " << pString((poly)L->m[i]) << std::endl;
    }

    // Debugging output
    std::cout << "M->rank = " << M->rank << ", pr = " << pr << std::endl;
    std::cout << "L->rank after idInit: " << L->rank << std::endl;

   //ideal MM=kStd(L,currRing->qideal, testHomog, NULL);

    ideal Li = kStd(L, currRing->qideal,  testHomog,  NULL);
    if (!Li) {
        std::cerr << "Error: kStd returned null!" << std::endl;
        return {0, nullptr};
    }

    // Check size of Li
    int tt = IDELEMS(Li);
    std::cout << "Li_size: " << tt << std::endl;
    if (tt <= 0) {
        std::cerr << "Error: Li has no elements!" << std::endl;
        return {0, nullptr};
    }

    // Debugging output for Li
    for (int i = 0; i < tt; i++) {
        std::cout << "Li->m[" << i << "]: " << pString((poly)Li->m[i]) << std::endl;
    }

    // Prepare the output token
    lists output = (lists)omAlloc0Bin(slists_bin);
    output->Init(4);

    lists t = (lists)omAlloc0Bin(slists_bin);
    t->Init(2);
    t->m[0].rtyp = STRING_CMD; t->m[0].data = strdup("generators");
    t->m[1].rtyp = STRING_CMD; t->m[1].data = strdup("module_std");
    output->m[1].rtyp = LIST_CMD; output->m[1].data = t;
    output->m[0].rtyp = RING_CMD; output->m[0].data = currRing;
    output->m[2].rtyp = RING_CMD; output->m[2].data = currRing;

    t = (lists)omAlloc0Bin(slists_bin);
    t->Init(tt); // Use the size of Li to initialize t
    for (int i = 0; i < tt; i++) {
        t->m[i].rtyp = POLY_CMD; t->m[i].data = pCopy((poly)Li->m[i]);
    }
    output->m[3].rtyp = LIST_CMD; output->m[3].data = t;

    // Cleanup
    id_Delete(&L, currRing); // Cleanup L if no longer needed

    return {p, output};
}
