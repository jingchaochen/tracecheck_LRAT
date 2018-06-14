
#include <errno.h>

#include <signal.h>
#include <zlib.h>
#include <sys/stat.h>

#include "utils/System.h"
#include "utils/ParseUtils.h"
#include "utils/Options.h"
#include "core/verify.h"

using namespace traceCheck;

BoolOption    forward ("forward mode", "f",    "foreward check", false);

//=================================================================================================

int main(int argc, char** argv)
{
    printf("c\nc verify tracecheck file in LRAT format\nc\n");
    try {
        setUsageHelp("c USAGE: %s <trace-file> <OPTIONS>\n\n");
        
        parseOptions(argc, argv, true);
        int ret=0;
        checker S;
        double initial_time = cpuTime();
        S.readtracefile(argv[1]);
        S.backwardCheck();
     
        double check_time = cpuTime();
        printf("c |  check time:  %12.2f s |\n", check_time - initial_time);
        return ret;
    } catch (OutOfMemoryException&){
                printf("c INDETERMINATE\n");
                exit(0);
    }
}

