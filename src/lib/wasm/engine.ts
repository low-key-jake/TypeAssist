/**
 * TypeAssist — C/WebAssembly Engine Bridge & Fallback Runtime
 *
 * Provides a production-grade WebAssembly bridge connecting Next.js to the
 * native C data structures (Trie, HashTable, Min-Heap, Damerau-Levenshtein,
 * N-Gram Model, Stack, Linked List).
 *
 * If WebAssembly fails to load, gracefully falls back to the verified TypeScript
 * implementation without interrupting user workflows.
 */

import {
  TrieSuggestion,
  SpellCheckResult,
  NGramPrediction,
  HistoryType,
  HistoryEntry,
  EngineStats,
  TypeAssistEngineInterface,
} from './types';

// ============================================================================
// TypeScript Fallback Algorithms (Faithful algorithmic twins to engine/src/*.c)
// ============================================================================

function damerauLevenshtein(s1: string, s2: string): number {
  const m = s1.length;
  const n = s2.length;
  if (m === 0) return n;
  if (n === 0) return m;

  const d: number[][] = Array.from({ length: m + 1 }, () => new Array(n + 1).fill(0));

  for (let i = 0; i <= m; i++) d[i][0] = i;
  for (let j = 0; j <= n; j++) d[0][j] = j;

  for (let i = 1; i <= m; i++) {
    for (let j = 1; j <= n; j++) {
      const cost = s1[i - 1].toLowerCase() === s2[j - 1].toLowerCase() ? 0 : 1;
      d[i][j] = Math.min(
        d[i - 1][j] + 1,       // Deletion
        d[i][j - 1] + 1,       // Insertion
        d[i - 1][j - 1] + cost // Substitution
      );

      // Adjacent Transposition (Damerau addition)
      if (
        i > 1 &&
        j > 1 &&
        s1[i - 1].toLowerCase() === s2[j - 2].toLowerCase() &&
        s1[i - 2].toLowerCase() === s2[j - 1].toLowerCase()
      ) {
        d[i][j] = Math.min(d[i][j], d[i - 2][j - 2] + 1);
      }
    }
  }

  return d[m][n];
}

interface HeapElement {
  word: string;
  distance: number;
  frequency: number;
}

class MinHeap {
  private data: HeapElement[] = [];

  insert(word: string, distance: number, frequency: number): void {
    const elem: HeapElement = { word, distance, frequency };
    this.data.push(elem);
    this.siftUp(this.data.length - 1);
  }

  extractMin(): HeapElement | null {
    if (this.data.length === 0) return null;
    const min = this.data[0];
    const last = this.data.pop()!;
    if (this.data.length > 0) {
      this.data[0] = last;
      this.siftDown(0);
    }
    return min;
  }

  isEmpty(): boolean {
    return this.data.length === 0;
  }

  private compare(a: HeapElement, b: HeapElement): number {
    if (a.distance !== b.distance) return a.distance - b.distance;
    return b.frequency - a.frequency; // higher freq wins ties
  }

  private siftUp(idx: number): void {
    while (idx > 0) {
      const parent = Math.floor((idx - 1) / 2);
      if (this.compare(this.data[idx], this.data[parent]) < 0) {
        [this.data[idx], this.data[parent]] = [this.data[parent], this.data[idx]];
        idx = parent;
      } else break;
    }
  }

  private siftDown(idx: number): void {
    const length = this.data.length;
    while (true) {
      let smallest = idx;
      const left = 2 * idx + 1;
      const right = 2 * idx + 2;

      if (left < length && this.compare(this.data[left], this.data[smallest]) < 0) {
        smallest = left;
      }
      if (right < length && this.compare(this.data[right], this.data[smallest]) < 0) {
        smallest = right;
      }

      if (smallest !== idx) {
        [this.data[idx], this.data[smallest]] = [this.data[smallest], this.data[idx]];
        idx = smallest;
      } else break;
    }
  }
}

interface TrieNode {
  children: Map<string, TrieNode>;
  isEndOfWord: boolean;
  frequency: number;
}

class Trie {
  private root: TrieNode = { children: new Map(), isEndOfWord: false, frequency: 0 };
  public nodeCount: number = 1;

