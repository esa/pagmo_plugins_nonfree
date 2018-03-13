#ifndef WORHP_MACROS_CORE_F
#define WORHP_MACROS_CORE_F

! check for NaN
#define ISNAN(X) (X/=X)

! macros for checking bound types
#define X_EQ(i)        (ABS(XU(i)-XL(i)) <= work%eps)
#define X_IEQ(i)       (.NOT. X_EQ(i) .AND. (XL(i) > -par%Infty .OR. XU(i) < par%Infty)
#define X_IEQ_LOW(i)   (.NOT. X_EQ(i) .AND. XL(i) > -par%Infty .AND. XU(i) >= par%Infty)
#define X_IEQ_UP(i)    (.NOT. X_EQ(i) .AND. XL(i) <= -par%Infty .AND. XU(i) < par%Infty)
#define X_IEQ_LOWUP(i) (.NOT. X_EQ(i) .AND. XL(i) > -par%Infty .AND. XU(i) < par%Infty)
#define G_EQ(i)        (ABS(GU(i)-GL(i)) <= work%eps)
#define G_IEQ(i)       (.NOT. G_EQ(i) .AND. (GL(i) > -par%Infty .OR. GU(i) < par%Infty))
#define G_IEQ_LOW(i)   (.NOT. G_EQ(i) .AND. GL(i) > -par%Infty .AND. GU(i) >= par%Infty)
#define G_IEQ_UP(i)    (.NOT. G_EQ(i) .AND. GL(i) <= -par%Infty .AND. GU(i) < par%Infty)
#define G_IEQ_LOWUP(i) (.NOT. G_EQ(i) .AND. GL(i) > -par%Infty .AND. GU(i) < par%Infty)
#define BOUND_EQ(BL,BU)        (ABS(BU-BL) <= work%eps)
#define BOUND_IEQ(BL,BU)       (.NOT. BOUND_EQ(BL,BU) .AND. (BL > -par%Infty .OR. BU < par%Infty))
#define BOUND_IEQ_LOW(BL,BU)   (.NOT. BOUND_EQ(BL,BU) .AND. BL > -par%Infty .AND. BU >= par%Infty)
#define BOUND_IEQ_UP(BL,BU)    (.NOT. BOUND_EQ(BL,BU) .AND. BL <= -par%Infty .AND. BU < par%Infty)
#define BOUND_IEQ_LOWUP(BL,BU) (.NOT. BOUND_EQ(BL,BU) .AND. BL > -par%Infty .AND. BU < par%Infty)

! settings for WORHP NLP
#define WORHP_NLP_CREATEMATRIX

! macros for indexing X (only relevant if worhp_nlp is used)
#define IDX_X_UNBND work%NLP_idxXUnbnd0:work%NLP_idxXUnbnd1
#define IDX_X_IEQ work%NLP_idxXIeqLow0:work%NLP_idxXIeqUp1
#define IDX_X_LOW work%NLP_idxXIeqLow0:work%NLP_idxXIeqLowUp1
#define IDX_X_UP work%NLP_idxXIeqLowUp0:work%NLP_idxXIeqUp1
#define IDX_X_JUSTLOW work%NLP_idxXIeqLow0:work%NLP_idxXIeqLow1
#define IDX_X_JUSTUP work%NLP_idxXIeqUp0:work%NLP_idxXIeqUp1
#define IDX_X_JUSTLOWUP work%NLP_idxXIeqLowUp0:work%NLP_idxXIeqLowUp1

! macros for indexing bounds of X (only relevant if worhp_nlp is used)
#define IDX_XL_JUSTLOW (work%NLP_idxXIeqLow0-work%NLP_idxXUnbnd1):(work%NLP_idxXIeqLow1-work%NLP_idxXUnbnd1)
#define IDX_XL_JUSTLOWUP (work%NLP_idxXIeqLowUp0-work%NLP_idxXUnbnd1):(work%NLP_idxXIeqLowUp1-work%NLP_idxXUnbnd1)
#define IDX_XU_JUSTUP (work%NLP_idxXIeqUp0-work%NLP_idxXIeqLow1):(work%NLP_idxXIeqUp1-work%NLP_idxXIeqLow1)
#define IDX_XU_JUSTLOWUP (work%NLP_idxXIeqLowUp0-work%NLP_idxXIeqLow1):(work%NLP_idxXIeqLowUp1-work%NLP_idxXIeqLow1)

