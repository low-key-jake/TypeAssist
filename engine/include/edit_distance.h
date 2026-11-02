/**
 * TypeAssist — Damerau-Levenshtein Edit Distance
 *
 * Computes the minimum number of single-character operations needed
 * to transform one string into another. Supports four operations:
 *   1. Insertion
 *   2. Deletion
 *   3. Substitution
 *   4. Adjacent transposition (Damerau extension)
 *
 * Example: "recieve" → "receive" = distance 1 (transposition of 'ie' → 'ei')
 *
 * Time Complexity:  O(m × n) where m,n are string lengths
 * Space Complexity: O(m × n)
 */

#ifndef TYPEASSIST_EDIT_DISTANCE_H
#define TYPEASSIST_EDIT_DISTANCE_H

/**
 * Compute the Damerau-Levenshtein distance between two strings.
 *
 * @param s1   First string (e.g., the misspelled word)
 * @param s2   Second string (e.g., a dictionary word)
 * @return     The edit distance (>= 0), or -1 on error
 */
int damerau_levenshtein(const char *s1, const char *s2);

/**
 * Compute the standard Levenshtein distance (no transpositions).
 * Useful for comparison and testing.
 *
 * @param s1   First string
 * @param s2   Second string
 * @return     The edit distance (>= 0), or -1 on error
 */
int levenshtein(const char *s1, const char *s2);

/**
 * Compute optimal string alignment distance.
 * Like Damerau-Levenshtein but does not allow a substring
 * to be edited more than once (simpler, slightly less accurate).
 *
 * @param s1   First string
 * @param s2   Second string
 * @return     The edit distance (>= 0), or -1 on error
 */
int optimal_string_alignment(const char *s1, const char *s2);

#endif /* TYPEASSIST_EDIT_DISTANCE_H */
