/**
 * TypeAssist — TypeScript interfaces for the C WebAssembly Engine
 */

export interface TrieSuggestion {
  word: string;
  frequency: number;
}

export interface SpellSuggestion {
  word: string;
  distance: number;
  frequency?: number;
}

export interface SpellCheckResult {
  correct: boolean;
  suggestions: SpellSuggestion[];
}

export interface NGramPrediction {
  word: string;
  score: number;
}

export enum HistoryType {
  CORRECTION = 0,
  AUTOCOMPLETE = 1,
  PREDICTION = 2,
  TONE_SHIFT = 3,
  TRANSLATION = 4,
  GRAMMAR_FIX = 5,
}

export interface HistoryEntry {
  original: string;
  modified: string;
  type: HistoryType;
  timestamp: number;
}

export interface EngineStats {
  dict_words: number;
  trie_nodes: number;
  personal_words: number;
  unigram_count: number;
  history_entries: number;
  undo_stack_size: number;
}

export interface TypeAssistEngineInterface {
  init(): Promise<boolean>;
  isReady(): boolean;
  autocomplete(prefix: string, maxResults?: number): TrieSuggestion[];
  spellcheck(word: string, maxResults?: number): SpellCheckResult;
  predictNext(context: string, maxResults?: number): NGramPrediction[];
  wordExists(word: string): boolean;
  addWord(word: string): boolean;
  removeWord(word: string): boolean;
  pushUndo(text: string): boolean;
  undo(): { text: string; timestamp: number } | null;
  addHistory(original: string, modified: string, type: HistoryType): boolean;
  getHistory(): HistoryEntry[];
  clearHistory(): void;
  getStats(): EngineStats | null;
}

