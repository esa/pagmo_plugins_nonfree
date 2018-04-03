/* Copyright 2018 PaGMO development team
This file is part of "pagmo plugins nonfree", a PaGMO affiliated library.
The "pagmo plugins nonfree" library, is free software;
you can redistribute it and/or modify it under the terms of either:
  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.
or
  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.
or both in parallel, as here.

Linking "pagmo plugins nonfree" statically or dynamically with other modules is
making a combined work based on "pagmo plugins nonfree". Thus, the terms and conditions
of the GNU General Public License cover the whole combination.

As a special exception, the copyright holders of "pagmo plugins nonfree" give you
permission to combine ABC program with free software programs or libraries that are
released under the GNU LGPL and with independent modules that communicate with
"pagmo plugins nonfree" solely through the interface defined by the headers included in
"pagmo plugins nonfree" bogus_libs folder.
You may copy and distribute such a system following the terms of the licence
for "pagmo plugins nonfree" and the licenses of the other code concerned, provided that
you include the source code of that other code when and as the "pagmo plugins nonfree" licence
requires distribution of source code and provided that you do not modify the interface defined in the bogus_libs folder

Note that people who make modified versions of "pagmo plugins nonfree" are not obligated to grant this special
exception for their modified versions; it is their choice whether to do so.
The GNU General Public License gives permission to release a modified version without this exception;
this exception also makes it possible to release a modified version which carries forward this exception.
If you modify the interface defined in the bogus_libs folder, this exception does not apply to your
modified version of "pagmo plugins nonfree", and you must remove this exception when you distribute your modified
version.

This exception is an additional permission under section 7 of the GNU General Public License, version 3 (“GPLv3”)

The "pagmo plugins nonfree" library, and its affiliated librares are distributed in the hope
that they will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.
You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the "pagmo plugins nonfree" library.  If not,
see https://www.gnu.org/licenses/. */

#ifndef SNOPT_C_H
#define SNOPT_C_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// When in windows symbols must be explictly marked for export
#ifdef _WIN32
#define __PAGMO_VISIBLE __declspec(dllexport)
#else
#define __PAGMO_VISIBLE __attribute__((visibility("default")))
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
// void snInit(snProblem *prob, char *name, char *prtfile, int summOn);

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
