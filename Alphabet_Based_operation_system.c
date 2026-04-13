// ============================================================
//  All word operations work on alphabet-based Word structs.
//  Includes automaton validation.
// ============================================================

#include <stdio.h>

#define MAX        200   /* max raw string length              */
#define MAX_SYMS    20   /* max symbols in alphabet            */
#define SYM_LEN     10   /* max characters per symbol          */
#define MAX_WORD   100   /* max symbols in a word              */

/* ---- Alphabet ---- */
typedef struct {
    char sym[MAX_SYMS][SYM_LEN];
    int  size;
} Alphabet;

/* ---- Word: stored as array of symbol indices ---- */
typedef struct {
    int idx[MAX_WORD];
    int len;
} Word;

//   Helper string functions (no <string.h>)

int str_len(char s[]) {
    if (s[0] == '\0') return 0;
    return 1 + str_len(s + 1);
}

int str_eq(char a[], char b[]) {
    if (a[0] == '\0' && b[0] == '\0') return 1;
    if (a[0] != b[0]) return 0;
    return str_eq(a + 1, b + 1);
}

/* ===========================================================
   Alphabet & Word I/O
   =========================================================== */

/* Create the alphabet interactively */
void create_alphabet(Alphabet *al) {
    char s[SYM_LEN];
    char more;
    int i;
    al->size = 0;

    printf("\nEnter alphabet symbols (can be multi-char, e.g. \"ab\"):\n");
    do {
        printf("  Symbol: ");
        scanf("%s", s);

        /* Reject duplicates */
        int dup = 0;
        for (i = 0; i < al->size; i++)
            if (str_eq(al->sym[i], s)) { dup = 1; break; }

        if (dup) {
            printf("  Already exists.\n");
        } else {
            /* Copy symbol into alphabet */
            int k = 0;
            while (s[k] != '\0') { al->sym[al->size][k] = s[k]; k++; }
            al->sym[al->size][k] = '\0';
            al->size++;
        }
        printf("  Add more? (y/n): ");
        scanf(" %c", &more);
    } while (more == 'y' && al->size < MAX_SYMS);

    /* Print the alphabet */
    printf("Alphabet = { ");
    for (i = 0; i < al->size; i++)
        printf("\"%s\"%s", al->sym[i], i < al->size - 1 ? ", " : "");
    printf(" }\n");
}

/* Parse raw string into Word using greedy longest-match.
   Returns 1 on success, 0 if a character cannot be matched. */
int parse_word(char raw[], Alphabet *al, Word *w) {
    int pos = 0, raw_l = str_len(raw);
    w->len = 0;
    while (pos < raw_l) {
        int best = -1, best_l = 0, i, j;
        for (i = 0; i < al->size; i++) {
            int sl = str_len(al->sym[i]);
            if (sl <= best_l) continue;
            /* Check if raw[pos..] starts with al->sym[i] */
            int match = 1;
            for (j = 0; j < sl; j++)
                if (raw[pos + j] != al->sym[i][j]) { match = 0; break; }
            if (match) { best = i; best_l = sl; }
        }
        if (best == -1) {
            printf("  [!] Cannot parse at position %d (char '%c')\n",
                   pos, raw[pos]);
            return 0;   /* word not in the language */
        }
        w->idx[w->len++] = best;
        pos += best_l;
    }
    return 1;   /* word is in the language */
}

/* Read a word from stdin; loops until valid */
void read_word(Alphabet *al, Word *w) {
    char raw[MAX];
    int ok = 0;
    while (!ok) {
        printf("  Word (raw string): ");
        scanf("%s", raw);
        ok = parse_word(raw, al, w);
        if (!ok) printf("  Word rejected — not in the language. Try again.\n");
    }
}

/* Print a word as a sequence of its symbols */
void print_word(Alphabet *al, Word *w) {
    int i;
    printf("[ ");
    for (i = 0; i < w->len; i++)
        printf("\"%s\"%s", al->sym[w->idx[i]], i < w->len - 1 ? ", " : "");
    printf(" ]");
}