! macros for indexing G (only relevant if worhp_nlp is used)
#define IDX_G_EQ work%NLP_idxGeq0:work%NLP_idxGeq1
#define IDX_G_IEQ work%NLP_idxGieqLow0:work%NLP_idxGieqUp1
#define IDX_G_S work%NLP_idxGieqLow0:work%NLP_idxGieqUp1
#define IDX_G_LOW work%NLP_idxGieqLow0:work%NLP_idxGieqLowUp1
#define IDX_G_UP work%NLP_idxGieqLowUp0:work%NLP_idxGieqUp1
#define IDX_G_JUSTLOW work%NLP_idxGieqLow0:work%NLP_idxGieqLow1
#define IDX_G_JUSTUP work%NLP_idxGieqUp0:work%NLP_idxGieqUp1
#define IDX_G_JUSTLOWUP work%NLP_idxGieqLowUp0:work%NLP_idxGieqLowUp1
#define IDX_G_EQ_(off) (off+work%NLP_idxGeq0):(off+work%NLP_idxGeq1)
#define IDX_G_IEQ_(off) (off+work%NLP_idxGieqLow0):(off+work%NLP_idxGieqUp1)
#define IDX_G_S_(off) (off+work%NLP_idxGieqLow0):(off+work%NLP_idxGieqUp1)
#define IDX_G_LOW_(off) (off+work%NLP_idxGieqLow0):(off+work%NLP_idxGieqLowUp1)
#define IDX_G_UP_(off) (off+work%NLP_idxGieqLowUp0):(off+work%NLP_idxGieqUp1)
#define IDX_G_JUSTLOW_(off) (off+work%NLP_idxGieqLow0):(off+work%NLP_idxGieqLow1)
#define IDX_G_JUSTUP_(off) (off+work%NLP_idxGieqUp0):(off+work%NLP_idxGieqUp1)
#define IDX_G_JUSTLOWUP_(off) (off+work%NLP_idxGieqLowUp0):(off+work%NLP_idxGieqLowUp1)

! macros for indexing S (only relevant if worhp_nlp is used)
#define IDX_S_LOW (work%NLP_idxGieqLow0-work%NLP_idxGeq1):(work%NLP_idxGieqLowUp1-work%NLP_idxGeq1)
#define IDX_S_UP (work%NLP_idxGieqLowUp0-work%NLP_idxGeq1):(work%NLP_idxGieqUp1-work%NLP_idxGeq1)
#define IDX_S_JUSTLOW (work%NLP_idxGieqLow0-work%NLP_idxGeq1):(work%NLP_idxGieqLow1-work%NLP_idxGeq1)
#define IDX_S_JUSTUP (work%NLP_idxGieqUp0-work%NLP_idxGeq1):(work%NLP_idxGieqUp1-work%NLP_idxGeq1)
#define IDX_S_JUSTLOWUP (work%NLP_idxGieqLowUp0-work%NLP_idxGeq1):(work%NLP_idxGieqLowUp1-work%NLP_idxGeq1)
#define IDX_S_LOW_(off) (off+work%NLP_idxGieqLow0-work%NLP_idxGeq1):(off+work%NLP_idxGieqLowUp1-work%NLP_idxGeq1)
#define IDX_S_UP_(off) (off+work%NLP_idxGieqLowUp0-work%NLP_idxGeq1):(off+work%NLP_idxGieqUp1-work%NLP_idxGeq1)
#define IDX_S_JUSTLOW_(off) (off+work%NLP_idxGieqLow0-work%NLP_idxGeq1):(off+work%NLP_idxGieqLow1-work%NLP_idxGeq1)
#define IDX_S_JUSTUP_(off) (off+work%NLP_idxGieqUp0-work%NLP_idxGeq1):(off+work%NLP_idxGieqUp1-work%NLP_idxGeq1)
#define IDX_S_JUSTLOWUP_(off) (off+work%NLP_idxGieqLowUp0-work%NLP_idxGeq1):(off+work%NLP_idxGieqLowUp1-work%NLP_idxGeq1)

