#ifndef SNOPT_C_H
#define SNOPT_C_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// We solve the library madness in windows
#ifdef _WIN32
	#define __PAGMO_VISIBLE __declspec(dllexport)
#else
	#define __PAGMO_VISIBLE __attribute__ ((visibility("default")))
#endif

typedef void (*isnLog)(int *iAbort, int *info, int *HQNType, int KTcond[], int *MjrPrt, int *minimz, int *n, int *nb,
                       int *nnCon0, int *nS, int *itn, int *nMajor, int *nMinor, int *nSwap, double *condHz, int *iObj,
                       double *sclObj, double *ObjAdd, double *fMrt, double *PenNrm, double *step, double *prInf,
                       double *duInf, double *vimax, double *virel, int hs[], int *ne, int nlocJ[], int locJ[],
                       int indJ[], double Jcol[], double Ascale[], double bl[], double bu[], double fCon[],
                       double yCon[], double x[], char cu[], int *lencu, int iu[], int *leniu, double ru[], int *lenru,
                       char cw[], int *lencw, int iw[], int *leniw, double rw[], int *lenrw);

typedef void (*isnLog2)(int *Prob, char *ProbTag, int *Elastc, int *gotR, int *jstFea, int *feasbl, int *m, int *mBS,
                        int *nnH, int *nS, int *jSq, int *jBr, int *jSr, int *linesP, int *linesS, int *itn, int *itQP,
                        int *kPrc, int *lvlInf, double *pivot, double *step, int *nInf, double *sInf, double *wtInf,
                        double *ObjPrt, double *condHz, double *djqPrt, double *rgNorm, int kBS[], double xBS[],
                        int iw[], int *leniw);

typedef void (*isqLog)(int *Prob, char *ProbTag, int *Elastc, int *gotR, int *jstFea, int *feasbl, int *m, int *mBS,
                       int *nnH, int *nS, int *jSq, int *jBr, int *jSr, int *linesP, int *linesS, int *itn, int *itQP,
                       int *kPrc, int *lvlInf, double *pivot, double *step, int *nInf, double *sInf, double *wtInf,
                       double *ObjPrt, double *condHz, double *djqPrt, double *rgNorm, int kBS[], double xBS[],
                       int iw[], int *leniw);

typedef void (*isnSTOP)(int *iAbort, int KTcond[], int *MjrPrt, int *minimz, int *m, int *maxS, int *n, int *nb,
                        int *nnCon0, int *nnCon, int *nnObj0, int *nnObj, int *nS, int *itn, int *nMajor, int *nMinor,
                        int *nSwap, double *condHz, int *iObj, double *sclObj, double *ObjAdd, double *fMrt,
                        double *PenNrm, double *step, double *prInf, double *duInf, double *vimax, double *virel,
                        int hs[], int *ne, int *nlocJ, int locJ[], int indJ[], double Jcol[], int *negCon,
                        double Ascale[], double bl[], double bu[], double fCon[], double gCon[], double gObj[],
                        double yCon[], double pi[], double rc[], double rg[], double x[], char cu[], int *lencu,
                        int iu[], int *leniu, double ru[], int *lenru, char cw[], int *lencw, int iw[], int *leniw,
                        double rw[], int *lenrw);

typedef void (*snFunA)(int *Status, int *n, double x[], int *needF, int *neF, double F[], int *needG, int *neG,
                       double G[], char cu[], int *lencu, int iu[], int *leniu, double ru[], int *lenru);

double closed_interval_rand(double x0, double x1);
//void snInit(snProblem *prob, char *name, char *prtfile, int summOn);
 
typedef struct {
    char *name;

    int memCalled;
    int initCalled;

    isnSTOP snSTOP;
    isnLog snLog;
    isnLog2 snLog2;
    isqLog sqLog;

    int lenrw, leniw;
    int *iw;
    double *rw;

    int lenru, leniu;
    int *iu;
    double *ru;

} snProblem;

#endif