  insert(word: string, frequency: number = 1): void {
    let curr = this.root;
    for (const char of word.toLowerCase()) {
      if (!curr.children.has(char)) {
        curr.children.set(char, { children: new Map(), isEndOfWord: false, frequency: 0 });
        this.nodeCount++;
      }
      curr = curr.children.get(char)!;
    }
    curr.isEndOfWord = true;
    curr.frequency = Math.max(curr.frequency, frequency);
  }

  autocomplete(prefix: string, maxResults: number = 5): TrieSuggestion[] {
    let curr = this.root;
    for (const char of prefix.toLowerCase()) {
      if (!curr.children.has(char)) return [];
      curr = curr.children.get(char)!;
    }

    const results: TrieSuggestion[] = [];
    this.collectWords(curr, prefix.toLowerCase(), results);
    return results.sort((a, b) => b.frequency - a.frequency).slice(0, maxResults);
  }

  private collectWords(node: TrieNode, currentWord: string, results: TrieSuggestion[]): void {
    if (node.isEndOfWord) {
      results.push({ word: currentWord, frequency: node.frequency });
    }
    for (const [char, child] of node.children.entries()) {
      this.collectWords(child, currentWord + char, results);
    }
  }
}

class HashTable {
  private map: Map<string, number> = new Map();

  insert(key: string, value: number): void {
    this.map.set(key.toLowerCase(), value);
  }

  contains(key: string): boolean {
    return this.map.has(key.toLowerCase());
  }

  get(key: string): number | undefined {
    return this.map.get(key.toLowerCase());
  }

  remove(key: string): boolean {
    return this.map.delete(key.toLowerCase());
  }

  size(): number {
    return this.map.size;
  }

  keys(): IterableIterator<string> {
    return this.map.keys();
  }
}

class UndoStack {
  private items: { text: string; timestamp: number }[] = [];
  private readonly maxDepth: number = 50;

  push(text: string): void {
    this.items.push({ text, timestamp: Date.now() });
    if (this.items.length > this.maxDepth) {
      this.items.shift();
    }
  }

  pop(): { text: string; timestamp: number } | null {
    return this.items.pop() || null;
  }

  size(): number {
    return this.items.length;
  }
}

class HistoryLinkedList {
  private entries: HistoryEntry[] = [];
  private readonly maxEntries: number = 200;

  add(original: string, modified: string, type: HistoryType): void {
    this.entries.unshift({
      original,
      modified,
      type,
      timestamp: Date.now(),
    });
    if (this.entries.length > this.maxEntries) {
      this.entries.pop();
    }
  }

  toArray(): HistoryEntry[] {
    return [...this.entries];
  }

  clear(): void {
    this.entries = [];
  }

  size(): number {
    return this.entries.length;
  }
}

// ============================================================================
// TypeAssist Unified Engine Bridge
// ============================================================================

export class TypeAssistEngine implements TypeAssistEngineInterface {
  // WebAssembly state
  // eslint-disable-next-line @typescript-eslint/no-explicit-any
  private wasmModule: any = null;
  private isWasmLoaded: boolean = false;
  private activeEngine: 'wasm' | 'fallback' = 'fallback';
  private initPromise: Promise<boolean> | null = null;
  private initialized: boolean = false;

  // TypeScript Fallback state
  private trie: Trie = new Trie();
  private dictHt: HashTable = new HashTable();
  private personalTrie: Trie = new Trie();
  private personalHt: HashTable = new HashTable();
  private unigrams: HashTable = new HashTable();
  private bigrams: HashTable = new HashTable();
  private trigrams: HashTable = new HashTable();
  private undoStack: UndoStack = new UndoStack();
  private historyList: HistoryLinkedList = new HistoryLinkedList();

  // Raw Corpora Cache
  private rawDictContent: string = '';
  private rawBigramContent: string = '';
  private rawTrigramContent: string = '';