! macros for indexing bounds of G (only relevant if worhp_nlp is used)
#define IDX_GL_JUSTLOW (work%NLP_idxGieqLow0-work%NLP_idxGeq1):(work%NLP_idxGieqLow1-work%NLP_idxGeq1)
#define IDX_GL_JUSTLOWUP (work%NLP_idxGieqLowUp0-work%NLP_idxGeq1):(work%NLP_idxGieqLowUp1-work%NLP_idxGeq1)
#define IDX_GU_JUSTUP (work%NLP_idxGieqUp0-work%NLP_idxGieqLow1):(work%NLP_idxGieqUp1-work%NLP_idxGieqLow1)
#define IDX_GU_JUSTLOWUP (work%NLP_idxGieqLowUp0-work%NLP_idxGieqLow1):(work%NLP_idxGieqLowUp1-work%NLP_idxGieqLow1)

! macros for indexing bounds of S (only relevant if worhp_nlp is used)
#define IDX_SL_JUSTLOW (work%NLP_idxGieqLow0-work%NLP_idxGeq1):(work%NLP_idxGieqLow1-work%NLP_idxGeq1)
#define IDX_SL_JUSTLOWUP (work%NLP_idxGieqLowUp0-work%NLP_idxGeq1):(work%NLP_idxGieqLowUp1-work%NLP_idxGeq1)
#define IDX_SU_JUSTUP (work%NLP_idxGieqUp0-work%NLP_idxGieqLow1):(work%NLP_idxGieqUp1-work%NLP_idxGieqLow1)
#define IDX_SU_JUSTLOWUP (work%NLP_idxGieqLowUp0-work%NLP_idxGieqLow1):(work%NLP_idxGieqLowUp1-work%NLP_idxGieqLow1)

! macros for loops over X (only relevant if worhp_nlp is used)
! k: index for original X
! l: index for modified X
! i: index for modified inequality X
! j: index for modified lower/upper inequality X
#define LOOP_X_ORIG(k,l) l = 1, work%NLP_n ;\
    k = idxOrigX(l)
#define LOOP_X_ORIG_LOW(k,l,i,jl) l = work%NLP_idxXIeqLow0, work%NLP_idxXIeqLowUp1 ;\
    k = idxOrigX(l) ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    jl = l - work%NLP_idxXIeqLow0 + 1
#define LOOP_X_ORIG_UP(k,l,i,ju) l = work%NLP_idxXIeqLowUp0, work%NLP_idxXIeqUp1 ;\
    k = idxOrigX(l) ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    ju = l - work%NLP_idxXIeqLowUp0 + 1
#define LOOP_X_ORIG_JUSTLOW(k,l,i,jl) l = work%NLP_idxXIeqLow0, work%NLP_idxXIeqLow1 ;\
    k = idxOrigX(l) ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    jl = l - work%NLP_idxXIeqLow0 + 1
#define LOOP_X_ORIG_JUSTUP(k,l,i,ju) l = work%NLP_idxXIeqUp0, work%NLP_idxXIeqUp1 ;\
    k = idxOrigX(l) ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    ju = l - work%NLP_idxXIeqLowUp0 + 1
#define LOOP_X_ORIG_JUSTLOWUP(k,l,i,jl,ju) l = work%NLP_idxXIeqLowUp0, work%NLP_idxXIeqLowUp1 ;\
    k = idxOrigX(l) ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    jl = l - work%NLP_idxXIeqLow0 + 1 ;\
    ju = l - work%NLP_idxXIeqLowUp0 + 1
#define LOOP_X(l) l = 1, work%NLP_n
#define LOOP_X_LOW(l,i,jl) l = work%NLP_idxXIeqLow0, work%NLP_idxXIeqLowUp1 ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    jl = l - work%NLP_idxXIeqLow0 + 1
#define LOOP_X_UP(l,i,ju) l = work%NLP_idxXIeqLowUp0, work%NLP_idxXIeqUp1 ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    ju = l - work%NLP_idxXIeqLowUp0 + 1
#define LOOP_X_JUSTLOW(l,i,jl) l = work%NLP_idxXIeqLow0, work%NLP_idxXIeqLow1 ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    jl = l - work%NLP_idxXIeqLow0 + 1
#define LOOP_X_JUSTUP(l,i,ju) l = work%NLP_idxXIeqUp0, work%NLP_idxXIeqUp1 ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    ju = l - work%NLP_idxXIeqLowUp0 + 1
#define LOOP_X_JUSTLOWUP(l,i,jl,ju) l = work%NLP_idxXIeqLowUp0, work%NLP_idxXIeqLowUp1 ;\
    i = l - work%NLP_idxXUnbnd1 ;\
    jl = l - work%NLP_idxXIeqLow0 + 1 ;\
    ju = l - work%NLP_idxXIeqLowUp0 + 1

