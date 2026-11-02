# TypeAssist — College Project Demonstration & Viva Script

> **Course**: Data Structures & Algorithms in C  
> **Project**: TypeAssist (Intelligent Writing & Communication Assistant)

---

## 1. Quick Launch

```bash
cd typeassist
npm install
npm run dev
```
Navigate to `http://localhost:3000`.

---

## 2. 5-Minute Demonstration Walkthrough

### Step 1: Keystroke Autocomplete (Trie)
1. In the **Workspace** text editor, start typing `def` or `rec`.
2. Point out the top ribbon showing `Trie: [receive, receipt, definitely...]`.
3. **Viva Point**: Explain that prefix matching is handled by traversing down the Trie nodes in $\mathcal{O}(L)$ time where $L$ is prefix length, completely independent of how many words exist in the dictionary.

### Step 2: Typo Detection & Ranking (Damerau-Levenshtein + Min-Heap)
1. Type intentional typos such as:
   - `recieve` (transposition typo)
   - `definately` (substitution typo)
   - `necesary` (deletion typo)
2. Notice the **Detected Spelling Queries** card instantly flags the typos.
3. Click the suggestion `receive` (distance: 1).
4. **Viva Point**: Explain that standard Levenshtein counts `recieve` $\to$ `receive` as distance 2 (delete + insert), whereas our C implementation of **Damerau-Levenshtein** recognizes adjacent swaps as distance 1. Candidate ranking uses a **Min-Heap** where edit distance is primary and corpus unigram frequency is secondary.

### Step 3: Next-Word Prediction (N-Gram Model)
1. Type `i want to` or `how are`.
2. Notice the purple **Next-Word (N-Gram)** prediction pills appear (e.g., `know`, `be`, `go`).
3. Click any pill to automatically append it.
4. **Viva Point**: Explain that the engine queries a Trigram Hash Table first ($P(w_3 \mid w_1, w_2)$); if no trigram matches, it falls back to Bigrams ($P(w_2 \mid w_1)$), and then Unigrams.

### Step 4: Undo & History (Stack & Linked List)
1. Click the **Undo** button in the editor.
2. Note that the previous text state is restored immediately via the C **Stack** LIFO pop.
3. Open the **History & Audit** tab in the sidebar.
4. Point out the chronological log of all corrections and autocompletes, powered by a singly **Linked List** with $\mathcal{O}(1)$ head insertion.

### Step 5: Tone Shifter & Hinglish Translator
1. Navigate to **Tone Shifter**. Enter: `send me the report right now`.
2. Select **Gen Z** or **Diplomatic** and click **Shift Tone**.
3. Navigate to **Translator**. Select **Hinglish to English**, enter `main kal aunga`, and click **Translate**.

---

## 3. Anticipated Viva Questions & Defenses

| Viva Question | Concise Academic Answer |
|---|---|
| *Why compile C to WebAssembly instead of writing pure JavaScript?* | WebAssembly executes raw compiled C with near-native CPU efficiency and manual memory management (`malloc`/`free`), avoiding garbage collector pauses during continuous keystroke processing. It also fulfills the academic requirement of genuine C data structure engineering. |
| *How does your Hash Table handle collisions?* | Separate chaining with linked lists on each bucket. When the load factor exceeds 0.75, the table dynamically doubles its capacity and rehashes all elements using the DJB2 hashing function. |
| *Why use a Min-Heap rather than sorting candidate words?* | When generating correction candidates from edit distance permutations, inserting into a bounded min-heap of size $K$ takes $\mathcal{O}(M \log K)$, compared to $\mathcal{O}(M \log M)$ to sort the entire array. |
| *What happens if the user types offline?* | All 7 core algorithms (Trie, Hash Table, Heap, Edit Distance, N-Grams, Stack, Linked List) and pre-indexed datasets execute locally in the browser with zero network reliance. |

