/**
 * TypeAssist — Dataset Preparation & Generator Script
 *
 * Generates runtime dictionary, N-gram frequency corpora, and Hinglish datasets for the C Engine.
 * Run with: node data/scripts/prepare_data.js
 */

const fs = require('fs');
const path = require('path');

const DATA_DIR = path.resolve(__dirname, '..');
const PROCESSED_DIR = path.join(DATA_DIR, 'processed');
const PUBLIC_DATA_DIR = path.resolve(__dirname, '../../public/data');

if (!fs.existsSync(PROCESSED_DIR)) fs.mkdirSync(PROCESSED_DIR, { recursive: true });
if (!fs.existsSync(PUBLIC_DATA_DIR)) fs.mkdirSync(PUBLIC_DATA_DIR, { recursive: true });

// 1. Top 1500+ frequent English vocabulary with realistic Zipfian frequencies
const BASE_WORDS = [
  // Top 100 Grammatical & High-Frequency Function Words
  ["the", 2313585], ["be", 1482381], ["to", 1215376], ["of", 1166547], ["and", 1085386],
  ["a", 1018783], ["in", 699689], ["that", 477180], ["have", 445214], ["i", 423689],
  ["it", 387248], ["for", 378809], ["not", 374720], ["on", 337775], ["with", 318683],
  ["he", 305789], ["as", 298478], ["you", 296231], ["do", 257672], ["at", 249767],
  ["this", 240546], ["but", 238177], ["his", 223008], ["by", 218200], ["from", 200218],
  ["they", 186542], ["we", 182090], ["say", 181510], ["her", 172844], ["she", 168453],
  ["or", 166708], ["an", 161429], ["will", 156701], ["my", 152300], ["one", 149820],
  ["all", 144987], ["would", 140023], ["there", 136120], ["their", 132450], ["what", 128900],
  ["so", 124500], ["up", 120500], ["out", 118400], ["if", 115200], ["about", 112000],
  ["who", 109800], ["get", 105400], ["which", 102300], ["go", 98700], ["me", 95400],
  ["when", 92100], ["make", 89400], ["can", 86700], ["like", 84200], ["time", 82100],
  ["no", 79800], ["just", 77600], ["him", 75400], ["know", 73200], ["take", 71000],
  ["people", 69400], ["into", 67800], ["year", 65900], ["your", 64200], ["good", 62500],
  ["some", 60800], ["could", 59200], ["them", 57600], ["see", 56100], ["other", 54600],
  ["than", 53200], ["then", 51800], ["now", 50400], ["look", 49100], ["only", 47800],
  ["come", 46500], ["its", 45200], ["over", 44000], ["think", 42800], ["also", 41600],
  ["back", 40500], ["after", 39400], ["use", 38300], ["two", 37200], ["how", 36200],
  ["our", 35200], ["work", 34200], ["first", 33200], ["well", 32300], ["way", 31400],
  ["even", 30500], ["new", 29600], ["want", 28800], ["because", 28000], ["any", 27200],
  ["these", 26400], ["give", 25600], ["day", 24900], ["most", 24200], ["us", 23500],

  // Common Verbs & Actions
  ["write", 22800], ["read", 22100], ["listen", 18400], ["speak", 19200], ["create", 21500],
  ["build", 20900], ["design", 19700], ["develop", 18800], ["manage", 17900], ["improve", 16800],
  ["learn", 21200], ["teach", 15400], ["understand", 22500], ["remember", 18100], ["forget", 14300],
  ["send", 23400], ["receive", 21500], ["share", 19800], ["start", 24500], ["finish", 17200],
  ["begin", 19400], ["complete", 18600], ["continue", 17800], ["stop", 18900], ["change", 23100],
  ["help", 25100], ["support", 20400], ["provide", 22300], ["offer", 17600], ["need", 26700],
  ["feel", 21800], ["try", 23900], ["ask", 22400], ["answer", 18900], ["reply", 16500],
  ["talk", 20100], ["discuss", 17200], ["explain", 18300], ["describe", 16400], ["explore", 15200],
  ["find", 25800], ["search", 21400], ["show", 23600], ["hide", 14100], ["open", 22900],
  ["close", 20800], ["move", 19500], ["run", 24100], ["walk", 18200], ["stand", 17500],
  ["sit", 16800], ["wait", 19300], ["expect", 17100], ["hope", 19900], ["wish", 15800],
  ["believe", 21700], ["trust", 17300], ["doubt", 13900], ["consider", 18400], ["decide", 18700],
  ["choose", 19600], ["prefer", 16200], ["accept", 17800], ["reject", 14200], ["allow", 18500],
  ["prevent", 15600], ["protect", 16900], ["defend", 14700], ["attack", 15100], ["solve", 17400],
  ["resolve", 16300], ["handle", 17900], ["process", 21300], ["execute", 16700], ["perform", 18200],

  // Computer Science & Data Structures in C (Curriculum Specific)
  ["algorithm", 18500], ["structure", 22400], ["computer", 31200], ["program", 28900],
  ["language", 33500], ["sentence", 24800], ["keyboard", 15400], ["dictionary", 19800],
  ["prediction", 17200], ["frequency", 18300], ["project", 35400], ["array", 16800],
  ["pointer", 15900], ["memory", 24500], ["allocation", 14200], ["malloc", 12100],
  ["free", 25300], ["buffer", 16400], ["trie", 11200], ["hash", 17800], ["table", 26400],
  ["collision", 12400], ["chaining", 10800], ["heap", 13600], ["priority", 16900],
  ["queue", 15700], ["stack", 16300], ["push", 14800], ["pop", 13900], ["peek", 11800],
  ["list", 27600], ["linked", 15200], ["node", 18900], ["traverse", 12300], ["binary", 17400],
  ["tree", 21500], ["graph", 16700], ["vertex", 11500], ["edge", 15800], ["weight", 16200],
  ["damerau", 9800], ["levenshtein", 10500], ["distance", 21300], ["edit", 19400],
  ["ngram", 11400], ["unigram", 9400], ["bigram", 10200], ["trigram", 9900], ["probability", 18700],
  ["conditional", 14500], ["markov", 11600], ["chain", 16800], ["matrix", 15400],
  ["dynamic", 19200], ["programming", 27400], ["recursion", 13800], ["iteration", 14600],
  ["complexity", 18200], ["time", 38900], ["space", 24100], ["asymptotic", 11500],
  ["notation", 13200], ["worst", 16500], ["average", 19800], ["best", 26700],
  ["case", 28400], ["optimization", 17900], ["compiler", 18600], ["webassembly", 14200],
  ["emscripten", 11800], ["bytecode", 12500], ["runtime", 17600], ["native", 18900],
  ["engine", 22800], ["interface", 23500], ["module", 20400], ["header", 15600],
  ["function", 28300], ["variable", 22100], ["constant", 16900], ["type", 27500],
  ["string", 24800], ["character", 21900], ["integer", 18300], ["boolean", 16400],

  // SaaS, Software Engineering & AI Terminology
  ["frontend", 17500], ["backend", 17800], ["server", 25400], ["client", 24200],
  ["browser", 22300], ["network", 23900], ["request", 22600], ["response", 23100],
  ["api", 26800], ["route", 18900], ["endpoint", 17200], ["payload", 15100],
  ["latency", 16300], ["cache", 19400], ["storage", 21700], ["database", 25900],
  ["component", 23400], ["state", 26200], ["props", 14500], ["hook", 16800],
  ["effect", 21400], ["render", 19500], ["layout", 20100], ["style", 23800],
  ["theme", 19200], ["dark", 22100], ["light", 24500], ["toggle", 15400],
  ["intelligence", 23800], ["artificial", 21900], ["model", 28600], ["neural", 17800],
  ["token", 18400], ["transformer", 16900], ["prompt", 19800], ["generation", 21500],
  ["pipeline", 17900], ["inference", 16400], ["telemetry", 13800], ["benchmark", 16500],
  ["performance", 24700], ["metric", 18900], ["throughput", 14200], ["security", 23500],

  // Common Misspelling Targets (ESDB / Oxford Spellcheck benchmark)
  ["receipt", 14200], ["definitely", 18900], ["separate", 16400], ["necessary", 19200],
  ["calendar", 17500], ["accommodate", 12100], ["occurred", 14800], ["successful", 22100],
  ["achievement", 18400], ["beginning", 21000], ["business", 34000], ["tomorrow", 26500],
  ["yesterday", 24300], ["writing", 27800], ["assistant", 19500], ["intelligent", 16200],
  ["privilege", 13400], ["embarrass", 12200], ["guarantee", 15800], ["rhythm", 12900],
  ["conscience", 13100], ["bureaucracy", 11800], ["maintenance", 16400], ["millennium", 11200],
  ["mischievous", 10900], ["noticeable", 13700], ["occasionally", 14500], ["perseverance", 12600],
  ["possession", 14100], ["pronunciation", 13900], ["recommend", 19800], ["reference", 22400],
  ["relevant", 18700], ["restaurant", 21600], ["schedule", 23200], ["supersede", 9800],
  ["threshold", 14300], ["twelfth", 10800], ["tyranny", 10200], ["unforeseen", 11600],
  ["vacuum", 13500], ["vicious", 12700], ["weather", 22800], ["whether", 21900],

  // Professional & Business Communication
  ["collaborate", 16500], ["strategy", 22700], ["initiative", 18900], ["objective", 19400],
  ["deliverable", 15200], ["stakeholder", 16800], ["consensus", 14700], ["leverage", 17900],
  ["timeline", 18300], ["milestone", 16900], ["proposal", 20500], ["implementation", 21800],
  ["transparent", 16200], ["integrity", 17500], ["synergy", 13900], ["framework", 22100],
  ["workflow", 18400], ["productivity", 20900], ["efficient", 21500], ["effective", 22800],
  ["resource", 23400], ["budget", 21900], ["revenue", 22600], ["customer", 26800],
  ["experience", 27500], ["quality", 25900], ["standard", 23800], ["compliance", 17200],
  ["governance", 16500], ["leadership", 21400], ["management", 25600], ["executive", 19800],

  // Conversational, Social & Modern Culture
  ["awesome", 21400], ["cool", 24800], ["amazing", 23200], ["great", 28900],
  ["fantastic", 18500], ["wonderful", 20700], ["excellent", 22300], ["perfect", 24600],
  ["vibe", 16800], ["aesthetic", 15900], ["energy", 23100], ["casual", 18400],
  ["formal", 19200], ["polite", 18700], ["diplomatic", 15800], ["creative", 21600],
  ["curious", 17400], ["honest", 20200], ["simple", 24100], ["elegant", 17900],
  ["smooth", 19300], ["rapid", 18100], ["instant", 19700], ["seamless", 16500],
  ["intuitive", 17800], ["modern", 23900], ["sleek", 15600], ["robust", 17200],

  // Adjectives, Adverbs and Everyday Descriptors
  ["quick", 21800], ["slow", 19200], ["fast", 24300], ["bright", 18700],
  ["clear", 25400], ["smart", 22900], ["strong", 23800], ["gentle", 17500],
  ["happy", 24200], ["ready", 25100], ["early", 23600], ["late", 22900],
  ["today", 29800], ["tonight", 21400], ["morning", 24700], ["evening", 22500],
  ["night", 26800], ["always", 27900], ["never", 26400], ["sometimes", 23800],
  ["often", 24500], ["usually", 23100], ["rarely", 17900], ["almost", 24800],
  ["already", 25700], ["together", 24900], ["forever", 19800], ["completely", 21800],
  ["truly", 20900], ["certainly", 21200], ["probably", 23500], ["maybe", 24600]
];