! macros for loops over G (only relevant if worhp_nlp is used)
! k: index for original G
! l: index for modified G
! i: index for modified inequality G
! j: index for modified lower/upper inequality G
#define LOOP_G_ORIG(k,l) l = 1, work%NLP_m ;\
    k = idxOrigG(l)
#define LOOP_G_ORIG_EQ(k,l,i) l = work%NLP_idxGeq0, work%NLP_idxGeq1 ;\
    k = idxOrigG(l) ;\
    i = l
#define LOOP_G_ORIG_IEQ(k,l,i) l = work%NLP_idxGieqLow0, work%NLP_idxGieqUp1 ;\
    k = idxOrigG(l) ;\
    i = l - work%NLP_idxGeq1
#define LOOP_G_ORIG_LOW(k,l,i,jl) l = work%NLP_idxGieqLow0, work%NLP_idxGieqLowUp1 ;\
    k = idxOrigG(l) ;\
    i = l - work%NLP_idxGeq1 ;\
    jl = l - work%NLP_idxGieqLow0 + 1
#define LOOP_G_ORIG_UP(k,l,i,ju) l = work%NLP_idxGieqLowUp0, work%NLP_idxGieqUp1 ;\
    k = idxOrigG(l) ;\
    i = l - work%NLP_idxGeq1 ;\
    ju = l - work%NLP_idxGieqLowUp0 + 1
#define LOOP_G_ORIG_JUSTLOW(k,l,i,jl) l = work%NLP_idxGieqLow0, work%NLP_idxGieqLow1 ;\
    k = idxOrigG(l) ;\
    i = l - work%NLP_idxGeq1 ;\
    jl = l - work%NLP_idxGieqLow0 + 1
#define LOOP_G_ORIG_JUSTUP(k,l,i,ju) l = work%NLP_idxGieqUp0, work%NLP_idxGieqUp1 ;\
    k = idxOrigG(l) ;\
    i = l - work%NLP_idxGeq1 ;\
    ju = l - work%NLP_idxGieqLowUp0 + 1
#define LOOP_G_ORIG_JUSTLOWUP(k,l,i,jl,ju) l = work%NLP_idxGieqLowUp0, work%NLP_idxGieqLowUp1 ;\
    k = idxOrigG(l) ;\
    i = l - work%NLP_idxGeq1 ;\
    jl = l - work%NLP_idxGieqLow0 + 1 ;\
    ju = l - work%NLP_idxGieqLowUp0 + 1
#define LOOP_G(l) l = 1, work%NLP_m
#define LOOP_G_EQ(l,i) l = work%NLP_idxGeq0, work%NLP_idxGeq1 ;\
    i = l
#define LOOP_G_IEQ(l,i) l = work%NLP_idxGieqLow0, work%NLP_idxGieqUp1 ;\
    i = l - work%NLP_idxGeq1
#define LOOP_G_LOW(l,i,jl) l = work%NLP_idxGieqLow0, work%NLP_idxGieqLowUp1 ;\
    i = l - work%NLP_idxGeq1 ;\
    jl = l - work%NLP_idxGieqLow0 + 1
#define LOOP_G_UP(l,i,ju) l = work%NLP_idxGieqLowUp0, work%NLP_idxGieqUp1 ;\
    i = l - work%NLP_idxGeq1 ;\
    ju = l - work%NLP_idxGieqLowUp0 + 1
#define LOOP_G_JUSTLOW(l,i,jl) l = work%NLP_idxGieqLow0, work%NLP_idxGieqLow1 ;\
    i = l - work%NLP_idxGeq1 ;\
    jl = l - work%NLP_idxGieqLow0 + 1
#define LOOP_G_JUSTUP(l,i,ju) l = work%NLP_idxGieqUp0, work%NLP_idxGieqUp1 ;\
    i = l - work%NLP_idxGeq1 ;\
    ju = l - work%NLP_idxGieqLowUp0 + 1
#define LOOP_G_JUSTLOWUP(l,i,jl,ju) l = work%NLP_idxGieqLowUp0, work%NLP_idxGieqLowUp1 ;\
    i = l - work%NLP_idxGeq1 ;\
    jl = l - work%NLP_idxGieqLow0 + 1 ;\
    ju = l - work%NLP_idxGieqLowUp0 + 1

