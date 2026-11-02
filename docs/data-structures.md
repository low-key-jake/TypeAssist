# TypeAssist — Core Data Structures & Algorithmic Defensibility

> **Academic Report & Viva Reference Guide**  
> **Subject**: Data Structures & Algorithms in C  
> **Project**: TypeAssist (Intelligent Writing & Communication Assistant)

---

## 1. Abstract

TypeAssist is architected around a non-trivial separation of concerns:
1. Keystroke-speed linguistic processing (autocomplete, error detection, candidate generation, ranking, and context-based next-word prediction) is executed exclusively by a native C engine compiled into WebAssembly.
2. Generative linguistic transformations (tone shifting, Hinglish translation, and comprehensive grammar surgery) are offloaded to asynchronous AI micro-routes.

This document details the 7 fundamental data structures and algorithms implemented in C, their theoretical time/space complexities, mathematical foundations, and concrete role within the application.

---

## 2. Comprehensive Data Structure Breakdown

```
                       ┌──────────────────────────────┐
                       │     TypeAssist C Engine      │
                       └──────────────┬───────────────┘
                                      │
         ┌────────────────────────────┼────────────────────────────┐
         │                            │                            │
   Lexical Indexing          Spelling Correction           Context & State
   ┌─────────────────┐       ┌─────────────────┐       ┌───────────────────────┐
   │ 1. Trie (Prefix)│       │ 3. Min-Heap     │       │ 5. N-Gram (Trigrams)  │
   │ 2. Hash Table   │       │ 4. Damerau-Lev. │       │ 6. Stack (Undo/Redo)  │
   └─────────────────┘       └─────────────────┘       │ 7. Linked List (Log)  │
                                                       └───────────────────────┘
```

---

### 1. Trie (Prefix Tree) — Autocomplete & Prefix Traversal

* **File**: `engine/src/trie.c`, `engine/include/trie.h`
* **Purpose**: Instantaneous prefix lookup and candidate retrieval for keystroke autocomplete.
* **Node Anatomy**:
  ```c
  typedef struct TrieNode {
      struct TrieNode *children[27]; // a-z + apostrophe (')
      bool is_end_of_word;
      int frequency;                 // Corpus frequency for candidate sorting
  } TrieNode;
  ```
* **Theoretical Complexity**:
  * **Insertion**: $\mathcal{O}(L)$, where $L$ is the length of the word.
  * **Search**: $\mathcal{O}(L)$.
  * **Prefix Autocomplete**: $\mathcal{O}(P + K \log K)$, where $P$ is prefix length and $K$ is the number of subtree candidate words ranked by frequency.
  * **Space Complexity**: $\mathcal{O}(|\Sigma| \cdot N)$, where $|\Sigma| = 27$ and $N$ is the total count of prefix nodes.
* **Defensibility**: Unlike binary search on a sorted string array ($\mathcal{O}(L \log N)$), the Trie guarantees predictable upper bounds independent of dictionary size $N$, making it strictly real-time and immune to scale latency.

---

### 2. Hash Table (Separate Chaining) — $O(1)$ Lexicon Verification

* **File**: `engine/src/hashtable.c`, `engine/include/hashtable.h`
* **Purpose**: Immediate dictionary existence checks, metadata storage, and N-gram frequency lookups.
* **Architecture**:
  * Array of pointers to singly-linked conflict buckets.
  * Hash Function: **Daniel J. Bernstein's DJB2** ($h = ((h \ll 5) + h) + c$).
  * Dynamic load-factor management: Rehashes and doubles capacity when $\frac{\text{size}}{\text{capacity}} \ge 0.75$.
* **Theoretical Complexity**:
  * **Insertion**: $\mathcal{O}(1)$ amortized.
  * **Lookup / Verification**: $\mathcal{O}(1)$ average case ($\mathcal{O}(N)$ catastrophic collision worst-case, mitigated by DJB2 uniform distribution).
  * **Deletion**: $\mathcal{O}(1)$ average case.
  * **Space Complexity**: $\mathcal{O}(N + M)$, where $N$ is word count and $M$ is table bucket capacity.
* **Defensibility**: Ensures dictionary membership tests during continuous typing do not incur Trie-traversal overhead when prefix tracking is unneeded.

---

### 3. Binary Min-Heap (Priority Queue) — Candidate Ranking

* **File**: `engine/src/minheap.c`, `engine/include/minheap.h`
* **Purpose**: Extracting the top-$K$ best spellcheck corrections from a candidate space.
* **Ordering Invariant**:
  $$\text{Parent.distance} \le \text{Child.distance}$$
  In the event of an edit distance tie, secondary ordering prioritizes words with higher natural corpus frequencies:
  $$\text{if } d_A = d_B \implies \text{Parent.frequency} \ge \text{Child.frequency}$$
* **Theoretical Complexity**:
  * **Heapify / Insert**: $\mathcal{O}(\log K)$.
  * **Extract-Min**: $\mathcal{O}(\log K)$.
  * **Peek**: $\mathcal{O}(1)$.
  * **Space Complexity**: $\mathcal{O}(K)$ contiguous dynamic array.
* **Defensibility**: Avoids sorting all generated candidates in $\mathcal{O}(M \log M)$ time; instead yields the top-$K$ suggestions in $\mathcal{O}(M \log K)$ using a bounded min-heap.

---

### 4. Damerau-Levenshtein Edit Distance — Typo Detection

