
#ifndef traceCheck_checker_h
#define traceCheck_checker_h

#include "mtl/Vec.h"
#include "mtl/Alg.h"
#include "utils/Options.h"
#include "core/checkerTypes.h"

#define _FILE_OFFSET_BITS 64
#define _LARGE_FILES

#define cNo_Undef 0x3fffffff

#ifdef  __APPLE__
   typedef  off_t  off64_t;
#endif

namespace traceCheck {

//=================================================================================================
class checker {
public:

    checker();
    ~checker();

    Var     newVar (); // Add a new variable 
    vec <off64_t> filebase;                                     // base position of clauses in the rup file
    vec <int> filePos;                                          // the position of clauses in the rup file
 
    FILE *traceFp;
    void  readtracefile(char * tracefile);
    vec<CRef>  clauses; 
    void  readfile(int i);
    vec<int *>  traceCls;
    int   backwardCheck();
    void  displayline(int *cls);

    lbool   value      (Lit p) const;       // The current value of a literal.
    int     nVars      ()      const { return assigns.size(); }     // The current number of variables.

    int       verbosity;
 
  protected:
    vec<lbool>          assigns;          // The current assignments.
    vec<Lit>            trail;            // Assignment stack; 
    vec<char>           seen;
    
    void     uncheckedEnqueue (Lit p);
    void     cancelUntil();
    vec<int> antecedents;
};
//=================================================================================================
inline lbool    checker::value         (Lit p) const   { return assigns[var(p)] ^ sign(p); }

}

#endif
