
#include <Singular/libsingular.h>


#include <interface/template_interface.hpp>
#include <interface/WorkflowResult.hpp>
#include <interface/ValuesOnPorts.hpp>
#include <interface/Workflow.hpp>
#include <stdexcept>
#include <unistd.h>

#include "config.hpp"
#include "singular_functions.hpp"
#include <chrono>
#include <tuple>
//#include "Singular/lists.h"
#include <typeinfo>

#include <stdio.h>
#include <string>

NO_NAME_MANGLING

std::string printGpiTokenContent(const std::string & tokenString, const std::string & needed_library)
{
    // Initialize and load the Singular library.
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);

    // Deserialize the token. Here 'worker()' returns an ID string.
    std::pair<int, lists> inputPair = deserialize(tokenString, worker());

    // Create a scoped left-value from the deserialized token.
    ScopedLeftv args(inputPair.first, lCopy(inputPair.second));
    lists Token = (lists)(args.leftV()->data);

    // Determine the number of elements in the token list.
    int L_size = lSize(Token) + 1;
    std::string outputContent;

    // Iterate over the list elements. 
    // In your system, element at index 3 holds the full content you wish to print.
    for (int i = 0; i < L_size; i++) {
        sleftv & listElement = Token->m[i];  // Access each element

        if (listElement.data != NULL) {
            // When i==3, print and store the element's string representation.
            if (i == 3) {
                outputContent = listElement.String();
                
            }
        }
        else {
            std::cout << "Token element at index " << i << " is NULL." << std::endl;
        }
    }
    return outputContent;
}

std::string singular_template_compute_StdBasis(std::string const& input_filename
    , std::string const& needed_library
)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> input;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    std::string out_filename1;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    input = deserialize(input_filename, ids);

    ScopedLeftv args(input.first, lCopy(input.second));
    //std::string function_name2 = "stdBasis";

    lists Token = (lists)(args.leftV()->data);

    int L_size = lSize(Token) + 1;
    for (int i = 0; i < L_size; i++) {
        sleftv& listElement = Token->m[i];  // Access each element as `leftv`
        if (listElement.data == NULL) {
           // std::cout << "Input: NULL" << std::endl;
        }
        else if (i == 3) {
            out_filename1 = listElement.String();
            // std::cout << "out_filename1= " << out_filename1 << std::endl;
        }
    }
    /*  // Example output filename for debug purposes
     std::cout << "Base filename: " << base_filename << std::endl;
     std::cout << "Output filename: " << out_filename1 << std::endl;
     std::cout << "Type of out_filename1: " << typeid(out_filename1).name() << std::endl; */

     //out = call_user_proc(function_name2, needed_library, args);
     //std::cout << "myout " << out.second << std::endl;
     //out_filename = serialize(out.second, base_filename);
     //std::cout << base_filename << std::endl;;


    return out_filename1;
}
std::string singular_getReducedIBPSystem_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "getReducedIBPSystem_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}


std::string singular_updateWeb_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "updateWeb_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}


std::string singular_reducedIBPs_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "reducedIBPs_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}
std::string singular_MI_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "MI_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

int singular_size_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);
    std::string out_filename1;

    ScopedLeftv args(Res.first, lCopy(Res.second));

    /*  lists Token = (lists)(args.leftV()->data);

     int L_size = lSize(Token) + 1;
     for (int i = 0; i < L_size; i++) {
         sleftv& listElement = Token->m[i];  // Access each element as `leftv`
         if (listElement.data == NULL) {
             std::cout << "Input: NULL" << std::endl;
         }

         out_filename1 = listElement.String();
         std::cout << "first= " << out_filename1 << std::endl;
     } */

    std::string function_name = "size_gpi";
    out = call_user_proc(function_name, needed_library, args);


    lists u = (lists)out.second->m[3].Data(); //token data

    // lists u = (lists)(args.leftV()->data);

 /*    int L_size1 = lSize(u) + 1;
    for (int i = 0; i < L_size1; i++) {
        sleftv& listElement = u->m[i];  // Access each element as `leftv`
        if (listElement.data == NULL) {
            std::cout << "Input: NULL" << std::endl;
        }

        out_filename1 = listElement.String();
        //  std::cout << "i=" << i << " " << "out_inside= " << out_filename1 << std::endl;


    } */
    sleftv& listElement = u->m[0];
    out_filename1 = listElement.String();

   // std::cout << "out_filename1= " << out_filename1 << std::endl;

    int ss = std::stoi(out_filename1);
    //std::cout << "ss " << ss << std::endl;
    out_filename = serialize(out.second, base_filename);

    return ss;
}

