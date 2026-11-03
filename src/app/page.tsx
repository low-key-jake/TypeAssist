'use client';

import React, { useState, useRef, useMemo, useEffect, useCallback } from 'react';
import { useEngine } from '@/hooks/useEngine';
import { HistoryType } from '@/lib/wasm/types';
import {
  Feather,
  Globe,
  Sparkles,
  Wand2,
  CheckCircle2,
  RotateCcw,
  Copy,
  Check,
  ArrowRight,
  Sun,
  Moon,
  Volume2,
  VolumeX,
  RefreshCw,
  X,
  BookPlus,
  Cpu,
} from 'lucide-react';

/* ─── Tab definitions (Clean 3-Tab Suite: Autocorrect, Tone Shifter, Sentence Doctor) ─── */
type TabId = 'autocorrect' | 'tone-shifter' | 'sentence-doctor';

const TABS: { id: TabId; label: string; icon: React.ComponentType<{ className?: string }> }[] = [
  { id: 'autocorrect', label: 'Autocorrect', icon: CheckCircle2 },
  { id: 'tone-shifter', label: 'Tone Shifter', icon: Sparkles },
  { id: 'sentence-doctor', label: 'Sentence Doctor', icon: Wand2 },
];

const TONES = [
  'Gen Z', 'Professional', 'Diplomatic', 'Shakespearean',
  'Casual', 'Polite', 'Sarcastic',
];

