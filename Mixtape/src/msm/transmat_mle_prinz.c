#include <float.h>tmp
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "transmat_mle_prinz.h"

/**
 * Compute a maximum likelihood reversible transition matrix, given
 * a set of directed transition counts.
 *
 * Algorithim 1 of Prinz et al.[1]
 *
 * Parameters
 * ----------
 * C : (input) pointer to a dense 2d array of shape=(n_states, n_states)
 *     The directed transition counts, in C (row-major) order.
 * n_state : (input) int
 *     The number of states, and dimension of the C matrix
 * tol : (input) float
 *     Convergence tolerance. The algorithim will iterate until the
 *     change in the log-likelihood is les than `tol`.
 * T : (output) pointer to output 2d array of shape=(n_states, n_states)
 *     The output transition matrix will be written to `T`.
 * pi : (output) pointer to output 1d array of shape=(n_states,)
 *     The stationary eigenvector of the output transition matrix will
 *     be written to pi
 * 
 * Returns
 * -------
 * n_iter : int
 *     Number of iterations performed. A value of n_iter < 0 indicates
 *     failure.
 *
 * 
 * References
 * ----------
 * .. [1] Prinz, Jan-Hendrik, et al. "Markov models of molecular kinetics:
 *    Generation and validation." J Chem. Phys. 134.17 (2011): 174105.
 */
int transmat_mle_prinz(const double* C, int n_states, double tol,
                       double* T, double* pi)
{
    double a, b, c, v, tmp, pi_sum;
    double *X, *X_RS, *C_RS;
    int iter = 0;
    int i, j;
#ifdef DEBUG
    int k;
#endif
    double logl, oldlogl;
    oldlogl = FLT_MAX;;

#define x(m,n) (X[m*n_states + n])
#define c(m,n) (C[m*n_states + n])
#define x_rs(m) (X_RS[m])
#define c_rs(m) (C_RS[m])

    X = (double*) malloc(n_states*n_states*sizeof(double));
    X_RS = (double*) malloc(n_states*sizeof(double));
    C_RS = (double*) malloc(n_states*sizeof(double));

    /* initialize X */
    for (i = 0; i < n_states; i++)
        for (j = 0; j < n_states; j++)
            x(i,j) = c(i,j) + c(j,i);

    /* initialize x_rs and c_rs */
    for (i = 0; i < n_states; i++) {
        x_rs(i) = 0;
        c_rs(i) = 0;
        for (j = 0; j < n_states; j++) {
            x_rs(i) += x(i,j);
            c_rs(i) += c(i,j);
        }
    }

    for (iter=0; fabs(oldlogl - logl) >= tol; iter++) {
        oldlogl = logl;
        logl = 0;

        /* update xii */
        for (i = 0; i < n_states; i++) {
            tmp = x(i,i);
            x(i,i) = c(i,i) * (x_rs(i) - x(i,i)) / (c_rs(i) - c(i,i));

            x_rs(i) = x_rs(i) + (x(i,i) - tmp);
#ifdef DEBUG
            x_rs(i) = 0;
            for (j = 0; j < n_states; j++)
                x_rs(i) += x(i,j);
#endif
            logl += c(i,i) * log(x(i,i) / x_rs(i));
        }

        /* update X for the offdiagonal entries */
        for (i = 0; i < n_states-1; i++) {
            for (j = i+1; j < n_states; j++) {

                a = (c_rs(i) - c(i,j)) + (c_rs(j) - c(j,i));
                b = c_rs(i) * (x_rs(j) - x(i,j)) \
                    + c_rs(j) * (x_rs(i) - x(i,j))
                    - (c(i,j) + c(j,i)) * (x_rs(i) + x_rs(j) - 2*x(i,j));

                c = -(c(i,j) + c(j,i)) * \
                    (x_rs(i) - x(i,j)) * \
                    (x_rs(j) - x(i,j));

                if (c > 0)
                    /* exit error */
                    return -1;

                /* the new value */
                v = (-b + sqrt((b*b) - (4*a*c))) / (2*a);

                /* update the row sums */
                x_rs(i) = x_rs(i) + (v - x(i,j));
                x_rs(j) = x_rs(j) + (v - x(j,i));

                /* add in the new value */
                x(i,j) = x(j,i) = v;
#ifdef DEBUG
                x_rs(i) = 0;
                for (k = 0; k < n_states; k++)
                    x_rs(i) += x(i, k);
                x_rs(j) = 0;
                for (k = 0; k < n_states; k++)
                    x_rs(j) += x(j, k);
#endif

                logl += c(i,j) * log(x(i,j) / x_rs(i)) +
                        c(j,i) * log(x(j,i) / x_rs(j));

            }
        }

        /* printf("logl = %f\n", logl); */
    }

    pi_sum = 0;
    for (i = 0; i < n_states; i++) {
        pi_sum += x_rs(i);
        for (j = 0; j < n_states; j++) {
            T[i*n_states+j] = x(i,j) / x_rs(i);
        }
    }
    for (i = 0; i < n_states; i++)
        pi[i] = x_rs(i) / pi_sum;

    free(X);
    free(X_RS);
    free(C_RS);
    /* exit success */
    return iter;
}