/* ===========================================================
   1. Length — number of symbols in the word (recursive)
   =========================================================== */
int word_length(Word *w, int pos) {
    if (pos == w->len) return 0;
    return 1 + word_length(w, pos + 1);
}

/* ===========================================================
   2. Count occurrences of a symbol (recursive)
   =========================================================== */
int count_occ(Word *w, int sym_idx, int pos) {
    if (pos == w->len) return 0;
    return (w->idx[pos] == sym_idx ? 1 : 0)
           + count_occ(w, sym_idx, pos + 1);
}

/* ===========================================================
   3. Concatenate — result = w1 followed by w2
   =========================================================== */
void concat_words(Word *w1, Word *w2, Word *result) {
    int i;
    result->len = 0;
    /* Copy all symbols from w1 */
    for (i = 0; i < w1->len; i++)
        result->idx[result->len++] = w1->idx[i];
    /* Append all symbols from w2 */
    for (i = 0; i < w2->len; i++)
        result->idx[result->len++] = w2->idx[i];
}

/* ===========================================================
   4. Power — repeat word w exactly n times
   =========================================================== */
void power_word(Word *w, int n, Word *result) {
    int i, r;
    result->len = 0;
    /* Repeat w exactly n times */
    for (r = 0; r < n; r++)
        for (i = 0; i < w->len; i++)
            result->idx[result->len++] = w->idx[i];
}

/* ===========================================================
   5. Equality — check if two words are identical (recursive)
   =========================================================== */
int equal_words(Word *w1, Word *w2, int pos) {
    /* Different lengths → not equal */
    if (w1->len != w2->len) return 0;
    if (pos == w1->len) return 1;   /* reached end, all matched */
    if (w1->idx[pos] != w2->idx[pos]) return 0;
    return equal_words(w1, w2, pos + 1);
}

/* ===========================================================
   6. Mirror (reverse) — reverse symbol order
   =========================================================== */
void mirror_word(Word *w, Word *result) {
    int i;
    result->len = w->len;
    /* Copy symbols in reverse order */
    for (i = 0; i < w->len; i++)
        result->idx[i] = w->idx[w->len - 1 - i];
}

/* ===========================================================
   7. Palindrome — word equals its mirror (recursive)
   =========================================================== */
int palindrome_word(Word *w, int left, int right) {
    if (left >= right) return 1;                    /* base case */
    if (w->idx[left] != w->idx[right]) return 0;   /* mismatch  */
    return palindrome_word(w, left + 1, right - 1);
}

/* ===========================================================
   8. Automaton — validate raw string against the alphabet
      Returns 1 (accepted) or 0 (rejected) without consuming
      the word into a struct, just prints the verdict.
   =========================================================== */
void automaton_check(Alphabet *al) {
    char raw[MAX];
    printf("  Raw string to validate: ");
    scanf("%s", raw);

    Word tmp;
    int accepted = parse_word(raw, al, &tmp);

    if (accepted)
        printf("  ACCEPTED: \"%s\" is a valid word over the alphabet.\n", raw);
    else
        printf("  REJECTED: \"%s\" cannot be generated by the alphabet.\n", raw);
}

/* ===========================================================
   MAIN MENU
   =========================================================== */