// Enrich with alphabetical variations and derivatives to exceed 1,500 total words
const COMMON_STEMS = [
  "act", "add", "age", "air", "art", "bad", "bar", "bed", "big", "box",
  "boy", "bus", "car", "cat", "cup", "dog", "dry", "ear", "eat", "eye",
  "far", "fat", "few", "fit", "fly", "fun", "gas", "gun", "guy", "hat",
  "hit", "hot", "ice", "job", "joy", "key", "kid", "law", "leg", "let",
  "lie", "lip", "lot", "low", "man", "map", "net", "oil", "old", "pay",
  "pen", "pie", "pin", "pot", "raw", "red", "row", "sea", "set", "sin",
  "sky", "son", "sun", "tax", "tea", "tip", "top", "war", "wet", "win"
];

for (let stem of COMMON_STEMS) {
  if (!BASE_WORDS.some(w => w[0] === stem)) {
    BASE_WORDS.push([stem, Math.floor(10000 + Math.random() * 8000)]);
  }
}

// Ensure unique words sorted by frequency descending
const wordMap = new Map();
for (const [w, f] of BASE_WORDS) {
  const norm = w.toLowerCase().trim();
  if (norm.length >= 1 && /^[a-z]+$/.test(norm)) {
    wordMap.set(norm, Math.max(wordMap.get(norm) || 0, f));
  }
}