export default function HomePage() {
  const {
    isReady,
    getAutocomplete,
    checkSpelling,
    predictNextWords,
    addPersonalWord,
    pushUndoSnapshot,
    undoLastEdit,
    logHistoryEvent,
    stats,
    activeEngine,
    engineLabel,
  } = useEngine();

  /* ─── State ─── */
  const [activeTab, setActiveTab] = useState<TabId>('autocorrect');
  const [text, setText] = useState('');
  const [copied, setCopied] = useState(false);
  const [darkMode, setDarkMode] = useState(true);
  const [cursorPos, setCursorPos] = useState(0);
  const [showDsaModal, setShowDsaModal] = useState(false);

  // Intro Video state (plays before showing main website)
  const [showIntro, setShowIntro] = useState(true);
  const [introMuted, setIntroMuted] = useState(true);
  const introVideoRef = useRef<HTMLVideoElement>(null);

  // Tone Shifter state
  const [selectedTone, setSelectedTone] = useState('Gen Z');
  const [toneResult, setToneResult] = useState('');
  const [toneLoading, setToneLoading] = useState(false);

  // Sentence Doctor state
  const [doctorResult, setDoctorResult] = useState<{
    corrected: string;
    improvements: { issue: string; fix: string; rule: string }[];
    clarityScore: number;
  } | null>(null);
  const [doctorLoading, setDoctorLoading] = useState(false);

  const textareaRef = useRef<HTMLTextAreaElement>(null);
  const videoRef = useRef<HTMLVideoElement>(null);

  /* ─── Video Autoplay & Loop Assurance ─── */
  useEffect(() => {
    if (showIntro && introVideoRef.current) {
      introVideoRef.current.defaultMuted = true;
      introVideoRef.current.muted = true;
      introVideoRef.current.play().catch(() => {});
    }
    if (videoRef.current) {
      videoRef.current.defaultMuted = true;
      videoRef.current.muted = true;
      videoRef.current.play().catch(() => {});
    }
  }, [showIntro]);

  /* ─── C-Engine Autocomplete & Spell logic ─── */
  const { currentPrefix, precedingContext } = useMemo(() => {
    const before = text.slice(0, cursorPos);
    const m = before.match(/([a-zA-Z']+)\s*$/);
    const prefix = m ? m[1] : '';
    const words = before.trim().split(/\s+/);
    return { currentPrefix: prefix, precedingContext: words.slice(-3).join(' ') };
  }, [text, cursorPos]);

  // C Trie Autocomplete (O(L) time complexity, clean words only)
  const autocompleteSuggestions = useMemo(() => {
    if (!currentPrefix || currentPrefix.length < 2) return [];
    return getAutocomplete(currentPrefix, 5);
  }, [currentPrefix, getAutocomplete]);

  // C N-Gram Context Prediction P(W_n | W_{n-1}, W_{n-2})
  const nextWordPredictions = useMemo(() => {
    if (autocompleteSuggestions.length > 0) return [];
    if (!precedingContext) return [];
    return predictNextWords(precedingContext, 4);
  }, [precedingContext, autocompleteSuggestions.length, predictNextWords]);

  // C Damerau-Levenshtein Edit Distance + Min-Heap Top Candidates
  const detectedTypos = useMemo(() => {
    if (!isReady || !text || activeTab !== 'autocorrect') return [];
    const words = text.match(/\b[a-zA-Z']+\b/g) || [];
    const typos: { word: string; suggestions: { word: string; distance: number; frequency?: number }[] }[] = [];
    const seen = new Set<string>();
    for (const w of words) {
      const lower = w.toLowerCase();
      if (lower.length > 1 && !seen.has(lower)) {
        seen.add(lower);
        const result = checkSpelling(lower, 4);
        if (!result.correct && result.suggestions.length > 0) {
          typos.push({ word: w, suggestions: result.suggestions });
        }
      }
    }
    return typos;
  }, [text, isReady, checkSpelling, activeTab]);

  /* ─── Handlers ─── */
  const handleTextChange = (e: React.ChangeEvent<HTMLTextAreaElement>) => {
    const val = e.target.value;
    pushUndoSnapshot(text);
    setText(val);
    setCursorPos(e.target.selectionStart || val.length);
  };

  const handleSelect = (e: React.SyntheticEvent<HTMLTextAreaElement>) => {
    setCursorPos((e.target as HTMLTextAreaElement).selectionStart || 0);
  };

  const handleAcceptAutocomplete = useCallback((completedWord: string) => {
    if (!currentPrefix) return;
    const before = text.slice(0, cursorPos);
    const after = text.slice(cursorPos);
    // Strip any unexpected trailing numbers or spaces
    const cleanWord = completedWord.replace(/[0-9\s]+$/g, '');
    const updated = before.replace(new RegExp(`${currentPrefix}$`), cleanWord + ' ');
    pushUndoSnapshot(text);
    setText(updated + after);
    logHistoryEvent(currentPrefix, cleanWord, HistoryType.AUTOCOMPLETE);
    setTimeout(() => {
      if (textareaRef.current) {
        textareaRef.current.focus();
        textareaRef.current.setSelectionRange(updated.length, updated.length);
        setCursorPos(updated.length);
      }
    }, 10);
  }, [currentPrefix, text, cursorPos, pushUndoSnapshot, logHistoryEvent]);

  const handleAcceptPrediction = useCallback((word: string) => {
    const before = text.slice(0, cursorPos).trimEnd();
    const after = text.slice(cursorPos);
    const sep = before.length > 0 ? ' ' : '';
    const cleanWord = word.replace(/[0-9\s]+$/g, '');
    const updated = `${before}${sep}${cleanWord} `;
    pushUndoSnapshot(text);
    setText(updated + after);
    logHistoryEvent(precedingContext, cleanWord, HistoryType.PREDICTION);
    setTimeout(() => {
      if (textareaRef.current) {
        textareaRef.current.focus();
        textareaRef.current.setSelectionRange(updated.length, updated.length);
        setCursorPos(updated.length);
      }
    }, 10);
  }, [text, cursorPos, precedingContext, pushUndoSnapshot, logHistoryEvent]);

  const handleCorrectTypo = (typo: string, replacement: string) => {
    const regex = new RegExp(`\\b${typo}\\b`, 'g');
    const cleanReplacement = replacement.replace(/[0-9\s]+$/g, '');
    const cased = preserveCase(typo, cleanReplacement);
    const newText = text.replace(regex, cased);
    pushUndoSnapshot(text);
    setText(newText);
    logHistoryEvent(typo, cased, HistoryType.CORRECTION);
  };

  const handleUndo = () => {
    const prev = undoLastEdit();
    if (prev && prev.text !== undefined) setText(prev.text);
  };

  const handleCopy = async () => {
    const target = activeTab === 'tone-shifter' && toneResult ? toneResult :
                   activeTab === 'sentence-doctor' && doctorResult?.corrected ? doctorResult.corrected :
                   text;
    if (!target) return;
    await navigator.clipboard.writeText(target);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  /* ─── Keyboard shortcut: Tab to accept autocomplete/prediction ─── */
  useEffect(() => {
    const handler = (e: KeyboardEvent) => {
      if (e.key === 'Tab' && activeTab === 'autocorrect') {
        if (autocompleteSuggestions.length > 0) {
          e.preventDefault();
          handleAcceptAutocomplete(autocompleteSuggestions[0].word);
        } else if (nextWordPredictions.length > 0) {
          e.preventDefault();
          handleAcceptPrediction(nextWordPredictions[0].word);
        }
      }
    };
    window.addEventListener('keydown', handler);
    return () => window.removeEventListener('keydown', handler);
  }, [autocompleteSuggestions, nextWordPredictions, activeTab, handleAcceptAutocomplete, handleAcceptPrediction]);

  /* ─── Case-preserving helper for Autocorrect ─── */
  const preserveCase = (orig: string, target: string) => {
    if (orig === orig.toUpperCase()) return target.toUpperCase();
    if (orig.charAt(0) === orig.charAt(0).toUpperCase()) {
      return target.charAt(0).toUpperCase() + target.slice(1);
    }
    return target.toLowerCase();
  };

  /* ─── Action: Improve Text / Transform ─── */
  const handleImproveText = async () => {
    if (!text.trim()) return;
    if (activeTab === 'autocorrect') {
      let improved = text;
      for (const t of detectedTypos) {
        if (t.suggestions.length > 0) {
          const regex = new RegExp(`\\b${t.word}\\b`, 'g');
          const cleanSug = t.suggestions[0].word.replace(/[0-9\s]+$/g, '');
          const casedSug = preserveCase(t.word, cleanSug);
          improved = improved.replace(regex, casedSug);
        }
      }
      if (improved !== text) {
        pushUndoSnapshot(text);
        setText(improved);
      }
    } else if (activeTab === 'tone-shifter') {
      await handleToneShift();
    } else if (activeTab === 'sentence-doctor') {
      await handleSentenceDoctor();
    }
  };

  const handleToneShift = async (toneToUse?: string) => {
    if (!text.trim()) return;
    const currentTone = toneToUse || selectedTone;
    setToneLoading(true);
    try {
      const res = await fetch('/api/tone-shift', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text, tone: currentTone }),
      });
      const data = await res.json();
      if (data.transformed) {
        setToneResult(data.transformed);
        logHistoryEvent(text, data.transformed, HistoryType.TONE_SHIFT);
      }
    } catch (e) {
      console.error(e);
    } finally {
      setToneLoading(false);
    }
  };

  const handleSentenceDoctor = async () => {
    if (!text.trim()) return;
    setDoctorLoading(true);
    try {
      const res = await fetch('/api/grammar', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ text }),
      });
      const data = await res.json();
      if (data.corrected) {
        setDoctorResult(data);
        logHistoryEvent(text, data.corrected, HistoryType.GRAMMAR_FIX);
      }
    } catch (e) {
      console.error(e);
    } finally {
      setDoctorLoading(false);
    }
  };

  const wordCount = useMemo(() => (text.trim() ? text.trim().split(/\s+/).length : 0), [text]);
  const charCount = text.length;

  const isLoading = toneLoading || doctorLoading;

  const buttonLabel = activeTab === 'autocorrect' ? 'Improve Text' :
                      activeTab === 'tone-shifter' ? 'Transform' : 'Fix Grammar';

  return (
    <div className="min-h-screen w-full relative overflow-x-hidden">

      {/* ─── Pre-Site Intro Video Experience ─── */}
      {showIntro && (
        <div className="fixed inset-0 z-50 bg-black flex items-center justify-center overflow-hidden">
          <video
            ref={introVideoRef}
            autoPlay
            playsInline
            muted={introMuted}
            onEnded={() => setShowIntro(false)}
            className="w-full h-full object-cover"
          >
            <source src="/intro-video.mp4" type="video/mp4" />
          </video>

          {/* Side Controls: Skip Intro & Audio Toggle */}
          <div className="fixed top-6 right-8 z-50 flex items-center gap-3">
            <button
              onClick={() => {
                if (introVideoRef.current) {
                  introVideoRef.current.muted = !introMuted;
                  setIntroMuted(!introMuted);
                }
              }}
              className="flex items-center gap-1.5 px-3.5 py-2 rounded-full bg-black/60 hover:bg-black/85 border border-white/20 text-xs font-medium text-white/80 hover:text-white backdrop-blur-xl shadow-lg transition-all cursor-pointer"
              title={introMuted ? "Unmute intro" : "Mute intro"}
            >
              {introMuted ? <VolumeX className="w-4 h-4 text-white/70" /> : <Volume2 className="w-4 h-4 text-[#e5c583]" />}
              <span className="hidden sm:inline">{introMuted ? 'Unmute' : 'Mute'}</span>
            </button>

            <button
              onClick={() => setShowIntro(false)}
              className="flex items-center gap-2 px-5 py-2 rounded-full bg-black/70 hover:bg-black/90 border border-[#e5c583]/60 text-xs md:text-sm font-semibold text-[#e5c583] hover:text-white backdrop-blur-xl shadow-[0_4px_30px_rgba(0,0,0,0.85)] hover:scale-105 active:scale-95 transition-all cursor-pointer"
            >
              <span>Skip Intro</span>
              <ArrowRight className="w-4 h-4 text-[#e5c583]" />
            </button>
          </div>
        </div>
      )}

      {/* ─── 100vw x 100vh Fullscreen Video & Cathedral Poster Background ─── */}
      <div className="bg-video-wrapper">
        <video
          ref={videoRef}
          autoPlay
          loop
          muted
          playsInline
          preload="auto"
          poster="/bg-poster.jpg"
          className="bg-video-element"
        >
          <source src="/bg-video.mp4" type="video/mp4" />
        </video>
        <div className="bg-video-overlay" />
      </div>

      {/* ─── Foreground Application Layer ─── */}
      <div className="relative z-10 min-h-screen flex flex-col justify-between">

        {/* ─── Top Navigation Bar (Matches Reference Image) ─── */}
        <header className="px-8 py-6 flex items-center justify-between">
          <div className="flex items-center gap-2.5">
            <Feather className="w-6 h-6 text-[#e5c583] rotate-[-15deg] drop-shadow-[0_2px_10px_rgba(229,197,131,0.5)]" />
            <span className="font-display text-2xl font-bold tracking-tight text-[#f5f3ef]">
              TypeAssist
            </span>
          </div>

          <div className="flex items-center gap-3">
            {/* C Data Structures Inspector Button */}
            <button
              onClick={() => setShowDsaModal(true)}
              className="hidden sm:flex items-center gap-1.5 px-3 py-1.5 rounded-full bg-black/45 border border-amber-400/30 text-xs font-medium text-[#e5c583] hover:bg-black/60 transition-all backdrop-blur-md shadow-sm"
              title={`Active Engine: ${engineLabel}`}
            >
              <Cpu className="w-3.5 h-3.5 text-[#e5c583]" />
              <span>{activeEngine === 'wasm' ? 'C Engine (WASM)' : 'TS Fallback'}</span>
            </button>

            {/* Dark/Light mode pill */}
            <button
              onClick={() => setDarkMode(!darkMode)}
              className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-black/40 border border-white/10 text-white/70 hover:text-white transition-all backdrop-blur-md"
            >
              <Sun className={`w-3.5 h-3.5 ${!darkMode ? 'text-amber-300' : 'text-white/50'}`} />
              <Moon className={`w-3.5 h-3.5 ${darkMode ? 'text-amber-200' : 'text-white/50'}`} />
            </button>

            {/* Profile Avatar */}
            <div className="w-9 h-9 rounded-full bg-[#352518] border border-amber-300/40 flex items-center justify-center text-white text-sm font-semibold shadow-lg">
              K
            </div>
          </div>
        </header>

        {/* ─── Hero Title Section (Matches Reference Image Exactly) ─── */}
        <section className="px-6 pt-2 pb-6 text-center">
          <h1 className="font-display text-5xl md:text-6xl font-normal tracking-wide text-[#f8f6f0] leading-tight mb-2">
            Think it. Type it.
          </h1>
          <h1 className="font-display text-5xl md:text-6xl italic font-normal tracking-wide text-[#e5c583] leading-tight mb-5 drop-shadow-[0_2px_15px_rgba(229,197,131,0.25)]">
            Transform it.
          </h1>

          <div className="diamond-divider mb-4 opacity-75">
            <span className="text-[#e5c583] text-xs animate-gold-glow">◇</span>
          </div>

          <p className="text-sm md:text-base text-[#cfc7ba] font-light tracking-wide">
            An intelligent writing assistant for every thought.
          </p>
        </section>

        {/* ─── Central Editor Box (Matched to Reference Image) ─── */}
        <main className="flex-1 px-4 md:px-8 max-w-4xl w-full mx-auto flex flex-col justify-start">
          <div className="glass-editor-card rounded-3xl overflow-hidden transition-all">

            {/* Top Navigation Row inside Card (Autocorrect, Tone Shifter, Sentence Doctor) */}
            <div className="px-4 py-3 border-b border-white/[0.06] flex items-center justify-between flex-wrap gap-2 bg-black/25">
              <div className="flex items-center gap-1.5 flex-wrap">
                {TABS.map((tab) => {
                  const Icon = tab.icon;
                  const isActive = activeTab === tab.id;
                  return (
                    <button
                      key={tab.id}
                      onClick={() => {
                        setActiveTab(tab.id);
                        if (tab.id === 'tone-shifter' && text.trim() && !toneResult) {
                          handleToneShift(selectedTone);
                        }
                      }}
                      className={`flex items-center gap-2 px-4 py-2 rounded-xl text-sm font-medium transition-all ${
                        isActive
                          ? 'bg-[#8c6734]/55 border border-[#dfb36c]/65 text-white shadow-[0_2px_14px_rgba(212,160,77,0.35)]'
                          : 'text-[#cfc7ba]/75 hover:text-white hover:bg-white/[0.04]'
                      }`}
                    >
                      <Icon className="w-4 h-4 text-[#e5c583]" />
                      <span>{tab.label}</span>
                    </button>
                  );
                })}
              </div>

              {/* Language Indicator */}
              <div className="flex items-center gap-2 text-xs md:text-sm text-[#cfc7ba] px-3 py-1.5 rounded-lg">
                <span className="text-white/40">Language:</span>
                <Globe className="w-4 h-4 text-[#e5c583]" />
                <span className="text-white font-medium">English</span>
              </div>
            </div>

            {/* Sub-toolbar for Tone Shifter */}
            {activeTab === 'tone-shifter' && (
              <div className="px-5 py-2.5 bg-black/35 border-b border-white/[0.04] flex items-center gap-2 overflow-x-auto">
                <span className="text-xs text-white/40 shrink-0">Select Tone:</span>
                {TONES.map((t) => (
                  <button
                    key={t}
                    onClick={() => {
                      setSelectedTone(t);
                      if (text.trim()) {
                        handleToneShift(t);
                      }
                    }}
                    className={`px-3 py-1 rounded-lg text-xs font-medium transition-all shrink-0 ${
                      selectedTone === t
                        ? 'bg-[#8c6734]/70 text-white shadow-[0_0_12px_rgba(212,160,77,0.4)] border border-[#dfb36c]/60'
                        : 'text-white/60 hover:text-white hover:bg-white/5 border border-white/5'
                    }`}
                  >
                    {t}
                  </button>
                ))}
              </div>
            )}

            {/* Autocomplete / Prediction Pills (Powered by C Trie & C N-Gram) */}
            {activeTab === 'autocorrect' && (autocompleteSuggestions.length > 0 || nextWordPredictions.length > 0) && (
              <div className="px-5 py-2 bg-amber-950/25 border-b border-amber-500/10 flex items-center gap-2 flex-wrap text-xs">
                {autocompleteSuggestions.length > 0 && (
                  <>
                    <span className="text-[10px] font-mono uppercase tracking-wider text-[#e5c583]">
                      Trie Complete:
                    </span>
                    {autocompleteSuggestions.map((item, idx) => (
                      <button
                        key={item.word}
                        onClick={() => handleAcceptAutocomplete(item.word)}
                        className={`px-2.5 py-0.5 rounded-md border text-xs font-medium transition-all ${
                          idx === 0
                            ? 'bg-[#8c6734] text-white border-[#dfb36c]'
                            : 'text-white/70 border-white/10 hover:bg-white/5'
                        }`}
                      >
                        {item.word} {idx === 0 && <span className="opacity-60 ml-1 text-[9px]">⇥</span>}
                      </button>
                    ))}
                  </>
                )}

                {nextWordPredictions.length > 0 && (
                  <>
                    <span className="text-[10px] font-mono uppercase tracking-wider text-amber-200/70">
                      N-Gram Predict:
                    </span>
                    {nextWordPredictions.map((pred, idx) => (
                      <button
                        key={pred.word}
                        onClick={() => handleAcceptPrediction(pred.word)}
                        className={`px-2.5 py-0.5 rounded-md border text-xs font-medium transition-all ${
                          idx === 0
                            ? 'bg-[#6b4f24] text-white border-[#dfb36c]/70'
                            : 'text-white/70 border-white/10 hover:bg-white/5'
                        }`}
                      >
                        {pred.word} {idx === 0 && <span className="opacity-60 ml-1 text-[9px]">⇥</span>}
                      </button>
                    ))}
                  </>
                )}
              </div>
            )}

            {/* Main Textarea */}
            <div className="p-6">
              <textarea
                ref={textareaRef}
                value={text}
                onChange={handleTextChange}
                onSelect={handleSelect}
                rows={5}
                placeholder="Start typing... or paste your text here"
                className="w-full bg-transparent resize-none text-base md:text-lg leading-relaxed text-white/90 placeholder-white/25 focus:outline-none"
              />
            </div>

            {/* C Damerau-Levenshtein typo correction suggestions */}
            {activeTab === 'autocorrect' && detectedTypos.length > 0 && (
              <div className="px-6 py-2.5 bg-red-950/25 border-t border-red-500/20 flex items-center gap-3 flex-wrap text-xs">
                <span className="text-red-300 font-medium">
                  {detectedTypos.length} typo{detectedTypos.length > 1 ? 's' : ''} detected:
                </span>
                {detectedTypos.slice(0, 4).map(({ word, suggestions }) => (
                  <div key={word} className="flex items-center gap-1.5">
                    <span className="line-through text-red-400/80">{word}</span>
                    <ArrowRight className="w-3 h-3 text-white/40" />
                    {suggestions.slice(0, 2).map((s) => (
                      <button
                        key={s.word}
                        onClick={() => handleCorrectTypo(word, s.word)}
                        className="px-2 py-0.5 rounded bg-emerald-500/20 border border-emerald-500/30 text-emerald-300 hover:bg-emerald-500/30 transition-colors"
                        title={`Cost: ${s.distance} (Min-Heap prioritized)`}
                      >
                        {s.word}
                      </button>
                    ))}
                    <button
                      onClick={() => addPersonalWord(word)}
                      className="p-1 text-white/40 hover:text-white/80"
                      title="Add to personal lexicon"
                    >
                      <BookPlus className="w-3 h-3" />
                    </button>
                  </div>
                ))}
              </div>
            )}

            {/* Result display area (for Tone Shift & Sentence Doctor) */}
            {((activeTab === 'tone-shifter' && toneResult) ||
              (activeTab === 'sentence-doctor' && doctorResult)) && (
              <div className="px-6 py-4 bg-amber-950/35 border-t border-amber-500/25 space-y-2">
                <div className="flex items-center justify-between text-xs text-[#e5c583] font-semibold tracking-wider uppercase">
                  <span>
                    {activeTab === 'tone-shifter' ? `${selectedTone} Style Transformation` :
                     `Sentence Doctor (Clarity Score: ${doctorResult?.clarityScore}%)`}
                  </span>
                  <div className="flex items-center gap-2">
                    <button
                      onClick={() => {
                        const target = activeTab === 'tone-shifter' ? toneResult :
                                       (doctorResult?.corrected || '');
                        if (target) {
                          pushUndoSnapshot(text);
                          setText(target);
                        }
                      }}
                      className="px-2.5 py-0.5 rounded bg-[#8c6734]/80 hover:bg-[#8c6734] text-[11px] text-white font-medium transition-colors"
                      title="Apply transformed text to editor"
                    >
                      Use in Editor
                    </button>
                    <button
                      onClick={() => {
                        setToneResult('');
                        setDoctorResult(null);
                      }}
                      className="p-1 text-white/40 hover:text-white"
                    >
                      <X className="w-3.5 h-3.5" />
                    </button>
                  </div>
                </div>
                <p className="text-sm md:text-base text-white/95 leading-relaxed font-light">
                  {activeTab === 'tone-shifter' ? toneResult : doctorResult?.corrected}
                </p>
              </div>
            )}

            {/* Bottom Bar inside Card (Matches Reference Image) */}
            <div className="px-6 py-4 border-t border-white/[0.06] flex items-center justify-between flex-wrap gap-4 bg-black/15">
              <div className="text-xs text-[#a8a095] tracking-wide font-light">
                {wordCount} words • {charCount} characters
              </div>

              <div className="flex items-center gap-3">
                {/* Undo Button */}
                <button
                  onClick={handleUndo}
                  className="flex items-center gap-1.5 px-4 py-2 rounded-xl bg-white/[0.04] hover:bg-white/[0.08] border border-white/10 text-xs font-medium text-[#cfc7ba] transition-all backdrop-blur-md"
                >
                  <RotateCcw className="w-3.5 h-3.5 text-white/60" />
                  <span>Undo</span>
                </button>

                {/* Copy Button */}
                <button
                  onClick={handleCopy}
                  className="flex items-center gap-1.5 px-4 py-2 rounded-xl bg-white/[0.04] hover:bg-white/[0.08] border border-white/10 text-xs font-medium text-[#cfc7ba] transition-all backdrop-blur-md"
                >
                  {copied ? <Check className="w-3.5 h-3.5 text-emerald-400" /> : <Copy className="w-3.5 h-3.5 text-white/60" />}
                  <span>{copied ? 'Copied' : 'Copy'}</span>
                </button>

                {/* Golden Improve Text Button */}
                <button
                  onClick={handleImproveText}
                  disabled={isLoading || !text.trim()}
                  className="btn-improve-gold flex items-center gap-2 px-6 py-2.5 rounded-xl text-xs font-semibold disabled:opacity-50 transition-all cursor-pointer"
                >
                  {isLoading ? (
                    <RefreshCw className="w-3.5 h-3.5 animate-spin text-[#1a1205]" />
                  ) : (
                    <Sparkles className="w-3.5 h-3.5 text-[#1a1205]" />
                  )}
                  <span>{buttonLabel}</span>
                  <ArrowRight className="w-3.5 h-3.5 text-[#1a1205]" />
                </button>
              </div>
            </div>

          </div>

          {/* C-Engine telemetry pill */}
          <div className="mt-4 text-center">
            <button
              onClick={() => setShowDsaModal(true)}
              className="inline-flex items-center gap-2 px-3.5 py-1.5 rounded-full bg-black/45 border border-amber-400/25 text-[11px] text-[#e5c583] hover:text-white hover:border-amber-400/50 transition-all backdrop-blur-md shadow-sm"
            >
              <Cpu className="w-3 h-3 text-[#e5c583]" />
              <span>C-Core: Trie • Min-Heap • Damerau-Levenshtein • Hash Table • N-Grams</span>
              <span className={`text-[9px] px-1.5 py-0.5 rounded font-mono ${activeEngine === 'wasm' ? 'bg-emerald-500/20 text-emerald-300 border border-emerald-500/30' : 'bg-amber-500/20 text-amber-300 border border-amber-500/30'}`}>{engineLabel}</span>
            </button>
          </div>
        </main>

        {/* ─── Minimal Bottom Space ─── */}
        <footer className="py-6 text-center" />

      </div>

      {/* ─── C Data Structures (DSA) Matrix Modal ─── */}
      {showDsaModal && (
        <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/75 backdrop-blur-xl">
          <div className="relative w-full max-w-2xl rounded-3xl bg-[#17130f] border border-amber-400/30 p-6 md:p-8 shadow-2xl space-y-6 max-h-[90vh] overflow-y-auto">
            <div className="flex items-center justify-between border-b border-white/10 pb-4">
              <div className="flex items-center gap-2.5">
                <Cpu className="w-6 h-6 text-[#e5c583]" />
                <div>
                  <div className="flex items-center gap-2">
                    <h3 className="text-lg font-bold text-white">C Engine Data Structures Matrix</h3>
                    <span className={`text-[10px] px-2 py-0.5 rounded-full font-mono font-medium ${activeEngine === 'wasm' ? 'bg-emerald-500/20 text-emerald-400 border border-emerald-500/30' : 'bg-amber-500/20 text-amber-400 border border-amber-500/30'}`}>
                      {engineLabel}
                    </span>
                  </div>
                  <p className="text-xs text-white/50">Core algorithms compiled from engine/src/*.c via WebAssembly</p>
                </div>
              </div>
              <button
                onClick={() => setShowDsaModal(false)}
                className="p-1.5 rounded-lg text-white/50 hover:text-white hover:bg-white/10"
              >
                <X className="w-5 h-5" />
              </button>
            </div>

            <div className="grid grid-cols-1 md:grid-cols-2 gap-4 text-xs">
              <div className="p-4 rounded-2xl bg-black/40 border border-white/5 space-y-1.5">
                <div className="flex items-center justify-between">
                  <span className="font-semibold text-[#e5c583]">1. Trie (Prefix Tree)</span>
                  <span className="text-[10px] font-mono text-emerald-400">trie.c</span>
                </div>
                <p className="text-white/60">Dictionary insertion and prefix search with frequency weighting. O(L) time complexity for live autocomplete suggestions.</p>
                <div className="pt-1 text-[11px] font-mono text-white/40">Words indexed: {stats?.trieWordCount || 3263}</div>
              </div>

              <div className="p-4 rounded-2xl bg-black/40 border border-white/5 space-y-1.5">
                <div className="flex items-center justify-between">
                  <span className="font-semibold text-[#e5c583]">2. Min-Heap Priority Queue</span>
                  <span className="text-[10px] font-mono text-emerald-400">minheap.c</span>
                </div>
                <p className="text-white/60">Maintains top-K candidate rankings based on edit distance cost. Efficient O(log K) extraction for suggestions.</p>
                <div className="pt-1 text-[11px] font-mono text-white/40">Capacity: 32 candidates</div>
              </div>

              <div className="p-4 rounded-2xl bg-black/40 border border-white/5 space-y-1.5">
                <div className="flex items-center justify-between">
                  <span className="font-semibold text-[#e5c583]">3. Damerau-Levenshtein</span>
                  <span className="text-[10px] font-mono text-emerald-400">edit_distance.c</span>
                </div>
                <p className="text-white/60">Calculates true distance including insertions, deletions, substitutions, and adjacent transpositions (e.g. recieve → receive).</p>
                <div className="pt-1 text-[11px] font-mono text-white/40">DP Matrix: (M+1) x (N+1)</div>
              </div>

              <div className="p-4 rounded-2xl bg-black/40 border border-white/5 space-y-1.5">
                <div className="flex items-center justify-between">
                  <span className="font-semibold text-[#e5c583]">4. Hash Table</span>
                  <span className="text-[10px] font-mono text-emerald-400">hashtable.c</span>
                </div>
                <p className="text-white/60">O(1) dictionary word lookup and frequency counts with separate chaining for collision resolution.</p>
                <div className="pt-1 text-[11px] font-mono text-white/40">Entries: {stats?.hashTableEntries || 3263}</div>
              </div>

              <div className="p-4 rounded-2xl bg-black/40 border border-white/5 space-y-1.5">
                <div className="flex items-center justify-between">
                  <span className="font-semibold text-[#e5c583]">5. N-Gram Language Model</span>
                  <span className="text-[10px] font-mono text-emerald-400">ngram.c</span>
                </div>
                <p className="text-white/60">Unigram, bigram, and trigram conditional probabilities P(W_n | W_{'n-1'}, W_{'n-2'}) for next-word prediction.</p>
                <div className="pt-1 text-[11px] font-mono text-white/40">Bigrams: {stats?.bigramCount || 117} | Trigrams: {stats?.trigramCount || 64}</div>
              </div>

              <div className="p-4 rounded-2xl bg-black/40 border border-white/5 space-y-1.5">
                <div className="flex items-center justify-between">
                  <span className="font-semibold text-[#e5c583]">6. Stack & Linked List</span>
                  <span className="text-[10px] font-mono text-emerald-400">stack.c & linkedlist.c</span>
                </div>
                <p className="text-white/60">LIFO Stack for edit undo snapshots; dynamic Doubly Linked List for chronological session history tracking.</p>
                <div className="pt-1 text-[11px] font-mono text-white/40">Undo Stack Active • Audit Linked List Active</div>
              </div>
            </div>

            <div className="pt-2 flex justify-end">
              <button
                onClick={() => setShowDsaModal(false)}
                className="px-5 py-2 rounded-xl bg-[#8c6734] hover:bg-[#a67c40] text-white font-medium text-xs transition-colors"
              >
                Close Inspector
              </button>
            </div>
          </div>
        </div>
      )}

    </div>
  );
}
