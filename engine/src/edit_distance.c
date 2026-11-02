/**
 * TypeAssist — Edit Distance Implementations
 *
 * Three flavours of edit distance:
 *   1. Levenshtein        — insert, delete, substitute
 *   2. Damerau-Levenshtein — + unrestricted adjacent transposition
 *   3. Optimal String Alignment — + restricted transposition (no re-editing)
 *
 * All functions return -1 on NULL input.
 */

#include "edit_distance.h"
#include <stdlib.h>
#include <string.h>

/* ---------- Helpers ---------- */

static int min2(int a, int b) { return a < b ? a : b; }
static int min3(int a, int b, int c) { return min2(a, min2(b, c)); }

/**
 * Allocate a 2-D int matrix of dimensions (rows x cols) as a flat block.
 * Returns NULL on failure.  Caller must free(*mat) then free(mat).
 */
static int **alloc_matrix(int rows, int cols)
{
    int **mat = malloc(sizeof(int *) * (size_t)rows);
    if (!mat)
        return NULL;

    int *block = malloc(sizeof(int) * (size_t)rows * (size_t)cols);
    if (!block) {
        free(mat);
        return NULL;
    }

    for (int i = 0; i < rows; i++)
        mat[i] = block + i * cols;

    return mat;
}

static void free_matrix(int **mat)
{
    if (!mat)
        return;
    free(mat[0]);   /* the flat block */
    free(mat);
}

/* ---------- Levenshtein ---------- */

int levenshtein(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return -1;

    int m = (int)strlen(s1);
    int n = (int)strlen(s2);

    /* Edge cases: one or both strings empty */
    if (m == 0) return n;
    if (n == 0) return m;

    int **d = alloc_matrix(m + 1, n + 1);
    if (!d)
        return -1;

    for (int i = 0; i <= m; i++) d[i][0] = i;
    for (int j = 0; j <= n; j++) d[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            d[i][j] = min3(
                d[i - 1][j]     + 1,      /* deletion */
                d[i][j - 1]     + 1,      /* insertion */
                d[i - 1][j - 1] + cost    /* substitution */
            );
        }
    }

    int result = d[m][n];
    free_matrix(d);
    return result;
}

/* ---------- Optimal String Alignment ---------- */

int optimal_string_alignment(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return -1;

    int m = (int)strlen(s1);
    int n = (int)strlen(s2);

    if (m == 0) return n;
    if (n == 0) return m;

    int **d = alloc_matrix(m + 1, n + 1);
    if (!d)
        return -1;

    for (int i = 0; i <= m; i++) d[i][0] = i;
    for (int j = 0; j <= n; j++) d[0][j] = j;

    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            d[i][j] = min3(
                d[i - 1][j]     + 1,
                d[i][j - 1]     + 1,
                d[i - 1][j - 1] + cost
            );

            /* Transposition check (restricted: substring edited at most once) */
            if (i > 1 && j > 1 &&
                s1[i - 1] == s2[j - 2] &&
                s1[i - 2] == s2[j - 1]) {
                d[i][j] = min2(d[i][j], d[i - 2][j - 2] + 1);
            }
        }
    }

    int result = d[m][n];
    free_matrix(d);
    return result;
}

/* ---------- Full Damerau-Levenshtein ---------- */

/**
 * Full Damerau-Levenshtein distance using the algorithm with the "da" array
 * that tracks the last row where each character appeared.  This allows
 * unrestricted transpositions (a substring may be edited multiple times).
 *
 * Reference: Wikipedia "Damerau–Levenshtein distance" —
 *   the algorithm attributed to Lowrance & Wagner (1975).
 */
int damerau_levenshtein(const char *s1, const char *s2)
{
    if (!s1 || !s2)
        return -1;

    int m = (int)strlen(s1);
    int n = (int)strlen(s2);

    if (m == 0) return n;
    if (n == 0) return m;

    /*
     * da[c] = last row in s1 where character c was seen.
     * We support the first 128 ASCII characters.
     */
    #define ALPHABET_SIZE 128
    int da[ALPHABET_SIZE];
    memset(da, 0, sizeof(da));

    /*
     * d is dimensioned (m+2) x (n+2).  Index 0 is a sentinel row/column
     * representing "beyond the boundary", filled with maxval.
     * Actual string indices map to d[i+1][j+1].
     */
    int rows = m + 2;
    int cols = n + 2;

    int **d = alloc_matrix(rows, cols);
    if (!d)
        return -1;

    int maxdist = m + n;   /* upper-bound sentinel value */

    d[0][0] = maxdist;
    for (int i = 0; i <= m; i++) {
        d[i + 1][0] = maxdist;
        d[i + 1][1] = i;
    }
    for (int j = 0; j <= n; j++) {
        d[0][j + 1] = maxdist;
        d[1][j + 1] = j;
    }

    for (int i = 1; i <= m; i++) {
        int db = 0;   /* last column in s2 where s1[i-1] was matched */

        for (int j = 1; j <= n; j++) {
            int i1 = da[(unsigned char)s2[j - 1]];   /* last row in s1 matching s2[j-1] */
            int j1 = db;
            int cost;

            if (s1[i - 1] == s2[j - 1]) {
                cost = 0;
                db = j;
            } else {
                cost = 1;
            }

            d[i + 1][j + 1] = min3(
                d[i][j] + cost,                         /* substitution */
                d[i + 1][j] + 1,                        /* insertion */
                d[i][j + 1] + 1                         /* deletion */
            );

            /* Transposition cost */
            int trans = d[i1][j1]
                        + (i - i1 - 1)   /* deletions between */
                        + 1              /* transposition */
                        + (j - j1 - 1);  /* insertions between */
            d[i + 1][j + 1] = min2(d[i + 1][j + 1], trans);
        }

        da[(unsigned char)s1[i - 1]] = i;
    }

    int result = d[m + 1][n + 1];
    free_matrix(d);
    return result;

    #undef ALPHABET_SIZE
}