! macros for loops over S (only relevant if worhp_nlp is used)
! k: index for original G
! l: index for modified G
! i: index for modified S
! j: index for modified lower/upper inequality S
#define LOOP_S_ORIG(k,l,i) LOOP_G_ORIG_IEQ(k,l,i)
#define LOOP_S_ORIG_LOW(k,l,i,jl) LOOP_G_ORIG_LOW(k,l,i,jl)
#define LOOP_S_ORIG_UP(k,l,i,ju) LOOP_G_ORIG_UP(k,l,i,ju)
#define LOOP_S_ORIG_JUSTLOW(k,l,i,jl) LOOP_G_ORIG_JUSTLOW(k,l,i,jl)
#define LOOP_S_ORIG_JUSTUP(k,l,i,ju) LOOP_G_ORIG_JUSTUP(k,l,i,ju)
#define LOOP_S_ORIG_JUSTLOWUP(k,l,i,jl,ju) LOOP_G_ORIG_JUSTLOWUP(k,l,i,jl,ju)
#define LOOP_S(l,i) LOOP_G_IEQ(l,i)
#define LOOP_S_LOW(l,i,jl) LOOP_G_LOW(l,i,jl)
#define LOOP_S_UP(l,i,ju) LOOP_G_UP(l,i,ju)
#define LOOP_S_JUSTLOW(l,i,jl) LOOP_G_JUSTLOW(l,i,jl)
#define LOOP_S_JUSTUP(l,i,ju) LOOP_G_JUSTUP(l,i,ju)
#define LOOP_S_JUSTLOWUP(l,i,jl,ju) LOOP_G_JUSTLOWUP(l,i,jl,ju)

! macros for loops over DF (only relevant if worhp_nlp is used)
#ifdef WORHP_NLP_CREATEMATRIX
#define LOOP_DF(k,i,row,val) i = 1, work%NLP_DF%nnz ;\
    k = idxOrigDF(i) ;\
    row = DFrow(i) ;\
    val = DFval(i)
#define LOOP_DF_IDX(k,i,row) i = 1, work%NLP_DF%nnz ;\
    k = idxOrigDF(i) ;\
    row = DFrow(i)
#define LOOP_DF_VAL(k,i,val) i = 1, work%NLP_DF%nnz ;\
    k = idxOrigDF(i) ;\
    val = DFval(i)
#define LOOP_DG(k,i,row,col,val) i = 1, work%NLP_DG%nnz ;\
    k = idxOrigDG(i) ;\
    row = DGrow(i) ;\
    col = DGcol(i) ;\
    val = DGval(i)
#define LOOP_DG_IDX(k,i,row,col) i = 1, work%NLP_DG%nnz ;\
    k = idxOrigDG(i) ;\
    row = DGrow(i) ;\
    col = DGcol(i)
#define LOOP_DG_VAL(k,i,val) i = 1, work%NLP_DG%nnz ;\
    k = idxOrigDG(i) ;\
    val = DGval(i)
#define LOOP_HM(k,i,row,col,val) i = 1, work%NLP_HM%nnz ;\
    k = idxOrigHM(i) ;\
    row = HMrow(i) ;\
    col = HMcol(i) ;\
    val = HMval(i)
#define LOOP_HM_IDX(k,i,row,col) i = 1, work%NLP_HM%nnz ;\
    k = idxOrigHM(i) ;\
    row = HMrow(i) ;\
    col = HMcol(i)
#define LOOP_HM_VAL(k,i,val) i = 1, work%NLP_HM%nnz ;\
    k = idxOrigHM(i) ;\
    val = HMval(i)
#else
#define LOOP_DF(k,i,row,val) i = 1, work%NLP_DF%nnz ;\
    k = idxOrigDF(i) ;\
    row = idxX(DFrow(k)) ;\
    val = DFval(k) * work%NLP_ScaleF * work%NLP_ScaleFalg
#define LOOP_DF_IDX(k,i,row) i = 1, work%NLP_DF%nnz ;\
    k = idxOrigDF(i) ;\
    row = idxX(DFrow(k))
#define LOOP_DF_VAL(k,i,val) i = 1, work%NLP_DF%nnz ;\
    k = idxOrigDF(i) ;\
    val = DFval(k) * work%NLP_ScaleF * work%NLP_ScaleFalg