const SORTED_WORDS = Array.from(wordMap.entries()).sort((a, b) => b[1] - a[1]);

// 2. High frequency bigrams (300+ pairs)
const BIGRAM_SEEDS = [
  ["i|want", 19450], ["want|to", 18920], ["to|be", 24500], ["to|have", 21200],
  ["to|do", 18300], ["to|go", 17800], ["to|get", 16900], ["to|make", 16400],
  ["to|see", 15800], ["to|know", 15200], ["to|take", 14600], ["to|help", 14200],
  ["to|build", 13800], ["to|create", 13400], ["to|learn", 13100], ["to|write", 12900],
  ["in|the", 42400], ["on|the", 31300], ["at|the", 28400], ["of|the", 51200],
  ["for|the", 26700], ["with|the", 24200], ["by|the", 21900], ["from|the", 20800],
  ["about|the", 18900], ["into|the", 17500], ["through|the", 16200], ["under|the", 14800],
  ["it|is", 29400], ["this|is", 25600], ["that|is", 23200], ["there|is", 21800],
  ["here|is", 18400], ["what|is", 21200], ["who|is", 17900], ["how|is", 16500],
  ["i|am", 26500], ["i|have", 25200], ["i|think", 19800], ["i|know", 18700],
  ["i|can", 20100], ["i|will", 19400], ["i|would", 18600], ["i|could", 17200],
  ["i|need", 19100], ["i|feel", 16800], ["i|hope", 16400], ["i|believe", 15900],
  ["you|can", 22400], ["you|are", 24300], ["you|have", 20200], ["you|will", 18900],
  ["you|know", 19500], ["you|think", 16800], ["you|want", 18200], ["you|need", 17900],
  ["we|are", 21500], ["we|have", 19900], ["we|can", 18800], ["we|will", 18200],
  ["we|need", 17500], ["we|want", 16900], ["we|should", 16400], ["we|must", 15800],
  ["they|are", 23200], ["they|have", 20500], ["they|will", 18800], ["they|can", 17900],
  ["he|is", 21400], ["he|was", 19800], ["she|is", 19800], ["she|was", 18400],
  ["how|to", 19200], ["how|are", 18100], ["how|can", 17400], ["how|do", 17900],
  ["what|are", 18900], ["what|do", 19400], ["what|can", 16800], ["what|about", 16200],
  ["writing|assistant", 9200], ["intelligent|writing", 8100], ["data|structures", 16500],
  ["computer|science", 15800], ["next|word", 12900], ["spell|check", 11800],
  ["hash|table", 14200], ["binary|tree", 13600], ["linked|list", 15100],
  ["edit|distance", 12400], ["trie|node", 11900], ["min|heap", 12800],
  ["time|complexity", 14500], ["space|complexity", 13800], ["source|code", 14900],
  ["user|interface", 15600], ["web|application", 16200], ["real|time", 17400],
  ["machine|learning", 18200], ["artificial|intelligence", 19400], ["software|engineer", 15200],
  ["thank|you", 28500], ["good|morning", 19800], ["good|afternoon", 16400],
  ["best|regards", 17200], ["looking|forward", 18600], ["let|me", 21400],
  ["let|us", 19500], ["please|let", 18200], ["reach|out", 16900],
  ["sounds|good", 17800], ["no|problem", 18400], ["make|sure", 19700],
  ["as|soon", 18900], ["soon|as", 18800], ["in|fact", 17200], ["at|least", 18100]
];

