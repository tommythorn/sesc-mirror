#include "blaswrap.h"
#include "f2c.h"

/* Subroutine */ int cggesx_(char *jobvsl, char *jobvsr, char *sort, L_fp 
	selctg, char *sense, integer *n, complex *a, integer *lda, complex *b,
	 integer *ldb, integer *sdim, complex *alpha, complex *beta, complex *
	vsl, integer *ldvsl, complex *vsr, integer *ldvsr, real *rconde, real 
	*rcondv, complex *work, integer *lwork, real *rwork, integer *iwork, 
	integer *liwork, logical *bwork, integer *info)
{
/*  -- LAPACK driver routine (version 3.0) --   
       Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,   
       Courant Institute, Argonne National Lab, and Rice University   
       June 30, 1999   


    Purpose   
    =======   

    CGGESX computes for a pair of N-by-N complex nonsymmetric matrices   
    (A,B), the generalized eigenvalues, the complex Schur form (S,T),   
    and, optionally, the left and/or right matrices of Schur vectors (VSL   
    and VSR).  This gives the generalized Schur factorization   

         (A,B) = ( (VSL) S (VSR)**H, (VSL) T (VSR)**H )   

    where (VSR)**H is the conjugate-transpose of VSR.   

    Optionally, it also orders the eigenvalues so that a selected cluster   
    of eigenvalues appears in the leading diagonal blocks of the upper   
    triangular matrix S and the upper triangular matrix T; computes   
    a reciprocal condition number for the average of the selected   
    eigenvalues (RCONDE); and computes a reciprocal condition number for   
    the right and left deflating subspaces corresponding to the selected   
    eigenvalues (RCONDV). The leading columns of VSL and VSR then form   
    an orthonormal basis for the corresponding left and right eigenspaces   
    (deflating subspaces).   

    A generalized eigenvalue for a pair of matrices (A,B) is a scalar w   
    or a ratio alpha/beta = w, such that  A - w*B is singular.  It is   
    usually represented as the pair (alpha,beta), as there is a   
    reasonable interpretation for beta=0 or for both being zero.   

    A pair of matrices (S,T) is in generalized complex Schur form if T is   
    upper triangular with non-negative diagonal and S is upper   
    triangular.   

    Arguments   
    =========   

    JOBVSL  (input) CHARACTER*1   
            = 'N':  do not compute the left Schur vectors;   
            = 'V':  compute the left Schur vectors.   

    JOBVSR  (input) CHARACTER*1   
            = 'N':  do not compute the right Schur vectors;   
            = 'V':  compute the right Schur vectors.   

    SORT    (input) CHARACTER*1   
            Specifies whether or not to order the eigenvalues on the   
            diagonal of the generalized Schur form.   
            = 'N':  Eigenvalues are not ordered;   
            = 'S':  Eigenvalues are ordered (see SELCTG).   

    SELCTG  (input) LOGICAL FUNCTION of two COMPLEX arguments   
            SELCTG must be declared EXTERNAL in the calling subroutine.   
            If SORT = 'N', SELCTG is not referenced.   
            If SORT = 'S', SELCTG is used to select eigenvalues to sort   
            to the top left of the Schur form.   
            Note that a selected complex eigenvalue may no longer satisfy   
            SELCTG(ALPHA(j),BETA(j)) = .TRUE. after ordering, since   
            ordering may change the value of complex eigenvalues   
            (especially if the eigenvalue is ill-conditioned), in this   
            case INFO is set to N+3 see INFO below).   

    SENSE   (input) CHARACTER   
            Determines which reciprocal condition numbers are computed.   
            = 'N' : None are computed;   
            = 'E' : Computed for average of selected eigenvalues only;   
            = 'V' : Computed for selected deflating subspaces only;   
            = 'B' : Computed for both.   
            If SENSE = 'E', 'V', or 'B', SORT must equal 'S'.   

    N       (input) INTEGER   
            The order of the matrices A, B, VSL, and VSR.  N >= 0.   

    A       (input/output) COMPLEX array, dimension (LDA, N)   
            On entry, the first of the pair of matrices.   
            On exit, A has been overwritten by its generalized Schur   
            form S.   

    LDA     (input) INTEGER   
            The leading dimension of A.  LDA >= max(1,N).   

    B       (input/output) COMPLEX array, dimension (LDB, N)   
            On entry, the second of the pair of matrices.   
            On exit, B has been overwritten by its generalized Schur   
            form T.   

    LDB     (input) INTEGER   
            The leading dimension of B.  LDB >= max(1,N).   

    SDIM    (output) INTEGER   
            If SORT = 'N', SDIM = 0.   
            If SORT = 'S', SDIM = number of eigenvalues (after sorting)   
            for which SELCTG is true.   

    ALPHA   (output) COMPLEX array, dimension (N)   
    BETA    (output) COMPLEX array, dimension (N)   
            On exit, ALPHA(j)/BETA(j), j=1,...,N, will be the   
            generalized eigenvalues.  ALPHA(j) and BETA(j),j=1,...,N  are   
            the diagonals of the complex Schur form (S,T).  BETA(j) will   
            be non-negative real.   

            Note: the quotients ALPHA(j)/BETA(j) may easily over- or   
            underflow, and BETA(j) may even be zero.  Thus, the user   
            should avoid naively computing the ratio alpha/beta.   
            However, ALPHA will be always less than and usually   
            comparable with norm(A) in magnitude, and BETA always less   
            than and usually comparable with norm(B).   

    VSL     (output) COMPLEX array, dimension (LDVSL,N)   
            If JOBVSL = 'V', VSL will contain the left Schur vectors.   
            Not referenced if JOBVSL = 'N'.   

    LDVSL   (input) INTEGER   
            The leading dimension of the matrix VSL. LDVSL >=1, and   
            if JOBVSL = 'V', LDVSL >= N.   

    VSR     (output) COMPLEX array, dimension (LDVSR,N)   
            If JOBVSR = 'V', VSR will contain the right Schur vectors.   
            Not referenced if JOBVSR = 'N'.   

    LDVSR   (input) INTEGER   
            The leading dimension of the matrix VSR. LDVSR >= 1, and   
            if JOBVSR = 'V', LDVSR >= N.   

    RCONDE  (output) REAL array, dimension ( 2 )   
            If SENSE = 'E' or 'B', RCONDE(1) and RCONDE(2) contain the   
            reciprocal condition numbers for the average of the selected   
            eigenvalues.   
            Not referenced if SENSE = 'N' or 'V'.   

    RCONDV  (output) REAL array, dimension ( 2 )   
            If SENSE = 'V' or 'B', RCONDV(1) and RCONDV(2) contain the   
            reciprocal condition number for the selected deflating   
            subspaces.   
            Not referenced if SENSE = 'N' or 'E'.   

    WORK    (workspace/output) COMPLEX array, dimension (LWORK)   
            On exit, if INFO = 0, WORK(1) returns the optimal LWORK.   

    LWORK   (input) INTEGER   
            The dimension of the array WORK.  LWORK >= 2*N.   
            If SENSE = 'E', 'V', or 'B',   
            LWORK >= MAX(2*N, 2*SDIM*(N-SDIM)).   

    RWORK   (workspace) REAL array, dimension ( 8*N )   
            Real workspace.   

    IWORK   (workspace/output) INTEGER array, dimension (LIWORK)   
            Not referenced if SENSE = 'N'.   
            On exit, if INFO = 0, IWORK(1) returns the optimal LIWORK.   

    LIWORK  (input) INTEGER   
            The dimension of the array WORK. LIWORK >= N+2.   

    BWORK   (workspace) LOGICAL array, dimension (N)   
            Not referenced if SORT = 'N'.   

    INFO    (output) INTEGER   
            = 0:  successful exit   
            < 0:  if INFO = -i, the i-th argument had an illegal value.   
            = 1,...,N:   
                  The QZ iteration failed.  (A,B) are not in Schur   
                  form, but ALPHA(j) and BETA(j) should be correct for   
                  j=INFO+1,...,N.   
            > N:  =N+1: other than QZ iteration failed in CHGEQZ   
                  =N+2: after reordering, roundoff changed values of   
                        some complex eigenvalues so that leading   
                        eigenvalues in the Generalized Schur form no   
                        longer satisfy SELCTG=.TRUE.  This could also   
                        be caused due to scaling.   
                  =N+3: reordering failed in CTGSEN.   

    =====================================================================   


       Decode the input arguments   

       Parameter adjustments */
    /* Table of constant values */
    static complex c_b1 = {0.f,0.f};
    static complex c_b2 = {1.f,0.f};
    static integer c__1 = 1;
    static integer c__0 = 0;
    static integer c_n1 = -1;
    
    /* System generated locals */
    integer a_dim1, a_offset, b_dim1, b_offset, vsl_dim1, vsl_offset, 
	    vsr_dim1, vsr_offset, i__1, i__2;
    /* Builtin functions */
    double sqrt(doublereal);
    /* Local variables */
    static integer ijob;
    static real anrm, bnrm;
    static integer ierr, itau, iwrk, i__;
    extern logical lsame_(char *, char *);
    static integer ileft, icols;
    static logical cursl, ilvsl, ilvsr;
    static integer irwrk, irows;
    extern /* Subroutine */ int cggbak_(char *, char *, integer *, integer *, 
	    integer *, real *, real *, integer *, complex *, integer *, 
	    integer *), cggbal_(char *, integer *, complex *, 
	    integer *, complex *, integer *, integer *, integer *, real *, 
	    real *, real *, integer *), slabad_(real *, real *);
    extern doublereal clange_(char *, integer *, integer *, complex *, 
	    integer *, real *);
    static real pl;
    extern /* Subroutine */ int cgghrd_(char *, char *, integer *, integer *, 
	    integer *, complex *, integer *, complex *, integer *, complex *, 
	    integer *, complex *, integer *, integer *), 
	    clascl_(char *, integer *, integer *, real *, real *, integer *, 
	    integer *, complex *, integer *, integer *);
    static real pr;
    static logical ilascl, ilbscl;
    extern /* Subroutine */ int cgeqrf_(integer *, integer *, complex *, 
	    integer *, complex *, complex *, integer *, integer *), clacpy_(
	    char *, integer *, integer *, complex *, integer *, complex *, 
	    integer *), claset_(char *, integer *, integer *, complex 
	    *, complex *, complex *, integer *), xerbla_(char *, 
	    integer *);
    extern integer ilaenv_(integer *, char *, char *, integer *, integer *, 
	    integer *, integer *, ftnlen, ftnlen);
    extern doublereal slamch_(char *);
    static real bignum;
    extern /* Subroutine */ int chgeqz_(char *, char *, char *, integer *, 
	    integer *, integer *, complex *, integer *, complex *, integer *, 
	    complex *, complex *, complex *, integer *, complex *, integer *, 
	    complex *, integer *, real *, integer *), 
	    ctgsen_(integer *, logical *, logical *, logical *, integer *, 
	    complex *, integer *, complex *, integer *, complex *, complex *, 
	    complex *, integer *, complex *, integer *, integer *, real *, 
	    real *, real *, complex *, integer *, integer *, integer *, 
	    integer *);
    static integer ijobvl, iright, ijobvr;
    static logical wantsb;
    static integer liwmin;
    static logical wantse, lastsl;
    static real anrmto, bnrmto;
    extern /* Subroutine */ int cungqr_(integer *, integer *, integer *, 
	    complex *, integer *, complex *, complex *, integer *, integer *);
    static integer minwrk, maxwrk;
    static logical wantsn;
    static real smlnum;
    extern /* Subroutine */ int cunmqr_(char *, char *, integer *, integer *, 
	    integer *, complex *, integer *, complex *, complex *, integer *, 
	    complex *, integer *, integer *);
    static logical wantst, wantsv;
    static real dif[2];
    static integer ihi, ilo;
    static real eps;
#define a_subscr(a_1,a_2) (a_2)*a_dim1 + a_1
#define a_ref(a_1,a_2) a[a_subscr(a_1,a_2)]
#define b_subscr(a_1,a_2) (a_2)*b_dim1 + a_1
#define b_ref(a_1,a_2) b[b_subscr(a_1,a_2)]
#define vsl_subscr(a_1,a_2) (a_2)*vsl_dim1 + a_1
#define vsl_ref(a_1,a_2) vsl[vsl_subscr(a_1,a_2)]


    a_dim1 = *lda;
    a_offset = 1 + a_dim1 * 1;
    a -= a_offset;
    b_dim1 = *ldb;
    b_offset = 1 + b_dim1 * 1;
    b -= b_offset;
    --alpha;
    --beta;
    vsl_dim1 = *ldvsl;
    vsl_offset = 1 + vsl_dim1 * 1;
    vsl -= vsl_offset;
    vsr_dim1 = *ldvsr;
    vsr_offset = 1 + vsr_dim1 * 1;
    vsr -= vsr_offset;
    --rconde;
    --rcondv;
    --work;
    --rwork;
    --iwork;
    --bwork;

    /* Function Body */
    if (lsame_(jobvsl, "N")) {
	ijobvl = 1;
	ilvsl = FALSE_;
    } else if (lsame_(jobvsl, "V")) {
	ijobvl = 2;
	ilvsl = TRUE_;
    } else {
	ijobvl = -1;
	ilvsl = FALSE_;
    }

    if (lsame_(jobvsr, "N")) {
	ijobvr = 1;
	ilvsr = FALSE_;
    } else if (lsame_(jobvsr, "V")) {
	ijobvr = 2;
	ilvsr = TRUE_;
    } else {
	ijobvr = -1;
	ilvsr = FALSE_;
    }

    wantst = lsame_(sort, "S");
    wantsn = lsame_(sense, "N");
    wantse = lsame_(sense, "E");
    wantsv = lsame_(sense, "V");
    wantsb = lsame_(sense, "B");
    if (wantsn) {
	ijob = 0;
	iwork[1] = 1;
    } else if (wantse) {
	ijob = 1;
    } else if (wantsv) {
	ijob = 2;
    } else if (wantsb) {
	ijob = 4;
    }

/*     Test the input arguments */

    *info = 0;
    if (ijobvl <= 0) {
	*info = -1;
    } else if (ijobvr <= 0) {
	*info = -2;
    } else if (! wantst && ! lsame_(sort, "N")) {
	*info = -3;
    } else if (! (wantsn || wantse || wantsv || wantsb) || ! wantst && ! 
	    wantsn) {
	*info = -5;
    } else if (*n < 0) {
	*info = -6;
    } else if (*lda < max(1,*n)) {
	*info = -8;
    } else if (*ldb < max(1,*n)) {
	*info = -10;
    } else if (*ldvsl < 1 || ilvsl && *ldvsl < *n) {
	*info = -15;
    } else if (*ldvsr < 1 || ilvsr && *ldvsr < *n) {
	*info = -17;
    }

/*     Compute workspace   
        (Note: Comments in the code beginning "Workspace:" describe the   
         minimal amount of workspace needed at that point in the code,   
         as well as the preferred amount for good performance.   
         NB refers to the optimal block size for the immediately   
         following subroutine, as returned by ILAENV.) */

    minwrk = 1;
    if (*info == 0 && *lwork >= 1) {
/* Computing MAX */
	i__1 = 1, i__2 = *n << 1;
	minwrk = max(i__1,i__2);
	maxwrk = *n + *n * ilaenv_(&c__1, "CGEQRF", " ", n, &c__1, n, &c__0, (
		ftnlen)6, (ftnlen)1);
	if (ilvsl) {
/* Computing MAX */
	    i__1 = maxwrk, i__2 = *n + *n * ilaenv_(&c__1, "CUNGQR", " ", n, &
		    c__1, n, &c_n1, (ftnlen)6, (ftnlen)1);
	    maxwrk = max(i__1,i__2);
	}
	work[1].r = (real) maxwrk, work[1].i = 0.f;
    }
    if (! wantsn) {
	liwmin = *n + 2;
    } else {
	liwmin = 1;
    }
    iwork[1] = liwmin;

    if (*info == 0 && *lwork < minwrk) {
	*info = -21;
    } else if (*info == 0 && ijob >= 1) {
	if (*liwork < liwmin) {
	    *info = -24;
	}
    }

    if (*info != 0) {
	i__1 = -(*info);
	xerbla_("CGGESX", &i__1);
	return 0;
    }

/*     Quick return if possible */

    if (*n == 0) {
	*sdim = 0;
	return 0;
    }

/*     Get machine constants */

    eps = slamch_("P");
    smlnum = slamch_("S");
    bignum = 1.f / smlnum;
    slabad_(&smlnum, &bignum);
    smlnum = sqrt(smlnum) / eps;
    bignum = 1.f / smlnum;

/*     Scale A if max element outside range [SMLNUM,BIGNUM] */

    anrm = clange_("M", n, n, &a[a_offset], lda, &rwork[1]);
    ilascl = FALSE_;
    if (anrm > 0.f && anrm < smlnum) {
	anrmto = smlnum;
	ilascl = TRUE_;
    } else if (anrm > bignum) {
	anrmto = bignum;
	ilascl = TRUE_;
    }
    if (ilascl) {
	clascl_("G", &c__0, &c__0, &anrm, &anrmto, n, n, &a[a_offset], lda, &
		ierr);
    }

/*     Scale B if max element outside range [SMLNUM,BIGNUM] */

    bnrm = clange_("M", n, n, &b[b_offset], ldb, &rwork[1]);
    ilbscl = FALSE_;
    if (bnrm > 0.f && bnrm < smlnum) {
	bnrmto = smlnum;
	ilbscl = TRUE_;
    } else if (bnrm > bignum) {
	bnrmto = bignum;
	ilbscl = TRUE_;
    }
    if (ilbscl) {
	clascl_("G", &c__0, &c__0, &bnrm, &bnrmto, n, n, &b[b_offset], ldb, &
		ierr);
    }

/*     Permute the matrix to make it more nearly triangular   
       (Real Workspace: need 6*N) */

    ileft = 1;
    iright = *n + 1;
    irwrk = iright + *n;
    cggbal_("P", n, &a[a_offset], lda, &b[b_offset], ldb, &ilo, &ihi, &rwork[
	    ileft], &rwork[iright], &rwork[irwrk], &ierr);

/*     Reduce B to triangular form (QR decomposition of B)   
       (Complex Workspace: need N, prefer N*NB) */

    irows = ihi + 1 - ilo;
    icols = *n + 1 - ilo;
    itau = 1;
    iwrk = itau + irows;
    i__1 = *lwork + 1 - iwrk;
    cgeqrf_(&irows, &icols, &b_ref(ilo, ilo), ldb, &work[itau], &work[iwrk], &
	    i__1, &ierr);

/*     Apply the unitary transformation to matrix A   
       (Complex Workspace: need N, prefer N*NB) */

    i__1 = *lwork + 1 - iwrk;
    cunmqr_("L", "C", &irows, &icols, &irows, &b_ref(ilo, ilo), ldb, &work[
	    itau], &a_ref(ilo, ilo), lda, &work[iwrk], &i__1, &ierr);

/*     Initialize VSL   
       (Complex Workspace: need N, prefer N*NB) */

    if (ilvsl) {
	claset_("Full", n, n, &c_b1, &c_b2, &vsl[vsl_offset], ldvsl);
	i__1 = irows - 1;
	i__2 = irows - 1;
	clacpy_("L", &i__1, &i__2, &b_ref(ilo + 1, ilo), ldb, &vsl_ref(ilo + 
		1, ilo), ldvsl);
	i__1 = *lwork + 1 - iwrk;
	cungqr_(&irows, &irows, &irows, &vsl_ref(ilo, ilo), ldvsl, &work[itau]
		, &work[iwrk], &i__1, &ierr);
    }

/*     Initialize VSR */

    if (ilvsr) {
	claset_("Full", n, n, &c_b1, &c_b2, &vsr[vsr_offset], ldvsr);
    }

/*     Reduce to generalized Hessenberg form   
       (Workspace: none needed) */

    cgghrd_(jobvsl, jobvsr, n, &ilo, &ihi, &a[a_offset], lda, &b[b_offset], 
	    ldb, &vsl[vsl_offset], ldvsl, &vsr[vsr_offset], ldvsr, &ierr);

    *sdim = 0;

/*     Perform QZ algorithm, computing Schur vectors if desired   
       (Complex Workspace: need N)   
       (Real Workspace:    need N) */

    iwrk = itau;
    i__1 = *lwork + 1 - iwrk;
    chgeqz_("S", jobvsl, jobvsr, n, &ilo, &ihi, &a[a_offset], lda, &b[
	    b_offset], ldb, &alpha[1], &beta[1], &vsl[vsl_offset], ldvsl, &
	    vsr[vsr_offset], ldvsr, &work[iwrk], &i__1, &rwork[irwrk], &ierr);
    if (ierr != 0) {
	if (ierr > 0 && ierr <= *n) {
	    *info = ierr;
	} else if (ierr > *n && ierr <= *n << 1) {
	    *info = ierr - *n;
	} else {
	    *info = *n + 1;
	}
	goto L40;
    }

/*     Sort eigenvalues ALPHA/BETA and compute the reciprocal of   
       condition number(s) */

    if (wantst) {

/*        Undo scaling on eigenvalues before SELCTGing */

	if (ilascl) {
	    clascl_("G", &c__0, &c__0, &anrmto, &anrm, n, &c__1, &alpha[1], n,
		     &ierr);
	}
	if (ilbscl) {
	    clascl_("G", &c__0, &c__0, &bnrmto, &bnrm, n, &c__1, &beta[1], n, 
		    &ierr);
	}

/*        Select eigenvalues */

	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    bwork[i__] = (*selctg)(&alpha[i__], &beta[i__]);
/* L10: */
	}

/*        Reorder eigenvalues, transform Generalized Schur vectors, and   
          compute reciprocal condition numbers   
          (Complex Workspace: If IJOB >= 1, need MAX(1, 2*SDIM*(N-SDIM))   
                              otherwise, need 1 ) */

	i__1 = *lwork - iwrk + 1;
	ctgsen_(&ijob, &ilvsl, &ilvsr, &bwork[1], n, &a[a_offset], lda, &b[
		b_offset], ldb, &alpha[1], &beta[1], &vsl[vsl_offset], ldvsl, 
		&vsr[vsr_offset], ldvsr, sdim, &pl, &pr, dif, &work[iwrk], &
		i__1, &iwork[1], liwork, &ierr);

	if (ijob >= 1) {
/* Computing MAX */
	    i__1 = maxwrk, i__2 = (*sdim << 1) * (*n - *sdim);
	    maxwrk = max(i__1,i__2);
	}
	if (ierr == -21) {

/*            not enough complex workspace */

	    *info = -21;
	} else {
	    rconde[1] = pl;
	    rconde[2] = pl;
	    rcondv[1] = dif[0];
	    rcondv[2] = dif[1];
	    if (ierr == 1) {
		*info = *n + 3;
	    }
	}

    }