#define LOOP_DG(k,i,row,col,val) i = 1, work%NLP_DG%nnz ;\
    k = idxOrigDG(i) ;\
    row = idxG(DGrow(k)) ;\
    col = idxX(DGcol(k)) ;\
    val = DGval(k) * ScaleG(row)
#define LOOP_DG_IDX(k,i,row,col) i = 1, work%NLP_DG%nnz ;\
    k = idxOrigDG(i) ;\
    row = idxG(DGrow(k)) ;\
    col = idxX(DGcol(k))
#define LOOP_DG_VAL(k,i,val) i = 1, work%NLP_DG%nnz ;\
    k = idxOrigDG(i) ;\
    val = DGval(k) * ScaleG(idxG(DGrow(k)))
#define LOOP_HM(k,i,row,col,val) i = 1, work%NLP_HM%nnz ;\
    k = idxOrigHM(i) ;\
    row = idxX(HMrow(k)) ;\
    col = idxX(HMcol(k)) ;\
    val = HMval(k)
#define LOOP_HM_IDX(k,i,row,col) i = 1, work%NLP_HM%nnz ;\
    k = idxOrigHM(i) ;\
    row = idxX(HMrow(k)) ;\
    col = idxX(HMcol(k))
#define LOOP_HM_VAL(k,i,val) i = 1, work%NLP_HM%nnz ;\
    k = idxOrigHM(i) ;\
    val = HMval(k)
#endif

! the following macros belong to the index sets but it is highly recommended
! to use the worhp_nlp module and its macros instead. The following old
! macros are marked with a 0 in the name.

! macros for loops over equality or (lower / upper) inequality constraints
! Example for indices k, i, l and j. Usage is always the same. Index l just
! appears for X, not for G.
! k: index in {1,...,n}
! l: index in {1,...,n-nXeq}
! i: index in {1,...,nXieq}
! j: index in {1,...,nXieqLow}
#define LOOP_X_EQ0(k,i) i = 1, work%nXeq ;\
    k = idxXeq(i)
#define LOOP_X_NEQ0(k,i) i = 1, work%nXneq ;\
    k = idxXneq(i)
#define LOOP_X_IEQ0(k,l,i) i = 1, work%nXieq ;\
    l = idxXieq(i) ;\
    k = idxXneq(l)
#define LOOP_X_IEQ_LOW0(k,l,i,j) j = 1, work%nXieqLowUp + work%nXieqLow ;\
    i = idxXieqLow(j) ;\
    l = idxXieq(i) ;\
    k = idxXneq(l)
#define LOOP_X_IEQ_UP0(k,l,i,j) j = 1, work%nXieqLowUp + work%nXieqUp ;\
    i = idxXieqUp(j) ;\
    l = idxXieq(i) ;\
    k = idxXneq(l)
#define LOOP_X_IEQ_JUST_LOW0(k,l,i,j) j = work%nXieqLowUp + 1, work%nXieqLowUp + work%nXieqLow ;\
    i = idxXieqLow(j) ;\
    l = idxXieq(i) ;\
    k = idxXneq(l)
#define LOOP_X_IEQ_JUST_UP0(k,l,i,j) j = work%nXieqLowUp + 1, work%nXieqLowUp + work%nXieqUp ;\
    i = idxXieqUp(j) ;\
    l = idxXieq(i) ;\
    k = idxXneq(l)
#define LOOP_X_IEQ_JUST_LOWUP0(k,l,i,j) j = 1, work%nXieqLowUp ;\
    i = idxXieqLow(j) ;\
    l = idxXieq(i) ;\
    k = idxXneq(l)
#define LOOP_G_EQ0(k,i) i = 1, work%nGeq ;\
    k = idxGeq(i)
#define LOOP_G_IEQ0(k,i) i = 1, work%nGieq ;\
    k = idxGieq(i)
#define LOOP_G_IEQ_LOW0(k,i,j) j = 1, work%nGieqLowUp + work%nGieqLow ;\
    i = idxGieqLow(j) ;\
    k = idxGieq(i)
#define LOOP_G_IEQ_UP0(k,i,j) j = 1, work%nGieqLowUp + work%nGieqUp ;\
    i = idxGieqUp(j) ;\
    k = idxGieq(i)