  async init(): Promise<boolean> {
    if (this.initialized) return true;
    if (this.initPromise) return this.initPromise;

    this.initPromise = (async () => {
      try {
        // 1. Fetch text data files
        await this.loadCorpora();

        // 2. Try loading and instantiating WebAssembly
        const wasmSuccess = await this.tryLoadWasm();
        if (wasmSuccess) {
          this.isWasmLoaded = true;
          this.activeEngine = 'wasm';
          console.info('[TypeAssist] WebAssembly C Engine initialized successfully (7 Data Structures Active).');
        } else {
          this.isWasmLoaded = false;
          this.activeEngine = 'fallback';
          console.warn('[TypeAssist] Running in TypeScript Fallback mode.');
        }

        this.initialized = true;
        return true;
      } catch (err) {
        console.error('[TypeAssist] Engine initialization error:', err);
        this.seedFallbackCorpus();
        this.isWasmLoaded = false;
        this.activeEngine = 'fallback';
        this.initialized = true;
        return true;
      }
    })();

    return this.initPromise;
  }

  private async tryLoadWasm(): Promise<boolean> {
    if (typeof window === 'undefined') {
      // In Node environment (e.g. tests or SSR)
      try {
        // eslint-disable-next-line @typescript-eslint/no-require-imports
        const createModule = require('../../../public/wasm/typeassist.js');
        if (typeof createModule === 'function') {
          this.wasmModule = await createModule();
          return this.initWasmEngineState();
        }
      } catch {
        return false;
      }
      return false;
    }

    // In Browser environment
    try {
      // Check if script already injected
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      if (!(window as any).createTypeAssistModule) {
        await new Promise<void>((resolve, reject) => {
          const script = document.createElement('script');
          script.src = '/wasm/typeassist.js';
          script.async = true;
          script.onload = () => resolve();
          script.onerror = (e) => reject(e);
          document.head.appendChild(script);
        });
      }

      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      const createModule = (window as any).createTypeAssistModule;
      if (typeof createModule === 'function') {
        this.wasmModule = await createModule({
          locateFile: (path: string) => `/wasm/${path}`,
        });
        return this.initWasmEngineState();
      }
    } catch (e) {
      console.warn('[TypeAssist] Failed to initialize WebAssembly module:', e);
      return false;
    }

    return false;
  }

  private initWasmEngineState(): boolean {
    if (!this.wasmModule || typeof this.wasmModule._engine_init !== 'function') {
      return false;
    }

    const status = this.wasmModule._engine_init();
    if (status !== 0) {
      console.error('[TypeAssist WASM] _engine_init returned non-zero code:', status);
      return false;
    }

    // Bulk load dictionary into C Engine
    if (this.rawDictContent) {
      this.withWasmString(this.rawDictContent, (ptr, len) => {
        const loaded = this.wasmModule._engine_load_dictionary(ptr, len);
        console.info(`[TypeAssist WASM] Loaded ${loaded} words into C Trie & HashTable.`);
      });
    }

    // Bulk load N-grams into C Engine
    if (this.rawBigramContent) {
      this.withWasmString(this.rawBigramContent, (ptr, len) => {
        this.wasmModule._engine_load_ngrams(ptr, len, 2);
      });
    }

    if (this.rawTrigramContent) {
      this.withWasmString(this.rawTrigramContent, (ptr, len) => {
        this.wasmModule._engine_load_ngrams(ptr, len, 3);
      });
    }

    return true;
  }

  /* ─── WASM String Memory Helpers ─── */

  private withWasmString<T>(str: string, fn: (ptr: number, len: number) => T): T {
    if (!this.wasmModule) throw new Error('WASM module not loaded');
    const len = this.wasmModule.lengthBytesUTF8(str) + 1;
    const ptr = this.wasmModule._malloc(len);
    this.wasmModule.stringToUTF8(str, ptr, len);
    try {
      return fn(ptr, len);
    } finally {
      this.wasmModule._free(ptr);
    }
  }

  private readAndFreeJson<T>(ptr: number, defaultValue: T): T {
    if (!ptr || !this.wasmModule) return defaultValue;
    try {
      const str = this.wasmModule.UTF8ToString(ptr);
      if (!str) return defaultValue;
      return JSON.parse(str) as T;
    } catch (err) {
      console.error('[TypeAssist WASM] Failed to parse JSON from C Engine:', err);
      return defaultValue;
    } finally {
      this.wasmModule._engine_free_string(ptr);
    }
  }

  /* ─── Corpora Loading ─── */