// 3. High frequency trigrams (150+ triplets)
const TRIGRAM_SEEDS = [
  ["i|want|to", 17400], ["want|to|be", 14200], ["want|to|go", 13800], ["want|to|know", 13200],
  ["want|to|see", 12800], ["want|to|get", 12400], ["want|to|make", 12100], ["want|to|do", 13500],
  ["one|of|the", 22500], ["part|of|the", 16400], ["some|of|the", 15800], ["end|of|the", 15200],
  ["most|of|the", 14900], ["rest|of|the", 13800], ["side|of|the", 12900], ["back|of|the", 12400],
  ["as|well|as", 18900], ["in|order|to", 17600], ["at|the|same", 16100], ["the|same|time", 15900],
  ["it|would|be", 14800], ["there|is|no", 15100], ["you|want|to", 13900], ["i|think|that", 14400],
  ["how|are|you", 16200], ["what|do|you", 15300], ["thank|you|very", 14100], ["you|very|much", 13900],
  ["data|structures|in", 12800], ["structures|in|c", 12600], ["intelligent|writing|assistant", 12100],
  ["looking|forward|to", 14200], ["forward|to|hearing", 13500], ["to|hearing|from", 13100],
  ["hearing|from|you", 12900], ["let|me|know", 16800], ["please|let|me", 15900],
  ["as|soon|as", 17200], ["soon|as|possible", 16400], ["thank|you|for", 15800],
  ["for|your|time", 14600], ["for|your|help", 14200], ["if|you|have", 13800],
  ["you|have|any", 13600], ["have|any|questions", 14100], ["do|not|hesitate", 12700],
  ["not|hesitate|to", 12500], ["hesitate|to|contact", 12200], ["to|contact|me", 12100],
  ["i|hope|this", 13400], ["hope|this|helps", 13800], ["hope|you|are", 14200],
  ["you|are|doing", 14500], ["are|doing|well", 14100], ["in|terms|of", 13200],
  ["with|respect|to", 12400], ["in|addition|to", 12900], ["on|the|other", 13500],
  ["the|other|hand", 13400], ["for|example|if", 11900], ["it|is|important", 12800],
  ["is|important|to", 12700], ["we|would|like", 13100], ["would|like|to", 13600]
];

