#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "worhp_bogus.h"

inline double closed_interval_rand(double x0, double x1)
{
    return x0 + (x1 - x0) * rand() / ((double)RAND_MAX);
}

void ReadParams(int *a, const char b[], Params *c) {}
void WorhpPreInit(OptVar *o, Workspace *w, Params *p, Control *c) {}
void WorhpInit(OptVar *o, Workspace *w, Params *p, Control *c)
{
    o->X = calloc(o->n, sizeof(double));
    o->G = calloc(o->m, sizeof(double));
    o->Lambda = calloc(o->n, sizeof(double));
    o->XL = calloc(o->n, sizeof(double));
    o->XU = calloc(o->n, sizeof(double));
    o->Mu = calloc(o->m, sizeof(double));
    o->GL = calloc(o->m, sizeof(double));
    o->GU = calloc(o->m, sizeof(double));
    w->DF.row = calloc(w->DF.nnz, sizeof(int));
    w->DF.val = calloc(w->DF.nnz, sizeof(double));
    w->DG.row = calloc(w->DG.nnz, sizeof(int));
    w->DG.col = calloc(w->DG.nnz, sizeof(int));
    w->DG.val = calloc(w->DG.nnz, sizeof(double));
    w->HM.row = calloc(w->HM.nnz, sizeof(int));
    w->HM.col = calloc(w->HM.nnz, sizeof(int));
    w->HM.val = calloc(w->HM.nnz, sizeof(double));
    w->DF.NeedStructure = false;
    w->DG.NeedStructure = false;
    w->HM.NeedStructure = false;
    w->ScaleObj = 1;
    c->status = 0; // to ensure it will enter the main loop in worhp.hpp
    srand((unsigned int)(time(NULL)));
}
bool GetUserAction(const Control *c, int b)
{
    return true;
}
void DoneUserAction(Control *a, int b) {}
void IterationOutput(OptVar *o, Workspace *w, Params *p, Control *c) {}
void Worhp(OptVar *o, Workspace *w, Params *p, Control *c)
{
    c->status = c->status + 100; // this will make it so after ten calls it concludes.
    if (c->status >= TerminateSuccess) {
        c->ZenInit = true;
    }
    // Random vector
    int j;
    for (j = 0; j < o->n; ++j) {
        o->X[j] = closed_interval_rand(o->XL[j], o->XU[j]);
    }
}

void WorhpDiag(OptVar *o, Workspace *w, Params *p, Control *c)
{
    printf("Many Diagnostics!!!! So helpful!!\n");
}

void StatusMsg(OptVar *o, Workspace *w, Params *p, Control *c)
{
    printf("All went great!!!! What a glamorous Success!!\n");
}

void StatusMsgString(OptVar *o, Workspace *w, Params *p, Control *c, char message[])
{
#ifdef _MSC_VER //Avoiding a warning in MSVC builds
    strcpy_s(message, sizeof(char) * 47, "All went great!!!! What a glamorous Success!!\n");
#else
    strcpy(message, "All went great!!!! What a glamorous Success!!\n");
#endif
}

void WorhpFree(OptVar *o, Workspace *w, Params *p, Control *c)
{
    free(o->X);
    free(o->G);
    free(o->Lambda);
    free(o->XL);
    free(o->XU);
    free(o->Mu);
    free(o->GL);
    free(o->GU);
    free(w->DF.row);
    free(w->DF.val);
    free(w->DG.row);
    free(w->DG.col);
    free(w->DG.val);
    free(w->HM.row);
    free(w->HM.col);
    free(w->HM.val);
}
void WorhpFidif(OptVar *o, Workspace *w, Params *p, Control *c) {}
bool WorhpSetBoolParam(Params *p, const char *stropt, bool b)
{
    char *invalid;
    invalid = "invalid_bool_option";
    if (strcmp(stropt, invalid) == 0) {
        return 0;
    } else {
        return 1;
    }
}
bool WorhpSetIntParam(Params *p, const char *stropt, int b)
{
    char *invalid;
    invalid = "invalid_integer_option";
    if (strcmp(stropt, invalid) == 0) {
        return 0;
    } else {
        return 1;
    }
}
bool WorhpSetDoubleParam(Params *p, const char *stropt, double b)
{
    char *invalid;
    invalid = "invalid_numeric_option";
    if (strcmp(stropt, invalid) == 0) {
        return 0;
    } else {
        return 1;
    }
}
void SetWorhpPrint(worhp_print_t l1) {}

void WorhpVersion(int *major, int *minor, char patch[PATCH_STRING_LENGTH])
{
#ifdef _MSC_VER //Avoiding a warning in MSVC builds
    strcpy_s(patch, sizeof(char) * PATCH_STRING_LENGTH, "1");
#else
    strcpy(patch, "1");
#endif
    *major = 1;
    *minor = 14;
};

void ZenUpdate(OptVar *o, Workspace *w, Params *p, Control *c,
    const char *var_pert, double *varnew, const double *dp,
    const double *dr, const double *dq,
    const double *db, const int *order)
{
    for (int j = 0; j < o->n; ++j) {
        varnew[j] = closed_interval_rand(o->XL[j], o->XU[j]);
    }
}

void ZenGetMaxPert(OptVar *o, Workspace *w, Params *p, Control *c,
        double *maxDP, double *maxDR, double *maxDQ, double *maxDB)
{
    for (int j = 0; j < o->k; ++j) {
        maxDP[j] = 0.1;
    }

    for (int j = 0; j < o->n; ++j) {
        maxDR[j] = 0.1;
        maxDB[j] = 0.1;
    }

    for (int j = 0; j < o->m; ++j) {
        maxDQ[j] = 0.1;
    }
}