  private async loadCorpora(): Promise<void> {
    try {
      const [dictRes, biRes, triRes] = await Promise.all([
        fetch('/data/dictionary.txt').then((r) => r.text()).catch(() => ''),
        fetch('/data/bigrams.txt').then((r) => r.text()).catch(() => ''),
        fetch('/data/trigrams.txt').then((r) => r.text()).catch(() => ''),
      ]);

      this.rawDictContent = dictRes || '';
      this.rawBigramContent = biRes || '';
      this.rawTrigramContent = triRes || '';

      // Also parse into fallback structures for seamless redundancy
      if (this.rawDictContent) this.parseDictionaryFallback(this.rawDictContent);
      if (this.rawBigramContent) this.parseBigramsFallback(this.rawBigramContent);
      if (this.rawTrigramContent) this.parseTrigramsFallback(this.rawTrigramContent);

      if (this.dictHt.size() === 0) {
        this.seedFallbackCorpus();
      }
    } catch {
      this.seedFallbackCorpus();
    }
  }

  private parseDictionaryFallback(content: string): void {
    const lines = content.split('\n');
    for (const line of lines) {
      const parts = line.trim().split(/\s+/);
      if (parts.length >= 1 && parts[0]) {
        const word = parts[0].toLowerCase();
        const freq = parts.length > 1 ? parseInt(parts[1], 10) || 1 : 1;
        this.trie.insert(word, freq);
        this.dictHt.insert(word, freq);
        this.unigrams.insert(word, freq);
      }
    }
  }

  private parseBigramsFallback(content: string): void {
    const lines = content.split('\n');
    for (const line of lines) {
      const parts = line.trim().split(/\s+/);
      if (parts.length >= 2) {
        const freq = parseInt(parts[1], 10) || 1;
        this.bigrams.insert(parts[0].toLowerCase(), freq);
      }
    }
  }

  private parseTrigramsFallback(content: string): void {
    const lines = content.split('\n');
    for (const line of lines) {
      const parts = line.trim().split(/\s+/);
      if (parts.length >= 2) {
        const freq = parseInt(parts[1], 10) || 1;
        this.trigrams.insert(parts[0].toLowerCase(), freq);
      }
    }
  }

  private seedFallbackCorpus(): void {
    const defaultWords: [string, number][] = [
      ['the', 1000], ['be', 800], ['to', 750], ['of', 700], ['and', 650],
      ['a', 600], ['in', 550], ['that', 500], ['have', 450], ['i', 400],
      ['it', 380], ['for', 360], ['not', 340], ['on', 320], ['with', 300],
      ['he', 280], ['as', 260], ['you', 250], ['do', 240], ['at', 230],
      ['receive', 200], ['receipt', 180], ['definitely', 190], ['separate', 170],
      ['writing', 250], ['assistant', 240], ['intelligent', 220], ['algorithm', 210],
      ['structure', 205], ['computer', 230], ['science', 225], ['project', 240],
    ];

    for (const [w, f] of defaultWords) {
      this.trie.insert(w, f);
      this.dictHt.insert(w, f);
      this.unigrams.insert(w, f);
    }
  }

  /* ─── Public Engine API ─── */

  isReady(): boolean {
    return this.initialized;
  }

  getActiveEngine(): 'wasm' | 'fallback' {
    return this.activeEngine;
  }

  getEngineLabel(): string {
    return this.activeEngine === 'wasm'
      ? 'C Engine (WebAssembly)'
      : 'TypeScript Fallback';
  }

  /**
   * 1. Autocomplete via C Trie (or TypeScript Trie Fallback)
   */
  autocomplete(prefix: string, maxResults: number = 6): TrieSuggestion[] {
    const cleanPrefix = prefix.trim().toLowerCase();
    if (!cleanPrefix) return [];

    if (this.isWasmLoaded && this.wasmModule) {
      return this.withWasmString(cleanPrefix, (ptr) => {
        const resPtr = this.wasmModule._engine_autocomplete(ptr, maxResults);
        return this.readAndFreeJson<TrieSuggestion[]>(resPtr, []);
      });
    }

    // Fallback: TypeScript Trie
    const personal = this.personalTrie.autocomplete(cleanPrefix, maxResults);
    const standard = this.trie.autocomplete(cleanPrefix, maxResults);
    const map = new Map<string, number>();
    for (const p of personal) map.set(p.word, p.frequency + 10000);
    for (const s of standard) {
      if (!map.has(s.word)) map.set(s.word, s.frequency);
    }
    return Array.from(map.entries())
      .map(([word, frequency]) => ({ word, frequency }))
      .sort((a, b) => b.frequency - a.frequency)
      .slice(0, maxResults);
  }