int main() {
    Alphabet al;
    al.size = 0;
    int ready = 0;   /* flag: alphabet has been created */
    int choice;

    do {
        printf("========================================\n");
        printf(" 1. Create alphabet\n");
        printf(" 2. Validate word (automaton)\n");
        printf("========================================\n");
        printf(" 3. Length\n");
        printf(" 4. Count occurrences of a symbol\n");
        printf(" 5. Concatenate two words\n");
        printf(" 6. Power (w^n)\n");
        printf(" 7. Equality\n");
        printf(" 8. Mirror (reverse)\n");
        printf(" 9. Palindrome check\n");
        printf("========================================\n");
        printf(" 0. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);

        /* Options 2-9 require an alphabet */
        if (choice >= 2 && choice <= 9 && !ready) {
            printf("  [!] Please create an alphabet first (option 1).\n");
            continue;
        }

        switch (choice) {

            /* ---- 1. Create alphabet ---- */
            case 1:
                create_alphabet(&al);
                ready = 1;
                break;

            /* ---- 2. Automaton validation ---- */
            case 2:
                automaton_check(&al);
                break;

            /* ---- 3. Length ---- */
            case 3: {
                Word w;
                printf("Enter the word:\n");
                read_word(&al, &w);
                print_word(&al, &w);
                printf("\n  Length = %d symbol(s)\n", word_length(&w, 0));
                break;
            }

            /* ---- 4. Count occurrences ---- */
            case 4: {
                Word w;
                char s[SYM_LEN];
                int i, si = -1;
                printf("Enter the word:\n");
                read_word(&al, &w);
                printf("  Symbol to count: ");
                scanf("%s", s);
                /* Find symbol index in alphabet */
                for (i = 0; i < al.size; i++)
                    if (str_eq(al.sym[i], s)) { si = i; break; }
                if (si == -1) {
                    printf("  [!] Symbol \"%s\" is not in the alphabet.\n", s);
                    break;
                }
                print_word(&al, &w);
                printf("\n  Occurrences of \"%s\" = %d\n",
                       s, count_occ(&w, si, 0));
                break;
            }

            /* ---- 5. Concatenate ---- */
            case 5: {
                Word w1, w2, result;
                printf("Enter word 1:\n");
                read_word(&al, &w1);
                printf("Enter word 2:\n");
                read_word(&al, &w2);
                concat_words(&w1, &w2, &result);
                printf("  w1 = "); print_word(&al, &w1); printf("\n");
                printf("  w2 = "); print_word(&al, &w2); printf("\n");
                printf("  w1.w2 = "); print_word(&al, &result); printf("\n");
                break;
            }

            /* ---- 6. Power ---- */
            case 6: {
                Word w, result;
                int n;
                printf("Enter the word:\n");
                read_word(&al, &w);
                printf("  Exponent n: ");
                scanf("%d", &n);
                if (n < 0) { printf("  [!] n must be >= 0.\n"); break; }
                power_word(&w, n, &result);
                printf("  w   = "); print_word(&al, &w); printf("\n");
                printf("  w^%d = ", n); print_word(&al, &result); printf("\n");
                break;
            }

            /* ---- 7. Equality ---- */
            case 7: {
                Word w1, w2;
                printf("Enter word 1:\n");
                read_word(&al, &w1);
                printf("Enter word 2:\n");
                read_word(&al, &w2);
                printf("  w1 = "); print_word(&al, &w1); printf("\n");
                printf("  w2 = "); print_word(&al, &w2); printf("\n");
                if (equal_words(&w1, &w2, 0))
                    printf("  Result: words are EQUAL\n");
                else
                    printf("  Result: words are NOT equal\n");
                break;
            }

            /* ---- 8. Mirror ---- */
            case 8: {
                Word w, mirror;
                printf("Enter the word:\n");
                read_word(&al, &w);
                mirror_word(&w, &mirror);
                printf("  w      = "); print_word(&al, &w);      printf("\n");
                printf("  mirror = "); print_word(&al, &mirror); printf("\n");
                break;
            }

            /* ---- 9. Palindrome ---- */
            case 9: {
                Word w;
                printf("Enter the word:\n");
                read_word(&al, &w);
                print_word(&al, &w);
                if (palindrome_word(&w, 0, w.len - 1))
                    printf("\n  Result: IS a palindrome\n");
                else
                    printf("\n  Result: is NOT a palindrome\n");
                break;
            }

            case 0:
                printf("  Goodbye!\n");
                break;

            default:
                printf("  [!] Invalid choice. Try again.\n");
        }

    } while (choice != 0);

    return 0;
}