#define LOOP_G_IEQ_JUST_LOW0(k,i,j) j = work%nGieqLowUp + 1, work%nGieqLowUp + work%nGieqLow ;\
    i = idxGieqLow(j) ;\
    k = idxGieq(i)
#define LOOP_G_IEQ_JUST_UP0(k,i,j) j = work%nGieqLowUp + 1, work%nGieqLowUp + work%nGieqUp ;\
    i = idxGieqUp(j) ;\
    k = idxGieq(i)
#define LOOP_G_IEQ_JUST_LOWUP0(k,i,j) j = 1, work%nGieqLowUp ;\
    i = idxGieqLow(j) ;\
    k = idxGieq(i)


! macros for loops over nonzeros of HM/DG without Xeq
#define LOOP_DF_XNEQ(k,i) i = 1, work%nnzDFXneq ;\
    k = idxDFXneq(i)
#define LOOP_DF_XNEQ_CS(k,i,row) i = 1, work%nnzDFXneq ;\
    k = idxDFXneq(i) ;\
    row = idxXneqInv(DFrow(k))
#define LOOP_HM_XNEQ(k,i) i = 1, work%nnzHMXneq ;\
    k = idxHMXneq(i)
#define LOOP_HM_XNEQ_CS(k,i,row,col) i = 1, work%nnzHMXneq ;\
    k = idxHMXneq(i) ;\
    row = idxXneqInv(HMrow(k)) ;\
    col = idxXneqInv(HMcol(k))
#define LOOP_DG_XNEQ(k,i) i = 1, work%nnzDGXneq ;\
    k = idxDGXneq(i)
#define LOOP_DG_XNEQ_CS(k,i,row,col) i = 1, work%nnzDGXneq ;\
    k = idxDGXneq(i) ;\
    row = DGrow(k) ;\
    col = idxXneqInv(DGcol(k))
#define LOOP_HM_DIAG_XNEQ(k,i) i = 1, work%nXneq ;\
    k = idxHMdiagXneq(i)

! macros for loops ober nonzeros of DG taking constraint type into account
#define LOOP_DG_GEQ(k,i) i = 1, work%nnzDGGeq ;\
    k = idxDGGeq(i)

! WARNING: This macro differs from the above for accessing without
!          eliminated X (because of equality box constraints)
!          at the moment we do not want to eliminate box constraints
!          because we have to duplicate the hessian matrix, which might
!          be very unfortunate in high dimensions
!          @sge
#define LOOP_DG_GEQ_CS(k,i,row,col) i = 1, work%nnzDGGeq ;\
    k = idxDGGeq(i) ;\
    row = DGrow(k) ;\
    col = DGcol(k)
#define LOOP_DG_GIEQ(k,i) i = 1, work%nnzDGGieq ;\
    k = idxDGGieq(i)
! WARNING: Warning from above applies here as well
#define LOOP_DG_GIEQ_CS(k,i,row,col) i = 1, work%nnzDGGieq ;\
    k = idxDGGieq(i) ;\
    row = DGrow(k) ;\
    col = DGcol(k)
#define LOOP_DG_GIEQLOW(k,i) i = 1, work%nnzDGGieqLow ;\
    k = idxDGGieqLow(i)
! WARNING: Warning from above applies here as well
#define LOOP_DG_GIEQLOW_CS(k,i,row,col) i = 1, work%nnzDGGieqLow ;\
    k = idxDGGieqLow(i) ;\
    row = DGrow(k) ;\
    col = DGcol(k)
#define LOOP_DG_GIEQUP(k,i) i = 1, work%nnzDGGieqUp ;\
    k = idxDGGieqUp(i)
! WARNING: Warning from above applies here as well
#define LOOP_DG_GIEQUP_CS(k,i,row,col) i = 1, work%nnzDGGieqUp ;\
    k = idxDGGieqUp(i) ;\
    row = DGrow(k) ;\
    col = DGcol(k)
#define LOOP_DG_GIEQLOWUP(k,i) i = 1, work%nnzDGGieqLowUp ;\
    k = idxDGGieqLowUp(i)
! WARNING: Warning from above applies here as well
#define LOOP_DG_GIEQLOWUP_CS(k,i,row,col) i = 1, work%nnzDGGieqLowUp ;\
    k = idxDGGieqLowUp(i) ;\
    row = DGrow(k) ;\
    col = DGcol(k)

#endif
