#ifndef WORHP_MACROS_WMT_F
#define WORHP_MACROS_WMT_F

! 
! Macros for secure access to workspace partitions
! via Workspace Management Tables
! 

! Get whole slices. Preferred way of access 
#define IWS_SLICE(ENTRY) iws(work%IWMT(ENTRY,3):work%IWMT(ENTRY,4))
#define RWS_SLICE(ENTRY) rws(work%RWMT(ENTRY,3):work%RWMT(ENTRY,4))

! Get size of the specified slice 
#define IWMT_SIZE(ENTRY) INT(work%IWMT(ENTRY,5), KIND=WORHP_INT)
#define RWMT_SIZE(ENTRY) INT(work%RWMT(ENTRY,5), KIND=WORHP_INT)


!
! Macros for custom access with 1-indexing: 1...N
!

! Get index in WS 
#define IWS_IDX_1(ENTRY) work%IWMT(ENTRY,1)
#define RWS_IDX_1(ENTRY) work%RWMT(ENTRY,1)

! Get single element in WS 
#define IWS_ELEM_1(ENTRY,IDX) iws(work%IWMT(ENTRY,1)+IDX)
#define RWS_ELEM_1(ENTRY,IDX) rws(work%RWMT(ENTRY,1)+IDX)

! Get custom range of WS 
#define IWS_RANGE_1(E,IS,IE) iws(work%IWMT(E,1)+IS:work%IWMT(E,1)+IE)
#define RWS_RANGE_1(E,IS,IE) rws(work%RWMT(E,1)+IS:work%RWMT(E,1)+IE)


!
! Macros for custom access with 0-indexing: 0...N-1
! 

! Get index in WS 
#define IWS_IDX_0(ENTRY) work%IWMT(ENTRY,3)
#define RWS_IDX_0(ENTRY) work%RWMT(ENTRY,3)

! Get single element in WS 
#define IWS_ELEM_0(ENTRY,IDX) iws(work%IWMT(ENTRY,3)+IDX)
#define RWS_ELEM_0(ENTRY,IDX) rws(work%RWMT(ENTRY,3)+IDX)

! Get custom range of WS 
#define IWS_RANGE_0(E,IS,IE) iws(work%IWMT(E,3)+IS:work%IWMT(E,3)+IE)
#define RWS_RANGE_0(E,IS,IE) rws(work%RWMT(E,3)+IS:work%RWMT(E,3)+IE)


! Macros for pointer declaration.
 
#define INT_POINTER  INTEGER(WORHP_INT),  POINTER, DIMENSION(:)
#define REAL_POINTER REAL(WORHP_DOUBLE),  POINTER, DIMENSION(:)
#define BOOL_POINTER LOGICAL(WORHP_BOOL), POINTER, DIMENSION(:)



!**********************************************
!                                              
!                 NEW MACROS                   
! These take more arguments, so the workspace  
! instance and iws/rws names can be chosen     
! freely (the old macros assume them to be     
! "work" and "Xws")                            
!                                              
!**********************************************

! Get whole slices. Preferred way of access 
#define I_S(wsp,idx,iws) iws(wsp%IWMT(idx,3):wsp%IWMT(idx,4))
#define R_S(wsp,idx,rws) rws(wsp%RWMT(idx,3):wsp%RWMT(idx,4))

! Get size of the specified slice 
#define I_N(wsp,idx) wsp%IWMT(idx,5)
#define R_N(wsp,idx) wsp%RWMT(idx,5)


!
! Macros for custom access with 1-indexing: 1...N
!

! Get single element in WS 
#define I_E1(wsp,idx,iws,i) iws(wsp%IWMT(idx,1)+i)
#define R_E1(wsp,idx,rws,i) rws(wsp%RWMT(idx,1)+i)

! Get custom range of WS 
#define I_R1(wsp,idx,iws,i,j) iws(wsp%IWMT(idx,1)+i:wsp%IWMT(idx,1)+j)
#define R_R1(wsp,idx,rws,i,j) rws(wsp%RWMT(idx,1)+i:wsp%RWMT(idx,1)+j)

! Get index in WS 
#define I_I(wsp,idx) wsp%IWMT(idx,1)
#define R_I(wsp,idx) wsp%RWMT(idx,1)

#endif