* **File**: `engine/src/edit_distance.c`, `engine/include/edit_distance.h`
* **Purpose**: Quantifying string divergence considering four elementary human typographic errors:
  1. **Insertion** ($\epsilon \to c$)
  2. **Deletion** ($c \to \epsilon$)
  3. **Substitution** ($c_1 \to c_2$)
  4. **Adjacent Transposition** ($c_1 c_2 \to c_2 c_1$) — e.g., `recieve` $\to$ `receive`
* **Recurrence Relation**:
  $$D(i, j) = \min \begin{cases}
  D(i-1, j) + 1 & \text{Deletion} \\
  D(i, j-1) + 1 & \text{Insertion} \\
  D(i-1, j-1) + \text{cost} & \text{Substitution} \\
  D(i-2, j-2) + 1 & \text{Transposition (if } s_1[i]=s_2[j-1] \land s_1[i-1]=s_2[j]\text{)}
  \end{cases}$$
* **Theoretical Complexity**:
  * **Time Complexity**: $\mathcal{O}(|s_1| \cdot |s_2|)$.
  * **Space Complexity**: $\mathcal{O}(|s_1| \cdot |s_2|)$ with dynamic programming matrix.
* **Defensibility**: Standard Levenshtein counts adjacent swap as 2 operations (deletion + insertion); Damerau-Levenshtein accurately accounts for human slip-of-finger keystroke inversion as a single unit penalty.

---

### 5. N-Gram Language Model — Next-Word Statistical Predictor

* **File**: `engine/src/ngram.c`, `engine/include/ngram.h`
* **Purpose**: Context-aware word recommendation using Markov chain probability estimation.
* **Model Hierarchy & Backoff**:
  1. **Trigram Context**: Given $w_{i-2}, w_{i-1}$, predict $w_i$:
     $$P(w_i \mid w_{i-2}, w_{i-1}) = \frac{\text{Count}(w_{i-2}, w_{i-1}, w_i)}{\text{Count}(w_{i-2}, w_{i-1})}$$
  2. **Bigram Fallback**: If trigram count is zero, back off to:
     $$P(w_i \mid w_{i-1}) = \frac{\text{Count}(w_{i-1}, w_i)}{\text{Count}(w_{i-1})}$$
  3. **Unigram Fallback**: If bigram count is zero, recommend highest absolute frequency lexical unigrams:
     $$P(w_i) = \frac{\text{Count}(w_i)}{\sum_w \text{Count}(w)}$$
* **Theoretical Complexity**:
  * **Lookup**: $\mathcal{O}(1)$ via internal Hash Table keys (`w1|w2|w3`).
  * **Prediction Sorting**: $\mathcal{O}(C \log C)$ over matching prefix candidates.
* **Defensibility**: Gives the browser an instantaneous, deterministic statistical language predictor without requiring high-latency neural network inference for basic typing completions.

---

### 6. Dynamic Stack — Undo/Redo Text State Management

* **File**: `engine/src/stack.c`, `engine/include/stack.h`
* **Purpose**: Snapshotting text states to allow atomic undo of accepted suggestions, corrections, or transformations.
* **Implementation Details**:
  * Dynamic array of `StackEntry` structs containing text buffers and integer timestamps.
  * Doubles capacity when full, avoiding fragmentation.
* **Theoretical Complexity**:
  * **Push**: $\mathcal{O}(1)$ amortized.
  * **Pop**: $\mathcal{O}(1)$.
  * **Peek**: $\mathcal{O}(1)$.
  * **Space Complexity**: $\mathcal{O}(S)$ snapshots.

---

### 7. Singly Linked List — Audit Log & Correction History

* **File**: `engine/src/linkedlist.c`, `engine/include/linkedlist.h`
* **Purpose**: Maintaining a chronological timeline of user actions, transformed phrases, and autocorrect events.
* **Implementation Details**:
  * Head insertion allows instant $\mathcal{O}(1)$ additions of newest events.
  * Bounded memory with automated tail eviction when `count >= max_entries`.
  * Serializes directly to a JSON array string for consumption by the Next.js UI.
* **Theoretical Complexity**:
  * **Insert (Head)**: $\mathcal{O}(1)$.
  * **Traversal / JSON Serialization**: $\mathcal{O}(N)$.
  * **Space Complexity**: $\mathcal{O}(N)$ dynamically allocated nodes.

---

## 3. Summary Complexity Matrix

| Data Structure | Primary Operation | Average Time | Worst Time | Space |
|---|---|---|---|---|
| **Trie** | Prefix Autocomplete | $\mathcal{O}(L + K \log K)$ | $\mathcal{O}(L + N \log N)$ | $\mathcal{O}(\Sigma \cdot N)$ |
| **Hash Table** | Word Lookup | $\mathcal{O}(1)$ | $\mathcal{O}(N)$ | $\mathcal{O}(N + M)$ |
| **Min-Heap** | Candidate Priority Extraction | $\mathcal{O}(\log K)$ | $\mathcal{O}(\log K)$ | $\mathcal{O}(K)$ |
| **Damerau-Levenshtein** | Edit Distance DP | $\mathcal{O}(|s_1| \cdot |s_2|)$ | $\mathcal{O}(|s_1| \cdot |s_2|)$ | $\mathcal{O}(|s_1| \cdot |s_2|)$ |
| **N-Gram Model** | Next-Word Inference | $\mathcal{O}(C \log C)$ | $\mathcal{O}(V \log V)$ | $\mathcal{O}(V + B + T)$ |
| **Stack** | Undo Pop/Push | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | $\mathcal{O}(S)$ |
| **Linked List** | History Log Insert | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | $\mathcal{O}(N)$ |