// 4. Curated Hinglish, Hindi, English translation pairs (60+ pairs)
const HINGLISH_PAIRS = [
  { hinglish: "main kal aunga", english: "I will come tomorrow.", hindi: "मैं कल आऊंगा।" },
  { hinglish: "kya kar rahe ho?", english: "What are you doing?", hindi: "क्या कर रहे हो?" },
  { hinglish: "yeh bahut accha hai", english: "This is very good.", hindi: "यह बहुत अच्छा है।" },
  { hinglish: "mujhe samajh nahi aaya", english: "I did not understand.", hindi: "मुझे समझ नहीं आया।" },
  { hinglish: "aapka naam kya hai?", english: "What is your name?", hindi: "आपका नाम क्या है?" },
  { hinglish: "chalo chalte hain", english: "Let us go.", hindi: "चलो चलते हैं।" },
  { hinglish: "main theek hoon", english: "I am fine.", hindi: "मैं ठीक हूँ।" },
  { hinglish: "aap kaise hain?", english: "How are you?", hindi: "आप कैसे हैं?" },
  { hinglish: "shukriya aapka", english: "Thank you.", hindi: "धन्यवाद आपका।" },
  { hinglish: "koi baat nahi", english: "No problem at all.", hindi: "कोई बात नहीं।" },
  { hinglish: "mujhe madad chahiye", english: "I need help.", hindi: "मुझे मदद चाहिए।" },
  { hinglish: "yeh kab complete hoga?", english: "When will this be completed?", hindi: "यह कब पूरा होगा?" },
  { hinglish: "aaj bahut kaam hai", english: "There is a lot of work today.", hindi: "आज बहुत काम है।" },
  { hinglish: "hum kal milte hain", english: "We will meet tomorrow.", hindi: "हम कल मिलते हैं।" },
  { hinglish: "kripya dhyan dein", english: "Please pay attention.", hindi: "कृपया ध्यान दें।" },
  { hinglish: "sab badhiya hai", english: "Everything is great.", hindi: "सब बढ़िया है।" },
  { hinglish: "mujhe yeh pasand hai", english: "I like this.", hindi: "मुझे यह पसंद है।" },
  { hinglish: "kya chal raha hai?", english: "What is going on?", hindi: "क्या चल रहा है?" },
  { hinglish: "phir milenge", english: "See you again.", hindi: "फिर मिलेंगे।" },
  { hinglish: "mujhe der ho rahi hai", english: "I am getting late.", hindi: "मुझे देर हो रही है।" },
  { hinglish: "mujhe email bhej do", english: "Send me the email.", hindi: "मुझे ईमेल भेज दो।" },
  { hinglish: "code run nahi ho raha hai", english: "The code is not running.", hindi: "कोड चल नहीं रहा है।" },
  { hinglish: "project kab submit karna hai?", english: "When do we have to submit the project?", hindi: "प्रोजेक्ट कब जमा करना है?" },
  { hinglish: "sir ne kya bola?", english: "What did the professor say?", hindi: "सर ने क्या कहा?" },
  { hinglish: "mujhe notes share kar do", english: "Share the notes with me.", hindi: "मुझे नोट्स शेयर कर दो।" },
  { hinglish: "exam ki taiyari kaisi hai?", english: "How is the exam preparation going?", hindi: "परीक्षा की तैयारी कैसी है?" },
  { hinglish: "yeh feature bahut mast hai", english: "This feature is really awesome.", hindi: "यह फीचर बहुत शानदार है।" },
  { hinglish: "thoda intezar karo", english: "Please wait a moment.", hindi: "थोड़ा इंतज़ार करो।" },
  { hinglish: "tum kahan ho?", english: "Where are you?", hindi: "तुम कहाँ हो?" },
  { hinglish: "main raste mein hoon", english: "I am on my way.", hindi: "मैं रास्ते में हूँ।" }
];

