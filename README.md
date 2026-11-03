# TypeAssist — Intelligent Writing & Communication Assistant

> **"Think it. Type it. Transform it."**  
> *A high-performance writing assistant powered by a native C text-processing core compiled to WebAssembly.*

[![C Engine](https://img.shields.io/badge/C-ANSI_C99-blue.svg)](engine/)
[![WebAssembly](https://img.shields.io/badge/WebAssembly-Emscripten-purple.svg)](public/wasm/)
[![Next.js](https://img.shields.io/badge/Next.js-16_App_Router-black.svg)](https://nextjs.org/)
[![TypeScript](https://img.shields.io/badge/TypeScript-5.0-blue.svg)](https://www.typescriptlang.org/)
[![Tailwind CSS](https://img.shields.io/badge/Tailwind-v4-cyan.svg)](https://tailwindcss.com/)

---

## 1. Project Overview

TypeAssist is designed for the academic course **Data Structures in C**. Unlike traditional writing tools that rely exclusively on cloud APIs or simulate algorithms in JavaScript, TypeAssist features a **genuine native C engine** that executes all lexical indexing, typo calculation, priority ranking, and statistical n-gram predictions directly in WebAssembly.

Generative capabilities (tone shifting across Gen Z, Shakespearean, Diplomatic, and Hinglish translation) are exposed via asynchronous server-side routes, ensuring that **not a single keystroke is transmitted over the network for basic typing operations**.

---

## 2. Core Data Structures Implemented in C

All data structures are implemented from scratch in `engine/src/`:

| Data Structure | Source File | Header File | Application Function | Complexity |
|---|---|---|---|---|
| **Trie (Prefix Tree)** | `engine/src/trie.c` | `engine/include/trie.h` | Keystroke Autocomplete & Prefix Traversal | $\mathcal{O}(L)$ Insert/Search |
| **Hash Table** | `engine/src/hashtable.c` | `engine/include/hashtable.h` | Dictionary Lookups & $\mathcal{O}(1)$ Existence Verification | $\mathcal{O}(1)$ Amortized |
| **Binary Min-Heap** | `engine/src/minheap.c` | `engine/include/minheap.h` | Typo Suggestion Ranking by Distance & Frequency | $\mathcal{O}(\log K)$ Extraction |
| **Damerau-Levenshtein** | `engine/src/edit_distance.c` | `engine/include/edit_distance.h` | Typo Detection (Insert, Delete, Sub, Swap) | $\mathcal{O}(|s_1| \cdot |s_2|)$ DP |
| **N-Gram Model** | `engine/src/ngram.c` | `engine/include/ngram.h` | Trigram / Bigram Next-Word Statistical Prediction | $\mathcal{O}(1)$ Hash Lookup |
| **Dynamic Stack** | `engine/src/stack.c` | `engine/include/stack.h` | Multi-Level Text State Undo/Redo | $\mathcal{O}(1)$ Push/Pop |
| **Singly Linked List** | `engine/src/linkedlist.c` | `engine/include/linkedlist.h` | Chronological Session History & Audit Trail | $\mathcal{O}(1)$ Head Insertion |

---

## 3. Directory Layout

```
typeassist/
├── engine/                     # Native C Text-Processing Core
│   ├── include/                # Public C Headers (trie.h, hashtable.h, etc.)
│   ├── src/                    # Complete C Implementations
│   ├── tests/                  # Unit Test Suites & Benchmarks
│   ├── Makefile                # Native & WebAssembly Build Rules
│   └── README.md               # Algorithmic & Complexity Documentation
├── data/                       # Linguistic Corpora & Preparation Scripts
│   ├── scripts/                # prepare_data.js (Tokenization & N-Grams)
│   ├── processed/              # dictionary.txt, bigrams.txt, trigrams.txt
│   └── README.md               # Dataset Sources, Licences, & Attribution
├── public/                     # Static Web Assets & Pre-Indexed Lexicons
│   ├── data/                   # Runtime Text Files Read by Engine
│   └── wasm/                   # Compiled WebAssembly Output
├── src/                        # Next.js Application
│   ├── app/                    # App Router Pages & API Routes
│   │   ├── api/                # Tone Shifter, Translator, Sentence Doctor
│   │   ├── tone-shifter/       # Persona Shifting Studio
│   │   ├── translator/         # Hinglish / Hindi / English Translation
│   │   ├── sentence-doctor/    # Grammatical Repair & Rule Breakdown
│   │   ├── smart-reply/        # Contextual Reply Generation
│   │   ├── dictionary/         # Personal Lexicon Manager
│   │   ├── history/            # Linked List Session Audit Trail
│   │   └── settings/           # Telemetry & Viva Reference
│   ├── components/             # Reusable UI & Layout Components
│   ├── hooks/                  # useEngine React Hook
│   └── lib/wasm/               # TypeScript WebAssembly Bridge
└── docs/                       # Project Documentation
    ├── data-structures.md       # Detailed Viva Reference & Mathematical Proofs
    ├── architecture.md         # Full System Architecture Breakdown
    └── demo-guide.md           # 5-Minute Demonstration Script
```

---

## 4. Getting Started

### Prerequisites
- **Node.js** v18+ (tested on v24.20.0)
- **npm** (comes with Node.js)
- *(Optional)* **Emscripten SDK** (only needed if you modify C code and want to recompile WebAssembly; precompiled `.wasm` binaries are already included in `public/wasm/`)

### Installation & Launch
```bash
# 1. Clone the repository
git clone https://github.com/low-key-jake/TypeAssist.git
cd TypeAssist

# 2. Install dependencies
npm install

# 3. (Optional) Set up AI API Key for Tone Shifter / Sentence Doctor
# Copy .env.example or set in .env.local:
# GEMINI_API_KEY=your_key_here

# 4. Start the application
npm run dev
```

Open [http://localhost:3001](http://localhost:3001) in your browser.

> **Note on WebAssembly:** The repository includes precompiled `public/wasm/typeassist.js` and `public/wasm/typeassist.wasm`. If WebAssembly is loaded, the engine badge shows `C Engine (WebAssembly)`. If WASM is unavailable in a browser environment, TypeAssist automatically and gracefully switches to `TypeScript Fallback`.

---

## 5. Compiling C Engine (Native & WebAssembly)

### Recompiling to WebAssembly (Emscripten)
If you modify the C engine in `engine/src/`:
1. Install and activate [emsdk](https://github.com/emscripten-core/emsdk):
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   # On Windows: emsdk_env.bat
   # On Linux/macOS: source ./emsdk_env.sh
   ```
2. Run the build command:
   ```bash
   npm run build:wasm
   # or on Windows:
   engine\build_wasm.bat
   # or on Linux/macOS:
   cd engine && make wasm
   ```
This updates `public/wasm/typeassist.js` and `public/wasm/typeassist.wasm`.

### Running Tests & Benchmarks
```bash
# Test WASM compilation and all 7 C data structures in Node:
node engine/tests/test_wasm.js
node engine/tests/benchmark_wasm.js

# Or compile and run native C unit tests (with gcc/clang):
cd engine
make test
make benchmark
```
```bash
cd engine
make test
make benchmark
```

### Compiling to WebAssembly (with Emscripten)
```bash
cd engine
make wasm
```
This generates `typeassist.js` and `typeassist.wasm` inside `public/wasm/`.

---

## 6. Academic Defensibility

Full time/space complexity proofs, mathematical recurrences, and viva preparation Q&A are catalogued in [docs/data-structures.md](docs/data-structures.md).

