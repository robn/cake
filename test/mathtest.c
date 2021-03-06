#include <proto/exec.h>
#include <proto/aros.h>
#include <proto/dos.h>
#include <stdio.h>


double X_Mul(double x, double y)
{
  if (x==x) return x*y;
  return x*y;
}

/* to avoid casts these two lines are absolutely necessary!!*/
#define float LONG

#include <proto/mathffp.h>
#include <proto/mathtrans.h>
#include <proto/mathieeesingbas.h>
#include <proto/mathieeesingtrans.h>
#include <proto/mathieeedoubbas.h>
#include <proto/mathieeedoubtrans.h>

#include <stdio.h>

#include <exec/types.h>

struct Library * MathBase;
struct Library * MathTransBase;
struct Library * MathIeeeSingBasBase;
struct Library * MathIeeeSingTransBase;
struct Library * MathIeeeDoubBasBase;
struct Library * MathIeeeDoubTransBase;



int main(int argc, char ** argv)
{
    float FFPOne, FFPTwo, FFPOnehalf, FFPMinusOne, FFPNull;
    float SPOne, SPTwo, SPOnehalf;
    float float_res;
    LONG * float_resptr = (LONG *)&float_res;
    LONG * ptr;
    LONG wanted;
    double double_res, double_res2;
    double * Darg1, * Darg2;
    QUAD * double_resptr = (QUAD *)&double_res;
    QUAD * double_resptr2 = (QUAD *)&double_res2;
    QUAD QArg1, QArg2;

    Darg1 = (double *)&QArg1;
    Darg2 = (double *)&QArg2;

    #define DEF_FFPOne		0x80000041UL
    #define DEF_FFPTwo		0x80000042UL
    #define DEF_FFPMinusOne 	0x800000C1UL
    #define DEF_FFPOnehalf	0x80000040UL
    #define DEF_FFPNull		0x00000000UL

    #define DEF_SPOne		0x3f800000UL
    #define DEF_SPTwo		0x40000000UL
    #define DEF_SPOnehalf	0x3f000000UL
    
    #define DEF_DPOne		0x3ff0000000000000ULL
    #define DEF_DPMinusOne	0xbff0000000000000ULL
    #define DEF_DPTwo		0x4000000000000000ULL
    #define DEF_DPThree		0x4008000000000000ULL
    #define DEF_DPFour          0x4010000000000000ULL
    #define DEF_DPTwenty        0x4034000000000000ULL     
    
    ptr = (LONG *)&FFPOne; 	*ptr = DEF_FFPOne;
    ptr = (LONG *)&FFPTwo; 	*ptr = DEF_FFPTwo;
    ptr = (LONG *)&FFPMinusOne;	*ptr = DEF_FFPMinusOne;
    ptr = (LONG *)&FFPOnehalf;	*ptr = DEF_FFPOnehalf;
    ptr = (LONG *)&FFPNull; 	*ptr = DEF_FFPNull;
    
    ptr = (LONG *)&SPOne; 	*ptr = DEF_SPOne;
    ptr = (LONG *)&SPOnehalf; 	*ptr = DEF_SPOnehalf;
    ptr = (LONG *)&SPTwo; 	*ptr = DEF_SPTwo;

/* if you deactivate #define float LONG something very funny happens here:    
printf("two: %x <-> %x \n",*ptr,SPTwo);
printf("two: %x <-> %x \n",SPTwo,*ptr);
*/

#define CHECK(func, args, cres)                                                          \
    float_res = func args;                                                               \
    if (*float_resptr != cres)                                                           \
	printf ("FAIL: " #func " " #args " in line %d (got=0x%08lx expected=0x%08lx)\n", \
                 __LINE__, *float_resptr, cres);                                         \
    else                                                                                 \
	printf ("OK  : " #func " " #args "\n");

/*
  When using doubles or QUADs it is important to pay attention to the Endianess of the processor,
  otherwise you'll never see what you want to see.
 */

#define CHECK_DOUBLE1A(func, arg1, cres)                                                           \
    double_res = func (arg1);                                                                      \
    if (*double_resptr != cres)                                                                    \
    {                                                                                              \
      if (0 != AROS_BIG_ENDIAN)                                                                    \
      {                                                                                            \
	printf ("FAIL: " #func " " #arg1 " in line %d (got=0x%08lx%08lx expected=0x%08lx%08lx)\n", \
                 __LINE__,                                                                         \
                 (LONG)(((QUAD)*double_resptr)>>32),*(((LONG *)double_resptr)+1),                  \
                 (LONG)(((QUAD)cres)>>32),(LONG)cres);                                             \
      }                                                                                            \
      else                                                                                         \
      {                                                                                            \
	printf ("(little endian) FAIL: " #func " " #arg1 " in line %d (got=0x%08lx%08lx expected=0x%08lx%08lx)\n", \
                 __LINE__,                                                                         \
                 *(((LONG *)double_resptr)+1),*(((LONG *)double_resptr)),                          \
                 (LONG)(((QUAD)cres)>>32),(LONG)cres);                                             \
      }                                                                                            \
    }                                                                                              \
    else                                                                                           \
      printf ("OK  : " #func " " #arg1 "\n");                                                      
    
#define CHECK_DOUBLE1B(func, arg1, cres)                                                           \
    QArg1 = arg1;                                                                                  \
    double_res = func (*Darg1);                                                                    \
    if (*double_resptr != cres)                                                                    \
    {                                                                                              \
      if (0 != AROS_BIG_ENDIAN)                                                                    \
      {                                                                                            \
	printf ("FAIL: " #func " " #arg1 " in line %d (got=0x%08lx%08lx expected=0x%08lx%08lx)\n", \
                 __LINE__,                                                                         \
                 (LONG)(((QUAD)*double_resptr)>>32),*(((LONG *)double_resptr)+1),                  \
                 (LONG)(((QUAD)cres)>>32),(LONG)cres);                                             \
      }                                                                                            \
      else                                                                                         \
      {                                                                                            \
	printf ("(little endian) FAIL: " #func " " #arg1 " in line %d (got=0x%08lx%08lx expected=0x%08lx%08lx)\n", \
                 __LINE__,                                                                         \
                 *(((LONG *)double_resptr)+1),*(((LONG *)double_resptr)),                          \
                 (LONG)(((QUAD)cres)>>32),(LONG)cres);                                             \
      }                                                                                            \
    }                                                                                              \
    else                                                                                           \
      printf ("OK  : " #func " " #arg1 "\n");                                                      

#define CHECK_DOUBLE2A(func, arg1, arg2, cres)                                                     \
    double_res = func (arg1, arg2);                                                                \
    double_res2 = cres;                                                                            \
    if (double_res != double_res2)                                                                 \
    {                                                                                              \
      if (0 != AROS_BIG_ENDIAN)                                                                    \
      {                                                                                            \
	printf ("FAIL: " #func " (" #arg1 "," #arg2 ") in line %d (got=0x%08lx%08lx expected=0x%08lx%08lx)\n", \
                 __LINE__,                                                                         \
                 (LONG)(((QUAD)*double_resptr )>>32),*(((LONG *)double_resptr )+1),                \
                 (LONG)(((QUAD)*double_resptr2)>>32),*(((LONG *)double_resptr2)+1) );              \
      }                                                                                            \
      else                                                                                         \
      {                                                                                            \
	printf ("(little endian) FAIL: " #func " (" #arg1 "," #arg2 ") in line %d (got=0x%08lx%08lx expected=0x%08lx%08lx)\n", \
                 __LINE__,                                                                         \
                 *(((LONG *)double_resptr )+1),*(((LONG *)double_resptr)),                         \
                 *(((LONG *)double_resptr2)+1),*(((LONG *)double_resptr2)) );                      \
      }                                                                                            \
    }                                                                                              \
    else                                                                                           \
      printf ("OK  : " #func " ( " #arg1 "," #arg2 " ) \n");                                                      






    if (!(MathBase = OpenLibrary("mathffp.library", 0L)))
    {
	printf ("Couldn't open mathffp.library\n");
	return (0);
    }
	
    printf("Very basic mathffp functionality test...\n");

    /* this should set the zero-bit*/
    wanted = DEF_FFPNull;	CHECK(SPAbs,(0),wanted);

    wanted = DEF_FFPNull;	CHECK(SPFlt,(0),wanted);
    wanted = DEF_FFPOne;	CHECK(SPFlt,(1),wanted);
    wanted = DEF_FFPTwo;	CHECK(SPFlt,(2),wanted);
    wanted = DEF_FFPMinusOne;	CHECK(SPFlt,(-1),wanted);
    wanted = DEF_FFPTwo;	CHECK(SPAdd,(FFPOne, FFPOne),wanted);
    wanted = DEF_FFPOnehalf;	CHECK(SPDiv,(FFPTwo, FFPOne),wanted);
    wanted = DEF_FFPTwo;	CHECK(SPMul,(FFPOne, FFPTwo),wanted);
    CloseLibrary(MathBase);

    if (!(MathTransBase = OpenLibrary("mathtrans.library", 0L)))
    {
	fprintf (stderr, "Couldn't open mathtrans.library\n");
	return (0);
    }

    printf("Very basic mathtrans functionality test...\n");

    CHECK (SPLog,   (FFPTwo),     0xb1721840UL);
    CHECK (SPLog10, (FFPTwo),     0x9a209b3fUL);
    CHECK (SPSin,   (FFPOne),     0xd76aa440UL);
    CHECK (SPCos,   (FFPOne),     0x8a514040UL);
    CHECK (SPTan,   (FFPOne),     0xc7592341UL);
    CHECK (SPSinh,  (FFPOne),     0x966cfe41UL);
    CHECK (SPCosh,  (FFPOne),     0xc583aa41UL);
    CHECK (SPTanh,  (FFPOne),     0xc2f7d640UL);
    CHECK (SPExp,   (FFPTwo),     0xec732543UL);
    CHECK (SPAsin,  (FFPOnehalf), 0x860a9240UL);
    CHECK (SPAcos,  (FFPOnehalf), 0x860a9241UL);

    CloseLibrary(MathTransBase);

    if (!(MathIeeeSingBasBase = OpenLibrary("mathieeesingbas.library", 0L)))
    {
	printf ("Couldn't open mathieeesingbas.library\n");
	return (0);
    }


    if (!(MathIeeeSingTransBase = OpenLibrary("mathieeesingtrans.library", 0L)))
    {
	printf ("Couldn't open mathieeesingtrans.library\n");
	return (0);
    }

    printf("Very basic mathieeesingtrans functionality test...\n");


    CHECK (IEEESPLog,   (SPTwo),      0x3f317218UL);
    CHECK (IEEESPLog10, (SPTwo),      0x3e9a209aUL);
    CHECK (IEEESPSin,   (SPOne),      0x3f576aa4UL);
    CHECK (IEEESPCos,   (SPOne),      0x3f0a5140UL);
    CHECK (IEEESPTan,   (SPOne),      0x3fc75923UL);
    CHECK (IEEESPSinh,  (SPTwo),      0x40681e7bUL);
    CHECK (IEEESPCosh,  (SPTwo),      0x4070c7d1UL);
    CHECK (IEEESPTanh,  (SPOne),      0x3f42f7d6UL);
    CHECK (IEEESPExp,   (SPTwo),      0x40ec7325UL);
    CHECK (IEEESPAsin,  (SPOnehalf),  0x3f060a92UL);
    CHECK (IEEESPAcos,  (SPOnehalf),  0x3f860a92UL);

    CloseLibrary(MathIeeeSingTransBase);

    if (!(MathIeeeDoubBasBase = OpenLibrary("mathieeedoubbas.library", 0L)))
    {
	printf ("Couldn't open mathieeedoubbas.library\n");
	return (0);
    }
    


    CHECK_DOUBLE1A(IEEEDPFlt, ((LONG)1), DEF_DPOne);
    CHECK_DOUBLE1A(IEEEDPFlt, ((LONG)2), DEF_DPTwo);
    CHECK_DOUBLE1A(IEEEDPFlt, ((LONG)3), DEF_DPThree);
    CHECK_DOUBLE1A(IEEEDPFlt, ((LONG)20), DEF_DPTwenty);
    CHECK_DOUBLE1B(IEEEDPAbs, ((QUAD)DEF_DPMinusOne), (QUAD)DEF_DPOne);
    CHECK_DOUBLE1B(IEEEDPNeg, ((QUAD)DEF_DPMinusOne), (QUAD)DEF_DPOne);
    CHECK_DOUBLE2A(IEEEDPAdd, IEEEDPFlt(1),      IEEEDPFlt(1),      IEEEDPFlt(2));
    CHECK_DOUBLE2A(IEEEDPAdd, IEEEDPFlt(123456), IEEEDPFlt(654321), IEEEDPFlt(777777));
    CHECK_DOUBLE2A(IEEEDPSub, IEEEDPFlt(123456), IEEEDPFlt(654321), IEEEDPFlt(-530865));
    CHECK_DOUBLE2A(IEEEDPMul, IEEEDPFlt(321),    IEEEDPFlt(123456), IEEEDPFlt(39629376));
    CHECK_DOUBLE2A(IEEEDPMul, IEEEDPFlt(2),      IEEEDPFlt(2),      IEEEDPFlt(4));
    CHECK_DOUBLE2A(IEEEDPMul, IEEEDPFlt(20),     IEEEDPFlt(20),     IEEEDPFlt(400));
    

    if (!(MathIeeeDoubTransBase = OpenLibrary("mathieeedoubtrans.library", 0L)))
    {
	printf ("Couldn't open mathieeedoubtrans.library\n");
	return (0);
    }

    CHECK_DOUBLE1A(IEEEDPSqrt, ((double)IEEEDPFlt(4)), DEF_DPTwo);
    CHECK_DOUBLE1A(IEEEDPSqrt, ((double)IEEEDPFlt(9)), DEF_DPThree);
    CHECK_DOUBLE1A(IEEEDPCos,  ((double)IEEEDPFlt(1)), 0x3fe14a280fb5068bULL);
    CHECK_DOUBLE1A(IEEEDPSin,  ((double)IEEEDPFlt(1)), 0x3feaed548f090ceeULL);
    
    CloseLibrary(MathIeeeDoubTransBase);
    CloseLibrary(MathIeeeDoubBasBase);

    return (0);
}
