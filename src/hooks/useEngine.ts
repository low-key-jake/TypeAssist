'use client';

import { useEffect, useState, useCallback } from 'react';
import { engine } from '@/lib/wasm/engine';
import {
  TrieSuggestion,
  SpellCheckResult,
  NGramPrediction,
  HistoryEntry,
  HistoryType,
  EngineStats,
} from '@/lib/wasm/types';
import {
  getStoredPersonalDictionary,
  saveStoredPersonalDictionary,
} from '@/lib/storage';

export function useEngine() {
  const [isReady, setIsReady] = useState<boolean>(false);
  const [stats, setStats] = useState<EngineStats | null>(null);
  const [history, setHistory] = useState<HistoryEntry[]>([]);
  const [personalWords, setPersonalWords] = useState<string[]>([]);

  useEffect(() => {
    let mounted = true;

    async function setup() {
      await engine.init();
      if (!mounted) return;

      // Load stored personal words into the engine
      const stored = getStoredPersonalDictionary();
      stored.forEach((w) => engine.addWord(w));
      setPersonalWords(stored);

      setIsReady(true);
      setStats(engine.getStats());
      setHistory(engine.getHistory());
    }

    setup();
    return () => {
      mounted = false;
    };
  }, []);

  const refreshStats = useCallback(() => {
    setStats(engine.getStats());
  }, []);

  const refreshHistory = useCallback(() => {
    setHistory(engine.getHistory());
  }, []);

  const getAutocomplete = useCallback(
    (prefix: string, maxResults: number = 5): TrieSuggestion[] => {
      if (!isReady || !prefix.trim()) return [];
      return engine.autocomplete(prefix, maxResults);
    },
    [isReady]
  );

  const checkSpelling = useCallback(
    (word: string, maxResults: number = 4): SpellCheckResult => {
      if (!isReady || !word.trim()) return { correct: true, suggestions: [] };
      return engine.spellcheck(word, maxResults);
    },
    [isReady]
  );

  const predictNextWords = useCallback(
    (context: string, maxResults: number = 4): NGramPrediction[] => {
      if (!isReady || !context.trim()) return [];
      return engine.predictNext(context, maxResults);
    },
    [isReady]
  );

  const addPersonalWord = useCallback(
    (word: string): boolean => {
      const clean = word.trim().toLowerCase();
      if (!clean) return false;
      const success = engine.addWord(clean);
      if (success) {
        const updated = Array.from(new Set([...personalWords, clean]));
        setPersonalWords(updated);
        saveStoredPersonalDictionary(updated);
        refreshStats();
      }
      return success;
    },
    [personalWords, refreshStats]
  );

  const removePersonalWord = useCallback(
    (word: string): boolean => {
      const clean = word.trim().toLowerCase();
      const success = engine.removeWord(clean);
      if (success) {
        const updated = personalWords.filter((w) => w !== clean);
        setPersonalWords(updated);
        saveStoredPersonalDictionary(updated);
        refreshStats();
      }
      return success;
    },
    [personalWords, refreshStats]
  );

  const pushUndoSnapshot = useCallback((text: string) => {
    engine.pushUndo(text);
  }, []);

  const undoLastEdit = useCallback((): { text: string; timestamp: number } | null => {
    return engine.undo();
  }, []);

  const logHistoryEvent = useCallback(
    (original: string, modified: string, type: HistoryType) => {
      engine.addHistory(original, modified, type);
      refreshHistory();
      refreshStats();
    },
    [refreshHistory, refreshStats]
  );

  const clearHistoryLog = useCallback(() => {
    engine.clearHistory();
    refreshHistory();
    refreshStats();
  }, [refreshHistory, refreshStats]);

  return {
    isReady,
    stats,
    history,
    personalWords,
    getAutocomplete,
    checkSpelling,
    predictNextWords,
    addPersonalWord,
    removePersonalWord,
    pushUndoSnapshot,
    undoLastEdit,
    logHistoryEvent,
    clearHistoryLog,
    refreshStats,
  };
}

