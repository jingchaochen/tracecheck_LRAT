/************************************************************************************
Copyright (c) 2018, Jingchao Chen (chen-jc@dhu.edu.cn)
June 12, 2018

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include <math.h>
#include "mtl/Sort.h"
#include "core/verify.h"
#include "utils/System.h"

#define ABS(x) ((x)>0 ? (x) : (-x) )
#define    SEG_SIZE  10000

using namespace traceCheck;

inline Lit makeLit(int lit) { return (lit > 0) ? mkLit(lit-1) : ~mkLit(-lit-1);}

//=================================================================================================
checker::checker() 
{

}

checker::~checker()
{
}

//=================================================================================================
// Minor methods:
Var checker::newVar()
{  int v = nVars();
    assigns  .push(l_Undef);
    trail    .capacity(v);
    seen     .push(0);
    seen     .push(0);
    return v;
}

void checker::cancelUntil()
{
    for (int c = trail.size()-1; c >=0; c--) assigns [var(trail[c])] = l_Undef;
    trail.clear();
}

void checker::uncheckedEnqueue(Lit p)
{   assigns[var(p)] = lbool(!sign(p));
    trail.push_(p);
}

void checker :: readtracefile(char * tracefile) 
{
#ifdef  __APPLE__
        traceFp  = fopen(tracefile, "r");
#else
        traceFp  = fopen64(tracefile, "r");
#endif
        if (traceFp == NULL) {
		fprintf(stderr, "c Error: cannot open the trace file: %s\n", tracefile);
		exit(-1);
	}
        filePos.clear();
        clauses.clear();
        off64_t curpos=0;
	int No;
        while(1) {
               	int ret=fscanf(traceFp, "%i", &No);
                if( ret == EOF) goto done;
                if(No<0) {
                     printf("c error clauseID %d is negative \n",No);
                     exit(1);
                }
                #ifdef  __APPLE__
        	     curpos = ftello(traceFp);
                #else 
                     curpos = ftello64(traceFp);
               	#endif
                int m=filePos.size();
                if( m%SEG_SIZE == 0) filebase.push(curpos);
                off64_t base = filebase[m/SEG_SIZE];
                filePos.push(int(curpos-base));
               	while (No>=clauses.size()) clauses.push(-1);
                clauses[No]=m;
                if(m%20000 == 19999){
                     printf("\b\rc read clauseID=%d ",No);
                     fflush(stdout);
                }
                while(1){
                      char c = fgetc(traceFp);
                      if(c == EOF) goto done;
                      if(c == '\n') break;
                }
 	}
done:
        printf("\n");
     
}

void checker :: readfile(int i)
{
     off64_t pos = filebase[i/SEG_SIZE]+filePos[i];
     #ifdef  __APPLE__
           fseeko(traceFp, pos,SEEK_SET);
     #else 
           fseeko64(traceFp, pos,SEEK_SET);
     #endif
        	
     vec <int>  cls;
     int cnt=0;
     while(1){
          int No;
          int ret=fscanf(traceFp, "%i", &No);
          if(ret == EOF) break;
          cls.push(No);
          if(No==0){
               cnt++;
               if(cnt==2) break;
          }
          else{
             if(cnt) continue;
             int v = ABS(No);
             while (v > nVars()) newVar();
          }
     }
     if(cls.size()==0 || cnt<2) {
          printf("error: format %d line is incorrect \n",i);
          exit(1);
     }  
     traceCls[i]=new int[cls.size()];
     for(int k=0; k<cls.size(); k++) traceCls[i][k]=cls[k];
}

int checker :: backwardCheck()
{   
    printf("c backward check\n");
    int cnt,j,k,lit,No,maxNo=filePos.size();
    bool emptyID=false;
    filePos.min_memory(maxNo);
    for(int i=0; i<maxNo; i++) traceCls.push(0);
    traceCls.min_memory(maxNo);
    bool change;
    Lit tmp=lit_Undef;
    vec <Lit> unitl;
    vec <Lit> lits;
    for(int i=clauses.size()-1; i>=0; i--){
          if(i%100 == 0){
                 printf("\b\rc verify clause ID=%d ....",i);
                 fflush(stdout);
          } 
          int m=clauses[i];
          if(m<0) continue;
          cancelUntil();
          if(traceCls[m]==0) readfile(m);
        
          int *cls=traceCls[m];
          for(j=0; cls[j]; j++);
          if(j==0){
                printf("\nc empty clause ID=%d \n", i);
                fflush(stdout);
                emptyID=true;
          }
          int abegin=j+1;
          int cend=abegin;
          int end=abegin+2,aend=abegin+2;
          for(k=abegin; No=cls[k]; k++){
               if(No<0){
                     static int tt=0;
                     if(tt++ < 20){
                         printf("c verify RAT ID=%d antID=%d \n",i, -No);
                         fflush(stdout);
                     }
                     for(int t=0; lit=cls[t]; t++) {
                          // printf("%d ",lit);
                          // fflush(stdout);
                           seen[toInt(makeLit(lit))]=1;
                     }
                     cnt=0;
                     int n=clauses[-No];
                     if(n<0) {
                         printf("\n ant ID is not present ");
                         goto error;
                     }
                     if(traceCls[n]==0) readfile(n);
                    
                    // printf("\nant:");
                      
                     int *acls=traceCls[n];
                     for(int t=0; lit=acls[t]; t++) {
                      //     printf("%d ",lit);
                      //     fflush(stdout);
                           int p=toInt(makeLit(lit))^1;
                           if(seen[p]){
                                 cnt++;
                                 seen[p]=0;
                           }
                     }
                     for(int t=0; lit=cls[t]; t++) seen[toInt(makeLit(lit))]=0;
                    // if(cnt>=2) printf(" success! ");
                    // else printf(" fail ? fail? fail?????????????????????????");     
                     goto next;
               }
               int n=clauses[No];
               if(n < 0){
                   printf("\n verify ID=%d ID=%d does not exist n=%d \n",i, No, n);
                   exit(1);
               }
               if(traceCls[n]==0) readfile(n);
          }
             
          end=aend=k;
          if(k==abegin) goto next; //original clause
         
          for(j=0; lit=cls[j]; j++){
                 Lit p=makeLit(lit);
                 if(assigns[var(p)] == l_Undef) uncheckedEnqueue(~p);
               //  else printf("\n %d is already assigned? \n",lit);
          }
//
          for(k=abegin; k<aend; k++){
                 if(cls[k]==i) continue;
                 int n=clauses[cls[k]];
                 int *acls=traceCls[n];
                 cnt=0;
                 for(j=0; lit=acls[j]; j++){
                      Lit p=makeLit(lit);
                      if( value(p) == l_False) continue;
                      cnt++; tmp=p;
                      if(cnt>1) break;
                  }
                  if(cnt==0) goto next;
                  if(cnt==1){
                      if( assigns[var(tmp)] == l_Undef) uncheckedEnqueue(tmp);
                  }
                  else break;
           }
 //          printf(" <%d %d %d ID=%d>",abegin,k,aend,i);
   //        fflush(stdout);
           abegin=k;
//       
          change=true;
          unitl.clear();
          while(change){
             change=false;
             for(k=abegin; k<aend; k++){
                 if(cls[k]==i) { 
                         printf("\n ant ID %d == verified ID ",i);
                         goto error;
                 }
                 int n=clauses[cls[k]]; // cls[k]:ant ID
                 int *acls=traceCls[n];
                 cnt=0;
                 for(j=0; lit=acls[j]; j++){
                      Lit p=makeLit(lit);
                      if( value(p) == l_False) continue;
                      cnt++; tmp=p;
                      if(cnt>1) break;
                  }
                  if(cnt==0) goto next;
                  if(cnt==1){
                      if( assigns[var(tmp)] == l_Undef) uncheckedEnqueue(tmp);
                      int t=cls[k]; cls[k]=cls[abegin]; cls[abegin]=t;
                      abegin++;
                      change=true;
                  }
              }
          }
error:
          printf("\nverify ID=%d sz=%d:",i,cend);
          for(k=0; k<cend && k<3; k++) printf("%d ",cls[k]);
          printf("\nabegin=%d aend=%d ",abegin,aend);
          for(k=abegin; k<aend && k<end && k<abegin+3; k++){
                 int n=clauses[cls[k]];
                 int *acls=traceCls[n];
                 printf("\nantID=%d:",cls[k]);
                 for(; *acls; acls++){
                     printf("%d",*acls);
                     Lit p=makeLit(*acls);
                     if( value(p) == l_False) printf("F ");
                     else  if( value(p) == l_True) printf("T ");
                           else printf("U ");
                 }
          }
          printf("\n unit.sz=%d:",unitl.size());
          sort(unitl);
          for(k=0; k<unitl.size() && k<3; k++) printf("%d ",toIntLit(unitl[k]));
        
          printf("\n <m=%d clsID=%d> is not verified, abegin=%d aend=%d end=%d\n",m,i,abegin,aend,end);
          exit(1);
   next:
         delete []traceCls[m];
         traceCls[m]=0;
    }
    printf("\n");
    if(!emptyID){
           printf("\nc No empty clause is verified\n");
           return 0;
    }
    printf("\nc LRAT proof is verified\n ");
    return 1;
}

