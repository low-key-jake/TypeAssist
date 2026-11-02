# TypeAssist — System Architecture & WebAssembly Bridge

```
┌────────────────────────────────────────────────────────────────────────┐
│                          NEXT.JS FRONTEND                              │
│                                                                        │
│   [Workspace]    [Tone Shifter]   [Translator]   [Sentence Doctor]     │
│        │                │               │                │             │
│        ▼                ▼               ▼                ▼             │
│   useEngine()      /api/tone-shift   /api/translate   /api/grammar     │
└────────┬──────────────────────┬───────────────┬────────────────────────┘
         │                      │               │
         ▼ (Keystroke Speed)    ▼ (Async Cloud) ▼ (Async Cloud)
┌──────────────────┐   ┌─────────────────────────────────────────────────┐
│ C ENGINE (WASM)  │   │              TYPEASSIST STUDIO (AI)             │
│                  │   │                                                 │
│ • Trie           │   │ • Google Gemini Generative API                  │
│ • Hash Table     │   │ • Indian English / Hinglish Language Model      │
│ • Min-Heap       │   │ • Style & Persona Transfer Engine               │
│ • Damerau-Lev.   │   │ • Rule-based Offline Fallback Synthesizer       │
│ • N-Gram Model   │   └─────────────────────────────────────────────────┘
│ • Dynamic Stack  │
│ • Linked List    │
└──────────────────┘
```

## 1. WebAssembly Integration Mechanics

The C engine is compiled with Emscripten into a standalone binary (`typeassist.wasm`) accompanied by an asynchronous JavaScript initialization wrapper (`typeassist.js`).

### Memory Safety & Allocation
- The browser allocates an initial linear memory segment of 32 MB with dynamic memory growth enabled (`-s ALLOW_MEMORY_GROWTH=1`).
- All string outputs from the C engine (`autocomplete`, `spellcheck`, `predict_next`) are formatted into JSON buffers allocated on the WASM heap using `malloc()`.
- The TypeScript wrapper reads the UTF-8 bytes via Emscripten's `UTF8ToString()` and calls `engine_free_string()` immediately after parsing, ensuring zero memory leaks across thousands of continuous typing strokes.

### Separation of Concerns
Keystrokes are handled exclusively client-side via the compiled C WebAssembly engine with sub-millisecond execution times. No keystrokes or raw drafts are continuously transmitted over the network. Only explicit user invocations of generative actions (like "Shift Tone" or "Diagnose Sentence") query server-side micro-routes.