  /**
   * 2. Spellcheck via C Damerau-Levenshtein + Min-Heap (or TypeScript Fallback)
   */
  spellcheck(word: string, maxResults: number = 5): SpellCheckResult {
    const clean = word.trim().toLowerCase();
    if (!clean) return { correct: true, suggestions: [] };

    if (this.isWasmLoaded && this.wasmModule) {
      return this.withWasmString(clean, (ptr) => {
        const resPtr = this.wasmModule._engine_spellcheck(ptr, maxResults);
        return this.readAndFreeJson<SpellCheckResult>(resPtr, { correct: true, suggestions: [] });
      });
    }

    // Fallback: TypeScript Damerau-Levenshtein + MinHeap
    if (this.dictHt.contains(clean) || this.personalHt.contains(clean)) {
      return { correct: true, suggestions: [] };
    }

    const heap = new MinHeap();
    for (const dictWord of this.dictHt.keys()) {
      if (Math.abs(dictWord.length - clean.length) > 2) continue;
      const dist = damerauLevenshtein(clean, dictWord);
      if (dist <= 2) {
        const freq = this.dictHt.get(dictWord) || 1;
        heap.insert(dictWord, dist, freq);
      }
    }

    const suggestions: { word: string; distance: number; frequency: number }[] = [];
    while (!heap.isEmpty() && suggestions.length < maxResults) {
      const min = heap.extractMin();
      if (min) suggestions.push(min);
    }

    return { correct: false, suggestions };
  }

  /**
   * 3. Next-Word Prediction via C N-Gram Model (or TypeScript Fallback)
   */
  predictNext(context: string, maxResults: number = 4): NGramPrediction[] {
    const cleanCtx = context.trim().toLowerCase();
    if (!cleanCtx) return [];

    if (this.isWasmLoaded && this.wasmModule) {
      return this.withWasmString(cleanCtx, (ptr) => {
        const resPtr = this.wasmModule._engine_predict_next(ptr, maxResults);
        return this.readAndFreeJson<NGramPrediction[]>(resPtr, []);
      });
    }

    // Fallback: TypeScript N-Gram Model
    const tokens = cleanCtx.split(/\s+/).filter(Boolean);
    if (tokens.length === 0) return [];

    const predictions: NGramPrediction[] = [];
    const seen = new Set<string>();

    if (tokens.length >= 2) {
      const w1 = tokens[tokens.length - 2];
      const w2 = tokens[tokens.length - 1];
      const prefix = `${w1}|${w2}|`;

      for (const key of this.trigrams.keys()) {
        if (key.startsWith(prefix)) {
          const target = key.slice(prefix.length);
          const score = (this.trigrams.get(key) || 1) / (this.bigrams.get(`${w1}|${w2}`) || 10);
          predictions.push({ word: target, score });
          seen.add(target);
        }
      }
    }

    if (tokens.length >= 1 && predictions.length < maxResults) {
      const w1 = tokens[tokens.length - 1];
      const prefix = `${w1}|`;

      for (const key of this.bigrams.keys()) {
        if (key.startsWith(prefix)) {
          const target = key.slice(prefix.length);
          if (!seen.has(target)) {
            const score = (this.bigrams.get(key) || 1) / (this.unigrams.get(w1) || 50);
            predictions.push({ word: target, score: score * 0.7 });
            seen.add(target);
          }
        }
      }
    }

    predictions.sort((a, b) => b.score - a.score);
    return predictions.slice(0, maxResults);
  }

  /**
   * 4. Word Existence Lookup via C Hash Table
   */
  wordExists(word: string): boolean {
    const clean = word.trim().toLowerCase();
    if (!clean) return false;

    if (this.isWasmLoaded && this.wasmModule) {
      return this.withWasmString(clean, (ptr) => {
        return this.wasmModule._engine_word_exists(ptr) === 1;
      });
    }

    return this.dictHt.contains(clean) || this.personalHt.contains(clean);
  }