/*     Apply permutation to VSL and VSR   
       (Workspace: none needed) */

    if (ilvsl) {
	cggbak_("P", "L", n, &ilo, &ihi, &rwork[ileft], &rwork[iright], n, &
		vsl[vsl_offset], ldvsl, &ierr);
    }

    if (ilvsr) {
	cggbak_("P", "R", n, &ilo, &ihi, &rwork[ileft], &rwork[iright], n, &
		vsr[vsr_offset], ldvsr, &ierr);
    }

/*     Undo scaling */

    if (ilascl) {
	clascl_("U", &c__0, &c__0, &anrmto, &anrm, n, n, &a[a_offset], lda, &
		ierr);
	clascl_("G", &c__0, &c__0, &anrmto, &anrm, n, &c__1, &alpha[1], n, &
		ierr);
    }

    if (ilbscl) {
	clascl_("U", &c__0, &c__0, &bnrmto, &bnrm, n, n, &b[b_offset], ldb, &
		ierr);
	clascl_("G", &c__0, &c__0, &bnrmto, &bnrm, n, &c__1, &beta[1], n, &
		ierr);
    }

/* L20: */

    if (wantst) {

/*        Check if reordering is correct */

	lastsl = TRUE_;
	*sdim = 0;
	i__1 = *n;
	for (i__ = 1; i__ <= i__1; ++i__) {
	    cursl = (*selctg)(&alpha[i__], &beta[i__]);
	    if (cursl) {
		++(*sdim);
	    }
	    if (cursl && ! lastsl) {
		*info = *n + 2;
	    }
	    lastsl = cursl;
/* L30: */
	}

    }

L40:

    work[1].r = (real) maxwrk, work[1].i = 0.f;
    iwork[1] = liwmin;

    return 0;

/*     End of CGGESX */

} /* cggesx_ */

#undef vsl_ref
#undef vsl_subscr
#undef b_ref
#undef b_subscr
#undef a_ref
#undef a_subscr