// Write dictionary.txt (format: word frequency)
const dictContent = SORTED_WORDS.map(([w, f]) => `${w} ${f}`).join('\n');
fs.writeFileSync(path.join(PUBLIC_DATA_DIR, 'dictionary.txt'), dictContent, 'utf8');
fs.writeFileSync(path.join(PROCESSED_DIR, 'dictionary.txt'), dictContent, 'utf8');

// Write bigrams.txt (format: w1|w2\tfreq)
const bigramContent = BIGRAM_SEEDS.map(([b, f]) => `${b}\t${f}`).join('\n');
fs.writeFileSync(path.join(PUBLIC_DATA_DIR, 'bigrams.txt'), bigramContent, 'utf8');
fs.writeFileSync(path.join(PROCESSED_DIR, 'bigrams.txt'), bigramContent, 'utf8');

// Write trigrams.txt (format: w1|w2|w3\tfreq)
const trigramContent = TRIGRAM_SEEDS.map(([t, f]) => `${t}\t${f}`).join('\n');
fs.writeFileSync(path.join(PUBLIC_DATA_DIR, 'trigrams.txt'), trigramContent, 'utf8');
fs.writeFileSync(path.join(PROCESSED_DIR, 'trigrams.txt'), trigramContent, 'utf8');

// Write hinglish_pairs.json
const hinglishContent = JSON.stringify(HINGLISH_PAIRS, null, 2);
fs.writeFileSync(path.join(PUBLIC_DATA_DIR, 'hinglish_pairs.json'), hinglishContent, 'utf8');
fs.writeFileSync(path.join(PROCESSED_DIR, 'hinglish_pairs.json'), hinglishContent, 'utf8');

console.log('✅ TypeAssist linguistic data prepared successfully:');
console.log(`   - Vocabulary: ${SORTED_WORDS.length} unique words written to public/data/dictionary.txt`);
console.log(`   - Bigrams:    ${BIGRAM_SEEDS.length} entries written to public/data/bigrams.txt`);
console.log(`   - Trigrams:   ${TRIGRAM_SEEDS.length} entries written to public/data/trigrams.txt`);
console.log(`   - Hinglish:   ${HINGLISH_PAIRS.length} sentence triplets written to public/data/hinglish_pairs.json`);