  /**
   * 5. Personal Dictionary Addition via C Engine
   */
  addWord(word: string): boolean {
    const clean = word.trim().toLowerCase();
    if (!clean) return false;

    if (this.isWasmLoaded && this.wasmModule) {
      this.withWasmString(clean, (ptr) => {
        this.wasmModule._engine_add_word(ptr);
      });
    }

    this.personalTrie.insert(clean, 5000);
    this.personalHt.insert(clean, 5000);
    return true;
  }

  /**
   * 6. Personal Dictionary Removal via C Engine
   */
  removeWord(word: string): boolean {
    const clean = word.trim().toLowerCase();
    if (!clean) return false;

    if (this.isWasmLoaded && this.wasmModule) {
      this.withWasmString(clean, (ptr) => {
        this.wasmModule._engine_remove_word(ptr);
      });
    }

    return this.personalHt.remove(clean);
  }

  /**
   * 7. Undo via C LIFO Stack
   */
  pushUndo(text: string): boolean {
    if (this.isWasmLoaded && this.wasmModule) {
      this.withWasmString(text, (ptr) => {
        this.wasmModule._engine_push_undo(ptr);
      });
    }

    this.undoStack.push(text);
    return true;
  }

  undo(): { text: string; timestamp: number } | null {
    if (this.isWasmLoaded && this.wasmModule) {
      const resPtr = this.wasmModule._engine_undo();
      if (resPtr) {
        return this.readAndFreeJson<{ text: string; timestamp: number } | null>(resPtr, null);
      }
      return null;
    }

    return this.undoStack.pop();
  }

  /**
   * 8. History Audit via C Linked List
   */
  addHistory(original: string, modified: string, type: HistoryType): boolean {
    if (this.isWasmLoaded && this.wasmModule) {
      this.withWasmString(original, (origPtr) => {
        this.withWasmString(modified, (modPtr) => {
          this.wasmModule._engine_add_history(origPtr, modPtr, type);
        });
      });
    }

    this.historyList.add(original, modified, type);
    return true;
  }

  getHistory(): HistoryEntry[] {
    if (this.isWasmLoaded && this.wasmModule) {
      const resPtr = this.wasmModule._engine_get_history();
      return this.readAndFreeJson<HistoryEntry[]>(resPtr, []);
    }

    return this.historyList.toArray();
  }

  clearHistory(): void {
    if (this.isWasmLoaded && this.wasmModule) {
      this.wasmModule._engine_clear_history();
    }
    this.historyList.clear();
  }

  /**
   * 9. Engine Statistics via C Engine
   */
  getStats(): EngineStats {
    if (this.isWasmLoaded && this.wasmModule) {
      const resPtr = this.wasmModule._engine_get_stats();
      // eslint-disable-next-line @typescript-eslint/no-explicit-any
      const raw = this.readAndFreeJson<any>(resPtr, null);
      if (raw) {
        return {
          dict_words: raw.dict_words || 0,
          trie_nodes: raw.trie_nodes || 0,
          personal_words: raw.personal_words || 0,
          unigram_count: raw.unigram_entries || 0,
          history_entries: raw.history_entries || 0,
          undo_stack_size: raw.undo_depth || 0,
          trieWordCount: raw.dict_words || 0,
          hashTableEntries: raw.dict_words || 0,
          bigramCount: raw.bigram_entries || 0,
          trigramCount: raw.trigram_entries || 0,
          activeEngine: 'C Engine (WebAssembly)',
        };
      }
    }

    return {
      dict_words: this.dictHt.size(),
      trie_nodes: this.trie.nodeCount,
      personal_words: this.personalHt.size(),
      unigram_count: this.unigrams.size(),
      history_entries: this.historyList.size(),
      undo_stack_size: this.undoStack.size(),
      trieWordCount: this.dictHt.size(),
      hashTableEntries: this.dictHt.size(),
      bigramCount: this.bigrams.size(),
      trigramCount: this.trigrams.size(),
      activeEngine: 'TypeScript Fallback',
    };
  }
}

// Global Singleton Export
export const engine = new TypeAssistEngine();
