#include <libraries/test.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "test_intern.h"

        AROS_LH1(void, RecordTestFailure,
        
        AROS_LHA(struct TestSuite *, ts, A0),
        
        struct TestBase *, TestBase, 9, Test)
{
    AROS_LIBFUNC_INIT

    ts->ts_Count++;
    ts->ts_Failed++;

    if (ts->ts_Output != NULL)
        FPrintf(ts->ts_Output, "not ok %d\n", ts->ts_Count);

    return;

    AROS_LIBFUNC_EXIT
} /* DestroyTestSuite */
