# TypeAssist C Engine Architecture & Data Structures

The **TypeAssist Core** is a high-performance text-processing engine written entirely in ANSI C, designed to compile both as a standalone native library and to WebAssembly via Emscripten.

All 7 core data structures are genuinely implemented from scratch with zero third-party C library dependencies.

---

## 1. Core Data Structures & Complexity Analysis

### A. Trie (Prefix Tree) — `src/trie.c`, `include/trie.h`
- **Purpose**: Fast prefix search, dictionary indexing, and frequency-ranked autocomplete.
- **Node Structure**: 27 branches (letters `a-z` + apostrophe `'`), end-of-word flag, and word frequency score.
- **Time Complexity**:
  - Insert: $\mathcal{O}(L)$ where $L$ is word length.
  - Search: $\mathcal{O}(L)$.
  - Prefix Autocomplete: $\mathcal{O}(L + K \log K)$ where $K$ is candidate count (bounded by DFS pruning).
- **Space Complexity**: $\mathcal{O}(\Sigma \cdot N)$ where $\Sigma = 27$ and $N$ is total node count.

### B. Hash Table — `src/hashtable.c`, `include/hashtable.h`
- **Purpose**: $\mathcal{O}(1)$ dictionary existence checks, metadata lookup, and N-gram storage.
- **Implementation**: Separate chaining with dynamically allocated linked lists per bucket.
- **Collision Resolution**: DJB2 hash algorithm with automated dynamic resizing at load factor $> 0.75$.
- **Time Complexity**:
  - Insert / Lookup / Delete: $\mathcal{O}(1)$ amortized ($\mathcal{O}(N)$ worst case before rehash).
- **Space Complexity**: $\mathcal{O}(N + M)$ where $N$ is item count and $M$ is bucket capacity.

### C. Binary Min-Heap — `src/minheap.c`, `include/minheap.h`
- **Purpose**: Candidate ranking for spelling correction.
- **Ordering Property**: Min-heap ordered by edit distance (smaller distance has higher priority). Ties are broken by unigram word frequency (higher frequency prioritized).
- **Time Complexity**:
  - Insert: $\mathcal{O}(\log k)$ where $k$ is candidate capacity.
  - Extract-Min: $\mathcal{O}(\log k)$.
  - Peek: $\mathcal{O}(1)$.
- **Space Complexity**: $\mathcal{O}(k)$ dynamic array.

### D. Damerau-Levenshtein Edit Distance — `src/edit_distance.c`, `include/edit_distance.h`
- **Purpose**: Typo detection and spelling candidate distance calculation.
- **Supported Operations**: Insertion, Deletion, Substitution, and Adjacent Transposition (e.g., `recieve` $\to$ `receive`).
- **Time Complexity**: $\mathcal{O}(|s_1| \cdot |s_2|)$.
- **Space Complexity**: $\mathcal{O}(|s_1| \cdot |s_2|)$ with dynamic 2D programming matrix.

### E. N-Gram Language Model — `src/ngram.c`, `include/ngram.h`
- **Purpose**: Contextual next-word prediction based on Markovian statistical language modeling.
- **Architecture**: Tri-level hierarchy (Trigram $\to$ Bigram $\to$ Unigram fallback).
  - Trigram: $P(w_3 \mid w_1, w_2) = \frac{\text{Count}(w_1, w_2, w_3)}{\text{Count}(w_1, w_2)}$
  - Bigram: $P(w_2 \mid w_1) = \frac{\text{Count}(w_1, w_2)}{\text{Count}(w_1)}$
  - Unigram: $P(w) = \frac{\text{Count}(w)}{N}$
- **Time Complexity**:
  - Training / Load: $\mathcal{O}(1)$ per n-gram insertion into hash table.
  - Prediction: $\mathcal{O}(C)$ where $C$ is the number of matching bigrams/trigrams.
- **Space Complexity**: $\mathcal{O}(V + B + T)$ hash table entries.

### F. Stack — `src/stack.c`, `include/stack.h`
- **Purpose**: Multi-level undo/redo management of user text modifications.
- **Implementation**: Dynamic LIFO array with automated capacity doubling.
- **Time Complexity**: Push $\mathcal{O}(1)$, Pop $\mathcal{O}(1)$, Peek $\mathcal{O}(1)$.
- **Space Complexity**: $\mathcal{O}(S)$ snapshots.

### G. Singly Linked List — `src/linkedlist.c`, `include/linkedlist.h`
- **Purpose**: Chronological history log of corrections, transformations, and suggestions.
- **Implementation**: Singly linked list with head insertion for $\mathcal{O}(1)$ logging and FIFO eviction when `max_entries` threshold is reached.
- **Time Complexity**: Insert $\mathcal{O}(1)$, Evict tail $\mathcal{O}(N)$ (or $\mathcal{O}(1)$ with tail pointer), Traversal $\mathcal{O}(N)$.
- **Space Complexity**: $\mathcal{O}(N)$ nodes.

---

## 2. WebAssembly Bridge (`engine_api.c`)

The WebAssembly interface acts as the single boundary between TypeScript/JavaScript and the C runtime:
- Strings passed across the boundary are encoded in UTF-8.
- Output payloads are serialized as compact JSON strings in C heap memory.
- `engine_free_string()` ensures deterministic memory reclamation with zero memory leaks.