std::string singular_computeManyIBP_gpi(std::string const& res
    , std::string const& res1
    , int const& j
    , int const& k
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> Res1;
    std::pair<int, lists> Res2;
    std::pair<int, lists> Res3;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    void* p = (char*)(long)(j);
    void* M = (char*)(long)(k);



    Res = deserialize(res, ids);
    Res1 = deserialize(res1, ids);


    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv args1(args, Res1.first, lCopy(Res1.second));
    ScopedLeftv arg(args1, INT_CMD, p);
    ScopedLeftv argss(args1, INT_CMD, M);

    std::string function_name = "computeManyIBP_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::string singular_targetInts_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "targetInts_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::string singular_targetInt_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "targetInt_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}


std::string singular_IBP_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "IBP_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}
std::string singular_getRedIBPs_gpi(std::string const& res
    , int const& j
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    void* p = (char*)(long)(j);

    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv arg(args, INT_CMD, p);

    std::string function_name = "getRedIBPs_gpi";
    out = call_user_proc(function_name, needed_library, args);


    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

int singular_alreadyIntheList_gpi(std::string const& res
    , std::string const& res1
    , std::string const& needed_library
 )
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> Res1;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    std::string out_filename1;

    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;

    Res = deserialize(res, ids);
    Res1 = deserialize(res1, ids);


    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv args1(args, Res1.first, lCopy(Res1.second));

    std::string function_name = "alreadyIntheList_gpi";
    out = call_user_proc(function_name, needed_library, args);
    //out_filename = serialize(out.second, base_filename);

    lists u = (lists)out.second->m[3].Data(); //token data

    sleftv& listElement = u->m[0];
    out_filename1 = listElement.String();

    //std::cout << "out_filename1= " << out_filename1 << std::endl;

    int ss = std::stoi(out_filename1);
  //  std::cout << "alreadyIntheList_gpi " << ss << std::endl;

    return ss;
}

std::string singular_over_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "over_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::string singular_seed_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "seed_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::string singular_equal_gpi(std::string const& res
    , std::string const& res1
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> Res1;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;

    Res = deserialize(res, ids);
    Res1 = deserialize(res1, ids);


    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv args1(args, Res1.first, lCopy(Res1.second));

    std::string function_name = "equal_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::string singular_setIBP_gpi(int res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    void* p = (char*)(long)(res);



    ScopedLeftv arg(INT_CMD, p);
    std::string function_name = "setIBP_gpi";
    out = call_user_proc(function_name, needed_library, arg);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::string singular_indpndIBP_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "indpndIBP_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}
std::string singular_masterAndTailIntgrals_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "masterAndTailIntgrals_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}
std::string singular_Return_place(std::string const& res
    , int const& j
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    void* p = (char*)(long)(j);

    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv arg(args, INT_CMD, p);

    std::string function_name = "Return_place";
    out = call_user_proc(function_name, needed_library, args);


    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::string singular_updateWeb_later(std::string const& res
    , std::string const& res1
    , int const& j
    , int const& k
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> Res1;
    std::pair<int, lists> Res2;
    std::pair<int, lists> Res3;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    void* p = (char*)(long)(j);
    void* M = (char*)(long)(k);



    Res = deserialize(res, ids);
    Res1 = deserialize(res1, ids);


    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv args1(args, Res1.first, lCopy(Res1.second));
    ScopedLeftv arg(args1, INT_CMD, p);
    ScopedLeftv argss(args1, INT_CMD, M);

    std::string function_name = "updateWeb_later";
    out = call_user_proc(function_name, needed_library, args);

   /*   lists u = (lists)out.second->m[3].Data();
    //  lists u1 = (lists)out.second->m[3].Data(); //token data

       int L_size1 = lSize(u)+1;
        std::cout << "LeadSyz in implementation.cpp " << L_size1 << std::endl;
    for (int i = 0; i < L_size1; i++) {
        sleftv& listElement = u->m[i];  // Access each element as `leftv`
        if (listElement.data == NULL) {
            std::cout << "Input: NULL" << std::endl;
        }

       auto out_filename1 = listElement.String();
         std::cout << "i=" << i << " " << "out_inside= " << out_filename1 << std::endl;


    }  */

    out_filename = serialize(out.second, base_filename);


    return out_filename;
}


std::string vector2dToString(const std::vector<std::vector<int>>& vec) {
    std::stringstream ss;
    for (const auto& innerVec : vec) {
        for (int val : innerVec) {
            ss << val << ' ';
        }
        ss << '\n'; // Add a newline after each inner vector
    }
    return ss.str();
}

std::vector<std::vector<int>> stringToVector2d(const std::string& str) {
    std::vector<std::vector<int>> result;
    std::stringstream ss(str);
    std::string line;
    while (std::getline(ss, line)) {
        std::vector<int> innerVec;
        std::stringstream lineStream(line);
        int num;
        while (lineStream >> num) {
            innerVec.push_back(num);
        }
        result.push_back(innerVec);
    }
    return result;
}
std::vector<int> getFirstList(std::vector<std::vector<int>>& vec2d) {
    if (!vec2d.empty()) {
        std::vector<int> firstList = std::move(vec2d.front());
        vec2d.erase(vec2d.begin());
        return firstList;
    }
    return {};
}
std::pair<int, int> string_to_pair(const std::string& pair_str) {
    std::istringstream iss(pair_str);
    int first, second;
    char comma; 

    if (iss >> first >> comma && comma == ',' && iss >> second) {
        return std::make_pair(first, second); 
    }
    return std::make_pair(0, 0); 
}

std::string singular_replace_two(std::string const& res
    , int const& j
    , int const& k
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
  

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    void* p = (char*)(long)(j);
    void* M = (char*)(long)(k);



    Res = deserialize(res, ids);


    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv arg(args, INT_CMD, p);
    ScopedLeftv argss(args, INT_CMD, M);

    std::string function_name = "replace_two";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::string singular_computeManyIBPjk_gpi(std::string const& res
    , std::string const& res1
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> Res1;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;

    Res = deserialize(res, ids);
    Res1 = deserialize(res1, ids);


    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv args1(args, Res1.first, lCopy(Res1.second));

    std::string function_name = "computeManyIBPjk_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}
int singular_updateWeb_sizejk(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);
    std::string out_filename1;

    ScopedLeftv args(Res.first, lCopy(Res.second));

    /*  lists Token = (lists)(args.leftV()->data);

     int L_size = lSize(Token) + 1;
     for (int i = 0; i < L_size; i++) {
         sleftv& listElement = Token->m[i];  // Access each element as `leftv`
         if (listElement.data == NULL) {
             std::cout << "Input: NULL" << std::endl;
         }

         out_filename1 = listElement.String();
         std::cout << "first= " << out_filename1 << std::endl;
     } */

    std::string function_name = "updateWeb_sizejk";
    out = call_user_proc(function_name, needed_library, args);


    lists u = (lists)out.second->m[3].Data(); //token data

    // lists u = (lists)(args.leftV()->data);

 /*    int L_size1 = lSize(u) + 1;
    for (int i = 0; i < L_size1; i++) {
        sleftv& listElement = u->m[i];  // Access each element as `leftv`
        if (listElement.data == NULL) {
            std::cout << "Input: NULL" << std::endl;
        }

        out_filename1 = listElement.String();
        //  std::cout << "i=" << i << " " << "out_inside= " << out_filename1 << std::endl;


    } */
    sleftv& listElement = u->m[0];
    out_filename1 = listElement.String();

   // std::cout << "out_filename1= " << out_filename1 << std::endl;

    int ss = std::stoi(out_filename1);
    //std::cout << "ss " << ss << std::endl;
    out_filename = serialize(out.second, base_filename);

    return ss;
}

std::string singular_computeM1_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));
    std::string function_name = "computeM1_gpi";
    out = call_user_proc(function_name, needed_library, args);
  /*  lists u = (lists)out.second->m[3].Data();
    //  lists u1 = (lists)out.second->m[3].Data(); //token data

       int L_size1 = lSize(u)+1;
        std::cout << "computeM1_gpi " << L_size1 << std::endl;
    for (int i = 0; i < L_size1; i++) {
        sleftv& listElement = u->m[i];  // Access each element as `leftv`
        if (listElement.data == NULL) {
            std::cout << "Input: NULL" << std::endl;
        }

       auto out_filename1 = listElement.String();
         std::cout << "i=" << i << " " << "out_inside= " << out_filename1 << std::endl;


    }   */
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}



std::string singular_computeM2_gpi(std::string const& res
    , std::string const& res1
    , int const& j
    , int const& k
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> Res1;
    std::pair<int, lists> Res2;
    std::pair<int, lists> Res3;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;
    void* p = (char*)(long)(j);
    void* M = (char*)(long)(k);



    Res = deserialize(res, ids);
    Res1 = deserialize(res1, ids);


    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv args1(args, Res1.first, lCopy(Res1.second));
    ScopedLeftv arg(args1, INT_CMD, p);
    ScopedLeftv argss(args1, INT_CMD, M);

    std::string function_name = "computeM2_gpi";
    out = call_user_proc(function_name, needed_library, args);

   
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}

std::pair<int, lists> intersection_gpi(leftv arg1) {
   

    // Extract input list
    lists input = (lists)(arg1->Data());
   
        // Extract tmp list
    lists tmp = (lists)(input->m[3].Data());

    lists Tok = (lists)(arg1->next->Data()); // extract Tok
    lists tmp1 = (lists)(Tok->m[3].Data()); // Tok.data


  

    // Extract ideal M
    ideal M1 = (ideal)tmp->m[0].Data();
    ideal M2 = (ideal)tmp1->m[0].Data();

    ideal M = idSect(M1, M2);
    int size_M= IDELEMS( M);
    std::cout<<"size_of_module_intersection "<<size_M<<std::endl;
int p=1;
    // Prepare the output token
    lists output = (lists)omAlloc0Bin(slists_bin);
    output->Init(4);

    lists t = (lists)omAlloc0Bin(slists_bin);
    t->Init(2);
    t->m[0].rtyp = STRING_CMD; t->m[0].data = strdup("generators");
    t->m[1].rtyp = STRING_CMD; t->m[1].data = strdup("module_intersection");
    output->m[1].rtyp = LIST_CMD; output->m[1].data = t;
    output->m[0].rtyp = RING_CMD; output->m[0].data = currRing;
    output->m[2].rtyp = RING_CMD; output->m[2].data = currRing;

    t = (lists)omAlloc0Bin(slists_bin);
    t->Init(1); // Use the size of Li to initialize t
        t->m[0].rtyp = MODUL_CMD; t->m[0].data = M;

    output->m[3].rtyp = LIST_CMD; output->m[3].data = t;

    // Cleanup

    return {p, output};
}

std::string singular_intersection_gpi(std::string const& res
    , std::string const& res1
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);
    std::pair<int, lists> Res;
    std::pair<int, lists> Res1;

    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;
    ids = worker();
    //std::cout << ids << " in singular_..._compute" << std::endl;

    Res = deserialize(res, ids);
    Res1 = deserialize(res1, ids);


    ScopedLeftv args(Res.first, lCopy(Res.second));
    ScopedLeftv args1(args, Res1.first, lCopy(Res1.second));

    out = intersection_gpi(args.leftV());  // Call reduce_GPI with the raw pointer

   // std::string function_name = "intersection_gpi";
    //out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}



/* 


std::pair<int, lists> std_gpi(leftv arg1) {

    // std::cout << "Type of 1.arg:" << arg1->Typ() <<std::endl;
    lists input = (lists)(arg1->Data()); //extract input
   

lists tmp = (lists)(input->m[3].Data()); // input.data
ideal M= (ideal)tmp->m[0].Data();
int p = IDELEMS(M);
std::cout<<"M_size"<<p<<std::endl;
   // std::cout<<" M_element "<<pString((poly)M->m[14])<<std::endl;

int pr=13;
ideal L = idInit(pr, 1);  // Initialize an empty ideal (syzygy module)

  for (int i = 0; i < pr; i++) {
        L->m[i] = pCopy(M->m[i]);
//        std::cout << "L->m[" << i << "]: " << pString((poly)L->m[i]) << std::endl;
    }

std::cout << "Current ring: " << rString(currRing) << std::endl;
std::cout << "Is global ordering: " << rHasGlobalOrdering(currRing) << std::endl;

ideal Li=kStd(L,NULL, testHomog, NULL);     
int tt=IDELEMS(Li);
 std::cout<<" Li_size "<<tt<<std::endl;

    // Debugging output for Li
    for (int i = 0; i < tt; i++) {
        std::cout << "Li->m[" << i << "]: " << pString((poly)Li->m[i]) << std::endl;
    } 


   // Debugging output for Li
    for (int i = 0; i < tt; i++) {
        std::cout << "Li->m[" << i << "]: " << pString((poly)Li->m[i]) << std::endl;
    }

for(int i=0;i<p;i++){
}
for(int i=0;i<p;i++){
bool b= p_Test((poly)M->m[i],currRing);
 std::cout<<"i= "<<i<<" p_test "<<b<<std::endl;
 p_ContentForGB((poly)M->m[i], currRing);
std::cout << "p_ContentForGB called successfully" << std::endl;
poly po= p_Cleardenom((poly)M->m[i], currRing);
 std::cout<<"i= "<<i<<" poly "<<pString(po)<<std::endl;
p_ProjectiveUnique((poly)M->m[i], currRing);
std::cout << "p_ProjectiveUnique called successfully" << std::endl;

ideal Mi=kStd((ideal)M->m[i],currRing->qideal, testHomog, NULL);
 std::cout<<"i= "<<i<<" Mi_size "<<IDELEMS(Mi)<<std::endl;

} 



//ideal MM=kStd(L,currRing->qideal, testHomog, NULL);
    

// Prepare the output token
    lists output=(lists)omAlloc0Bin(slists_bin);
    output->Init(4);// type token
    // fieldnames
    lists t=(lists)omAlloc0Bin(slists_bin);
    t->Init(2);
    t->m[0].rtyp=STRING_CMD; t->m[0].data=strdup("generators");
    t->m[1].rtyp=STRING_CMD; t->m[1].data=strdup("module_std");
    output->m[1].rtyp=LIST_CMD; output->m[1].data=t;
     output->m[0].rtyp=RING_CMD; output->m[0].data=currRing;
      output->m[2].rtyp=RING_CMD; output->m[2].data=currRing;
    //data
    t=(lists)omAlloc0Bin(slists_bin);
    t->Init(1);
      t->m[0].rtyp=MODUL_CMD; t->m[0].data=Li;
          output->m[3].rtyp=LIST_CMD; output->m[3].data=t;
          return {p,output};

}
  */

  lists std_gpi(leftv arg1) {

    // Extract input list
    lists input = (lists)(arg1->Data());

    // Extract tmp list
    lists tmp = (lists)(input->m[3].Data());

    // Extract ideal M
    ideal M = (ideal)tmp->m[0].Data();

    // Compute Li
    ideal Li = kStd(M, currRing->qideal, testHomog, NULL);
    int tt = IDELEMS(Li);
    std::cout << "Li_size " << tt << std::endl;

    // Debugging output for Li
    for (int i = 0; i < tt; i++) {
        std::cout << "Li->m[" << i << "]: " << pString((poly)Li->m[i]) << std::endl;
    } 

    // Prepare the output token
lists output = (lists)omAlloc0Bin(slists_bin);
output->Init(4);

// Create and initialize the first sublist
lists t1 = (lists)omAlloc0Bin(slists_bin);
t1->Init(2);
t1->m[0].rtyp = STRING_CMD; t1->m[0].data = strdup("generators");
t1->m[1].rtyp = STRING_CMD; t1->m[1].data = strdup("module_std");

output->m[0].rtyp = RING_CMD; output->m[0].data = currRing;
output->m[1].rtyp = LIST_CMD; output->m[1].data = t1;
output->m[2].rtyp = RING_CMD; output->m[2].data = currRing;

// Create and initialize the second sublist for Li elements
lists t2 = (lists)omAlloc0Bin(slists_bin);
t2->Init(tt); // Use the size of Li to initialize t2
for (int i = 0; i < tt; i++) {
    t2->m[i].rtyp = POLY_CMD; 
    t2->m[i].data = pCopy((poly)Li->m[i]);
}

output->m[3].rtyp = LIST_CMD; output->m[3].data = t2;

    return output;
}



std::string singular_std_gpi(std::string const& res,
    std::string const& needed_library,
    std::string const& base_filename)
{
init_singular(config::singularLibrary().string());
load_singular_library(needed_library);

std::pair<int, lists> Res;
lists out;  // Change type from std::pair<int, lists> to lists
std::string ids;
std::string out_filename;

ids = worker();
Res = deserialize(res, ids);

ScopedLeftv args(Res.first, lCopy(Res.second));

// Call std_gpi directly and assign to lists type
out = std_gpi(args.leftV());  

// Serialize the output
out_filename = serialize(out, base_filename);

return out_filename;
}

std::string singular_assign_gpi(std::string const& res
    , std::string const& needed_library
    , std::string const& base_filename)
{
    init_singular(config::singularLibrary().string());
    load_singular_library(needed_library);

    std::pair<int, lists> Res;
    std::pair<int, lists> out;
    std::string ids;
    std::string out_filename;

    ids = worker();
    Res = deserialize(res, ids);

    ScopedLeftv args(Res.first, lCopy(Res.second));

    std::string function_name = "assign_gpi";
    out = call_user_proc(function_name, needed_library, args);
    out_filename = serialize(out.second, base_filename);

    return out_filename;
}