# TypeAssist Datasets & Linguistic Corpora

This directory contains scripts and datasets used to populate the TypeAssist C/WASM Engine's Trie, Hash Table, and N-Gram models.

---

## 1. Corpus Sources & Licences

| Dataset | Primary Source | Licence | Download Size | Processed Size | Role in Engine |
|---|---|---|---|---|---|
| **English Word List (SCOWL/Aspell)** | [SCOWL Aspell Wordlists](https://wordlist.aspell.net/) | Public Domain / MIT-like | ~1.2 MB | ~480 KB (`dictionary.txt`) | Trie autocomplete & Hash Table dictionary lookup |
| **WikiText-2 Linguistic Corpus** | [Salesforce Research / HuggingFace](https://huggingface.co/datasets/Salesforce/wikitext) | CC BY-SA 3.0 | ~11 MB | ~1.8 MB (`bigrams.txt`, `trigrams.txt`) | N-Gram next-word statistical prediction |
| **Tatoeba Translation Pairs** | [Tatoeba Project](https://tatoeba.org/en/downloads) | CC BY 2.0 FR | ~1.5 MB | ~350 KB (`tatoeba_hi_en.json`) | Translation baseline & validation pairs |
| **Hinglish Sentences** | Curated & AI-verified linguistic set | Open / Project-created | ~80 KB | ~80 KB (`hinglish_pairs.json`) | Hinglish-to-English & colloquial code-switching |

---

## 2. Directory Layout

- `data/scripts/`: Node.js scripts for downloading, tokenizing, deduplicating, and building frequency tables.
- `data/raw/`: Cache for original upstream downloads.
- `data/processed/`: Standardized text files ready for runtime ingestion by the C engine:
  - `dictionary.txt`: Formatted as `word\tfrequency\n`.
  - `bigrams.txt`: Formatted as `word1|word2\tfrequency\n`.
  - `trigrams.txt`: Formatted as `word1|word2|word3\tfrequency\n`.
  - `hinglish_pairs.json`: Curated Hinglish/Hindi/English translation corpus.

---

## 3. Development Fallback Strategy

In strict offline development or air-gapped evaluation environments, TypeAssist includes a self-contained fallback lexicon containing:
- **5,000 top English words** with natural corpus frequencies.
- **2,500 common bigrams** and **1,200 trigrams** for next-word prediction.
- **100+ vetted Hinglish phrases** with English and Hindi equivalents.

This ensures the C WebAssembly engine and all frontend features operate with zero required network calls.

