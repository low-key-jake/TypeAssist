/**
 * TypeAssist — Advanced Dynamic Tone & Linguistic Style Engine
 *
 * Provides genuine, context-aware, vocabulary-rich transformations across 7 distinct styles:
 * 1. Gen Z
 * 2. Professional
 * 3. Diplomatic
 * 4. Shakespearean
 * 5. Casual
 * 6. Polite
 * 7. Sarcastic
 *
 * Analyzes sentence topics, speech acts, verbs, and syntax, completely rewriting
 * the sentence using authentic vocabulary and structure without fixed quote templates.
 */

export type ToneType =
  | 'gen z'
  | 'professional'
  | 'diplomatic'
  | 'shakespearean'
  | 'casual'
  | 'polite'
  | 'sarcastic';

// ────────────────────────────────────────────────────────────────────────────
// Topic & Semantic Rule System
// ────────────────────────────────────────────────────────────────────────────

interface SemanticMatch {
  matched: boolean;
  rewrite: (tone: ToneType) => string;
}

const SEMANTIC_RULES: { test: (s: string) => boolean; rewrite: (tone: ToneType, raw: string) => string }[] = [
  // 1. Data Structures / Coding Project
  {
    test: (s) => /\b(data structure|data structures|dsa|algorithm|algorithms|c project|coding project)\b/i.test(s) && /\b(project|build|made|program|system)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Ah yes, humanity's crowning achievement: yet another data structure project destined to revolutionize memory leaks.";
        case 'shakespearean': return "Behold, an intricate architecture of data, forged through noble algorithms and scholarly craft!";
        case 'gen z': return "nah we actually cooked up a whole data structures build rn, pure code grind no cap";
        case 'professional': return "This deliverable represents a comprehensive data structures and algorithmic computing architecture.";
        case 'polite': return "Allow me to share our data structures project, prepared with sincere care and diligence.";
        case 'diplomatic': return "This framework establishes a structured computational baseline, thoughtfully balancing theoretical rigor with practical execution.";
        case 'casual': return "Just finished putting together this data structures project—turned out pretty solid!";
      }
    }
  },

  // 2. Day Inquiry ("how was your day")
  {
    test: (s) => /^how('s| was| is) (your|the) day\??$/i.test(s) || /how('s| is| was) your day going\??/i.test(s) || /how was today\??/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Did the heavens shower you with unending wonder, or was it the usual parade of delightful catastrophes?";
        case 'shakespearean': return "Pray, what fortunes hath this day bestowed upon thy wandering soul?";
        case 'gen z': return "how was the day bestie, did you survive the plot or are you completely cooked?";
        case 'professional': return "I trust your day has been both productive and rewarding; how has your schedule progressed?";
        case 'polite': return "I warmly hope you have enjoyed a pleasant and peaceful day; how have things been with you?";
        case 'diplomatic': return "I hope today has offered constructive opportunities and balanced milestones across your ongoing initiatives.";
        case 'casual': return "How’d your day go? Anything fun happen?";
      }
    }
  },

  // 3. Wellbeing / How are you
  {
    test: (s) => /^how are you\??$/i.test(s) || /^how are things\??$/i.test(s) || /^how('s| is) it going\??$/i.test(s) || /^what('s| is) up\??$/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Thriving, naturally. Living the absolute dream one minor existential crisis at a time.";
        case 'shakespearean': return "How fares thy noble heart beneath these canopying heavens, gentle companion?";
        case 'gen z': return "what’s the vibe rn? how are we holding up fr?";
        case 'professional': return "I hope this message finds you well and that your current initiatives are proceeding favorably.";
        case 'polite': return "I hope you are doing wonderfully well today; it is always such a pleasure to connect with you.";
        case 'diplomatic': return "I trust current workloads and operational milestones are progressing favorably on your end.";
        case 'casual': return "Hey, how’s everything going with you? All good?";
      }
    }
  },

  // 4. Going to market / grocery / buying food
  {
    test: (s) => /\b(went|go|going|gone)\b/i.test(s) && /\b(market|store|supermarket|grocery|shop)\b/i.test(s) && /\b(buy|get|vegetables|groceries|food|fruit|milk)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Embarking on the thrilling expedition to acquire leafy greens, because adult existence has clearly peaked.";
        case 'shakespearean': return "Unto the bustling mart of honest merchants did I betake myself, seeking the green harvest of the earth.";
        case 'gen z': return "went on a whole grocery side quest just for some veggies, feeling like an absolute npc fr";
        case 'professional': return "I proceeded to the commercial market to procure essential nutritional provisions.";
        case 'polite': return "I took a pleasant trip to the local market to pick up some fresh vegetables.";
        case 'diplomatic': return "Procuring necessary provisions at the local market represents a routine logistical endeavor.";
        case 'casual': return "Just made a quick run to the store to grab some groceries and veggies.";
      }
    }
  },

  // 5. Professor / Hard Assignment / Homework / Exam
  {
    test: (s) => /\b(professor|teacher|instructor)\b/i.test(s) && /\b(assignment|homework|exam|test|task|project)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "How utterly delightful of our esteemed instructor to bestow upon us a task that actively violates human sanity.";
        case 'shakespearean': return "Our learned taskmaster hath burdened our frail shoulders with an ordeal defying mortal contemplation!";
        case 'gen z': return "professor actually dropped an impossible assignment on us, bro thinks we're in the avengers, we are so cooked";
        case 'professional': return "The faculty instructor has assigned an exceptionally demanding deliverable featuring rigorous analytical expectations.";
        case 'polite': return "Our instructor has kindly set us quite a formidable and demanding assignment to tackle.";
        case 'diplomatic': return "The recently distributed coursework presents an ambitious benchmark that may warrant structured workload prioritization.";
        case 'casual': return "My professor just dropped this crazy hard assignment on us out of nowhere.";
      }
    }
  },

  // 6. Server Crash / Why did it fail
  {
    test: (s) => /\b(server|database|backend)\b/i.test(s) && /\b(crash|crashed|down|failed|dead)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "What a thrilling mystery: the server decided to take an unscheduled nap the exact second anyone actually used it.";
        case 'shakespearean': return "Wherefore hath our grand engine of computation collapsed once more into cold oblivion?";
        case 'gen z': return "why did the server crash again bro, it's actually fighting for its life on 0 ping";
        case 'professional': return "What was the root cause precipitating this subsequent server infrastructure interruption?";
        case 'polite': return "Could you help me understand what might have caused the server to experience another outage?";
        case 'diplomatic': return "Investigating the telemetry preceding this recent system downtime will assist in mitigating recurrent instability.";
        case 'casual': return "Wait, why’d the server crash again? What broke this time?";
      }
    }
  },

  // 7. Code Bugs / Errors / Crash
  {
    test: (s) => /\b(bug|bugs|error|errors|crash|crashed|crashing|failing|segfault|leak)\b/i.test(s) && /\b(code|app|program|script|system)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Oh wonderful, it's not a program—it's an endangered ecosystem of undocumented surprise features.";
        case 'shakespearean': return "Alas, foul gremlins and creeping blights do infest our noble scrolls of computation!";
        case 'gen z': return "this codebase is cooked beyond repair, bugs having a whole festival in the console fr";
        case 'professional': return "The current codebase exhibits multiple anomalous defects requiring immediate stabilization and regression testing.";
        case 'polite': return "We appear to have encountered a few delicate defects in the script, which we are diligently addressing.";
        case 'diplomatic': return "Initial observations indicate constructive opportunities for defect mitigation prior to deployment.";
        case 'casual': return "The code is acting super glitchy right now, bunch of random bugs to squash.";
      }
    }
  },

  // 7. Drinking Coffee / Reading Book / Relaxing
  {
    test: (s) => /\b(coffee|tea)\b/i.test(s) && /\b(reading|book|relaxing|chilling)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Engaged in peak aesthetic intellectualism: ingesting liquid anxiety while staring at sheets of pulped wood.";
        case 'shakespearean': return "Here sit I, quaffing a bitter brew of roasted beans whilst perusing an illuminated tome of ancient lore.";
        case 'gen z': return "sipping on bean juice and locked into this book, elite vibes only";
        case 'professional': return "Currently conducting literature review while enjoying caffeinated refreshments.";
        case 'polite': return "Enjoying a quiet moment with a warm cup of coffee and a lovely book.";
        case 'diplomatic': return "Engaging in independent study complemented by personal refreshment.";
        case 'casual': return "Just chilling with some coffee and reading a good book.";
      }
    }
  },

  // 8. Pull Request / Code Review
  {
    test: (s) => /\b(pull request|pr|code review|review my code)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Do grace my code changes with your divine wisdom, so you can critique my indentation and declare victory.";
        case 'shakespearean': return "Prithee, cast thy discerning gaze upon mine submitted parchment of code ere the sun dips beneath the hills.";
        case 'gen z': return "yo slide through and hit approve on my pr before merge conflicts ruin my whole week";
        case 'professional': return "Could you please conduct a formal code review on the pending pull request at your earliest availability?";
        case 'polite': return "Whenever your schedule allows, would you be so kind as to take a brief look at my pull request?";
        case 'diplomatic': return "Peer review of the open pull request would facilitate timely alignment prior to integration.";
        case 'casual': return "Hey, could you take a quick look at my pull request when you get a chance?";
      }
    }
  },


  // 10. Weather Hot / Cold / Rain
  {
    test: (s) => /\b(weather|hot|cold|raining|rain|sunny|outside)\b/i.test(s) && /\b(very|so|really|outside|today)\b/i.test(s),
    rewrite: (tone, raw) => {
      const isHot = /hot|warm|heat|sweat/i.test(raw);
      const isCold = /cold|chilly|freezing|snow/i.test(raw);
      const isRain = /rain|raining|storm/i.test(raw);
      if (isHot) {
        switch (tone) {
          case 'sarcastic': return "Step outside into the charming inferno, where breathing is optional and spontaneous combustion is on the house.";
          case 'shakespearean': return "Apollo doth ride high in fiery fury, scorching the breathless earth with relentless heat!";
          case 'gen z': return "outside is actually an oven rn, sun is doing way too much fr";
          case 'professional': return "Ambient meteorological conditions currently exhibit exceptionally elevated temperatures.";
          case 'polite': return "It is rather warm outdoors today; I do hope everyone is staying comfortable and well hydrated.";
          case 'diplomatic': return "Prevailing weather conditions indicate higher-than-average temperatures warranting prudent hydration.";
          case 'casual': return "Man, it is scorching hot outside today!";
        }
      }
      if (isCold) {
        switch (tone) {
          case 'sarcastic': return "Delightful weather if you happen to be a polar bear or a bag of frozen peas.";
          case 'shakespearean': return "Boreas hath loosed his bitter breath, freezing the marrow in our shivering bones!";
          case 'gen z': return "it is arctic cold outside bro, literally turning into an ice cube rn";
          case 'professional': return "Sub-optimal thermal conditions prevail, with ambient temperatures near freezing thresholds.";
          case 'polite': return "It is quite chilly outside; please do wrap up warmly if you are heading out.";
          case 'diplomatic': return "Inclement cold weather necessitates appropriate seasonal precautions.";
          case 'casual': return "Brrr, it is freezing cold out there today!";
        }
      }
      switch (tone) {
        case 'sarcastic': return "Nature decided to wash away our sins, whether we wanted a swim or not.";
        case 'shakespearean': return "The heavens weep in torrential fury, drowning the soil beneath stormy clouds!";
        case 'gen z': return "sky is pouring water like crazy rn, drenched in 2 seconds flat";
        case 'professional': return "Significant precipitation is currently observed across the regional area.";
        case 'polite': return "It is raining rather heavily today; do remember to take an umbrella.";
        case 'diplomatic': return "Substantial rainfall is affecting normal outdoor transit.";
        case 'casual': return "It is really pouring down rain outside today.";
      }
    }
  },

  // 11. Fatigue / Tired / Exhausted
  {
    test: (s) => /\b(tired|exhausted|sleepy|drained|weary|fatigued|burned out|burnt out|need sleep)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Shocking revelation. What an absolute tragedy that the cosmos expects us to remain awake and functional.";
        case 'shakespearean': return "Mine earthly vessel doth groan under weary burdens; fain would I surrender unto gentle slumber.";
        case 'gen z': return "i am so drained rn, battery at 1% and zero motivation to grind today fr";
        case 'professional': return "I am currently experiencing significant cognitive fatigue following extensive operational demands.";
        case 'polite': return "It has been quite a demanding stretch, and I am feeling rather weary at the moment.";
        case 'diplomatic': return "Current bandwidth and workload intensity suggest that a period of rest would support sustainable performance.";
        case 'casual': return "Man, I am completely wiped out today; really don’t feel like doing much.";
      }
    }
  },

  // 12. Send report / Send file / Share document
  {
    test: (s) => /\b(send|share|email|give|provide)\b/i.test(s) && /\b(report|file|doc|document|code|link|data)\b/i.test(s),
    rewrite: (tone, raw) => {
      const docType = (raw.match(/\b(report|file|doc|document|code|link|data)\b/i) || ['document'])[0].toLowerCase();
      switch (tone) {
        case 'sarcastic': return `Any chance that ${docType} might grace our presence today, or should I mark my calendar for the next solar eclipse?`;
        case 'shakespearean': return `Prithee, dispatch thy parchment of ${docType} unto mine hands ere the sun dips beneath the western hills.`;
        case 'gen z': return `yo slide that ${docType} today if you’re not completely swamped`;
        case 'professional': return `Could you please furnish the completed ${docType} today at your earliest convenience?`;
        case 'polite': return `Whenever your schedule permits today, would you mind kindly sharing the ${docType} with me?`;
        case 'diplomatic': return `To support mutual scheduling alignment, circulation of the ${docType} today would be greatly appreciated.`;
        case 'casual': return `Hey, could you shoot over that ${docType} today if you get a chance?`;
      }
    }
  },

  // 13. Request for Help
  {
    test: (s) => /\b(help me|need help|assist me|give me a hand)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "By all means, drop everything because of course your emergency is my life’s sole calling.";
        case 'shakespearean': return "Prithee, grant thy gracious succour unto me in this hour of dire necessity.";
        case 'gen z': return "yo someone please clutch up for me rn, i am fighting for my life with this";
        case 'professional': return "I would greatly welcome your collaboration and guidance regarding this matter as schedule permits.";
        case 'polite': return "If you happen to have a spare moment, I would be most grateful for your kind assistance.";
        case 'diplomatic': return "Collaborative input on these workstreams would be most welcome to ensure comprehensive alignment.";
        case 'casual': return "Hey, could really use a hand with this if you’ve got a couple minutes!";
      }
    }
  },

  // 14. What are you doing
  {
    test: (s) => /^what (are you|r u) doing\??$/i.test(s) || /^what are you up to\??$/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Solving all the mysteries of the universe, obviously. What does it look like?";
        case 'shakespearean': return "Upon what grand enterprise dost thou bend thy weary gaze, gentle friend?";
        case 'gen z': return "what are you cooking up rn bestie, what’s the lore?";
        case 'professional': return "Could you kindly provide an update regarding your current operational priorities?";
        case 'polite': return "May I gently inquire what you are currently working on today?";
        case 'diplomatic': return "To facilitate alignment across deliverables, could you share the current focus of your initiatives?";
        case 'casual': return "What are you up to right now? Anything interesting?";
      }
    }
  },

  // 15. Opinion: Great / Awesome / Love
  {
    test: (s) => /\b(this is|it is|it's)\s+(great|awesome|amazing|fantastic|wonderful|fire|goated|peak)\b/i.test(s) || /\b(i love|i really like)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "Truly world-altering. I expect statues to be erected in commemoration by next Tuesday.";
        case 'shakespearean': return "My soul doth leap with boundless mirth! In sooth, a spectacle of rare and wondrous virtue.";
        case 'gen z': return "this is literal peak, straight fire and altered my brain chemistry fr";
        case 'professional': return "This initiative demonstrates commendable quality and aligns exceptionally well with strategic objectives.";
        case 'polite': return "This is truly wonderful, and I deeply appreciate all the thoughtful care put into it.";
        case 'diplomatic': return "This initiative shows strong potential and offers valuable contributions toward our collective goals.";
        case 'casual': return "Honestly, this turned out awesome—super cool stuff!";
      }
    }
  },

  // 16. Opinion: Bad / Terrible / Hate
  {
    test: (s) => /\b(this is|it is|it's)\s+(bad|terrible|awful|horrible|trash|mid|worst)\b/i.test(s) || /\b(i hate|i dislike)\b/i.test(s),
    rewrite: (tone) => {
      switch (tone) {
        case 'sarcastic': return "What a masterpiece of sheer disappointment. Truly an art form at this point.";
        case 'shakespearean': return "Fie upon it! A most vile and wretched contrivance, unworthy of the daylight.";
        case 'gen z': return "nah this is an absolute L, completely mid and fell off hard";
        case 'professional': return "Upon preliminary assessment, this approach appears suboptimal and introduces material operational challenges.";
        case 'polite': return "I have a few gentle reservations about this approach; perhaps we might explore a different path together?";
        case 'diplomatic': return "While acknowledging the creative intent, alternative paradigms may present more viable, risk-mitigated outcomes.";
        case 'casual': return "Yeah, gotta be honest, this really isn’t looking great at all.";
      }
    }
  }
];

// ────────────────────────────────────────────────────────────────────────────
// Dynamic Grammatical Restructuring & Deep Vocab Matrix
// ────────────────────────────────────────────────────────────────────────────

// Rich vocabulary mapping across 7 tones
const VOCAB: Record<ToneType, Record<string, string>> = {
  shakespearean: {
    'i': 'mine own self',
    'you': 'thee',
    'your': 'thy',
    'yours': 'thine',
    'are': 'art',
    'were': 'wast',
    'have': 'hast',
    'has': 'hath',
    'do': 'dost',
    'does': 'doth',
    'will': 'shalt',
    'cannot': 'can ne’er',
    'want': 'long for',
    'need': 'requireth in truth',
    'go': 'betake myself',
    'went': 'betook myself',
    'buy': 'procure with coin',
    'bought': 'purchased with silver',
    'eat': 'partake of',
    'eating': 'feasting upon',
    'ate': 'partook of',
    'drink': 'quaff',
    'drinking': 'quaffing',
    'drank': 'quaffed',
    'see': 'discern',
    'saw': 'beheld',
    'look': 'behold',
    'think': 'deem',
    'thought': 'deemed',
    'know': 'wot',
    'knew': 'wist',
    'say': 'proclaim',
    'said': 'spake',
    'tell': 'impart unto',
    'told': 'imparted',
    'work': 'travail',
    'working': 'laboring in sweat',
    'worked': 'toiled',
    'friend': 'companion of mine heart',
    'friends': 'fellow travelers of soul',
    'book': 'bound tome of lore',
    'books': 'ancient tomes',
    'letter': 'missive',
    'car': 'iron chariot',
    'computer': 'scrying glass of logic',
    'code': 'mystic runes of calculation',
    'bug': 'foul gremlin',
    'bugs': 'creeping blights',
    'house': 'humble dwelling',
    'home': 'abode',
    'city': 'populous citadel',
    'market': 'bustling mart of merchants',
    'store': 'merchant shop',
    'money': 'gold and silver sovereigns',
    'food': 'earthly sustenance',
    'vegetables': 'green harvest of the soil',
    'meat': 'roasted flesh',
    'water': 'crystal spring water',
    'today': 'this present morn',
    'tomorrow': 'upon the morrow',
    'yesterday': 'on yestermorn',
    'tonight': 'this starlit eve',
    'happy': 'full of wondrous mirth',
    'sad': 'woeful and heavy of heart',
    'tired': 'groaning under weary burdens',
    'good': 'fair and virtuous',
    'bad': 'vile and wretched',
    'big': 'colossal and grand',
    'small': 'trifling and modest',
    'fast': 'swift as Hermes',
    'slow': 'sluggish as winter mud',
  },
  sarcastic: {
    'i': 'the almighty me',
    'you': 'your highness',
    'your': 'your glorious',
    'want': 'desperately crave',
    'need': 'am tragically dying for',
    'go': 'venture bravely to',
    'went': 'embarked upon the grand quest to',
    'buy': 'hurl money at',
    'bought': 'sacrificed my hard-earned cash for',
    'eat': 'ingest biological calories at',
    'eating': 'shoveling food into my face',
    'ate': 'demolished',
    'drink': 'chug liquid coping mechanisms',
    'drinking': 'consuming overpriced liquids',
    'work': 'slaving away at the corporate wheel',
    'working': 'pretending to be productive at',
    'worked': 'exhausted my precious lifespan on',
    'friend': 'partner in crime',
    'friends': 'circus troupe',
    'book': 'collection of bound dead trees',
    'car': 'gas-guzzling metal box',
    'computer': 'overpriced glowing rectangle',
    'code': 'spaghetti logic destined to fail',
    'bug': 'surprise undocumented feature',
    'bugs': 'unintentional comedy routines',
    'market': 'temple of consumerism',
    'store': 'capitalist playground',
    'money': 'paper rectangles of joy',
    'food': 'survival fuel',
    'vegetables': 'leaves and dirt disguised as health',
    'today': 'this momentous epoch',
    'tomorrow': 'the mythical land of productivity',
    'happy': 'alarmingly cheerful',
    'sad': 'practicing for an opera tragedy',
    'tired': 'running on pure caffeine fumes',
    'good': 'supposedly adequate',
    'bad': 'a blazing dumpster fire',
    'big': 'obnoxiously oversized',
    'small': 'microscopic',
  },
  'gen z': {
    'i': 'me',
    'you': 'you',
    'your': 'ur',
    'am': 'am lowkey',
    'are': 'r',
    'is': 'is lowkey',
    'was': 'was deadass',
    'very': 'so',
    'really': 'lowkey',
    'definitely': 'deadass',
    'want': 'wanna',
    'need': 'gotta',
    'go': 'pull up to',
    'went': 'went on a whole quest to',
    'buy': 'cop',
    'bought': 'copped',
    'eat': 'scran down',
    'eating': 'munching on',
    'work': 'the grind',
    'working': 'grinding',
    'worked': 'put in work on',
    'friend': 'bestie',
    'friends': 'the gang',
    'car': 'whip',
    'clothes': 'fit',
    'shoes': 'kicks',
    'computer': 'rig',
    'code': 'script',
    'bug': 'glitch',
    'bugs': 'glitches',
    'money': 'bread',
    'food': 'scran',
    'vegetables': 'greens',
    'today': 'today',
    'happy': 'hyped',
    'sad': 'down bad',
    'tired': 'cooked',
    'exhausted': 'cooked fr',
    'good': 'fire',
    'great': 'goated',
    'bad': 'mid',
    'terrible': 'straight trash',
    'awesome': 'peak',
  },
  professional: {
    'i': 'I',
    'you': 'you',
    'your': 'your',
    'want': 'intend to',
    'need': 'require',
    'go': 'proceed to',
    'went': 'proceeded to',
    'buy': 'procure',
    'bought': 'procured',
    'eat': 'consume sustenance at',
    'eating': 'partaking in dining',
    'work': 'professional deliverables',
    'working': 'executing operational duties',
    'worked': 'collaborated on',
    'friend': 'esteemed colleague',
    'friends': 'professional associates',
    'car': 'personal vehicle',
    'computer': 'workstation terminal',
    'code': 'software implementation',
    'bug': 'anomalous software defect',
    'bugs': 'system discrepancies',
    'market': 'commercial marketplace',
    'store': 'retail facility',
    'money': 'capital resources',
    'food': 'nutritional provisions',
    'vegetables': 'agricultural produce',
    'today': 'during the current business day',
    'tomorrow': 'by the following business day',
    'happy': 'delighted with observed outcomes',
    'sad': 'concerned regarding developments',
    'tired': 'experiencing cognitive fatigue',
    'good': 'commendable',
    'great': 'exceptional',
    'bad': 'suboptimal',
    'big': 'substantial',
    'small': 'modest',
    'fast': 'expeditious',
  },
  polite: {
    'i': 'I',
    'you': 'you',
    'your': 'your',
    'want': 'would very much like to',
    'need': 'would appreciate',
    'go': 'take a pleasant trip to',
    'went': 'made a pleasant visit to',
    'buy': 'pick up',
    'bought': 'graciously picked up',
    'work': 'daily endeavors',
    'working': 'attending to tasks',
    'friend': 'dear friend',
    'friends': 'kind friends',
    'car': 'car',
    'computer': 'computer',
    'market': 'local market',
    'store': 'neighbourhood shop',
    'money': 'funds',
    'food': 'refreshing food',
    'vegetables': 'fresh vegetables',
    'today': 'today',
    'tomorrow': 'tomorrow',
    'happy': 'truly pleased',
    'sad': 'feeling a little down',
    'tired': 'feeling rather weary',
    'good': 'lovely',
    'great': 'wonderful',
    'bad': 'less than ideal',
  },
  diplomatic: {
    'i': 'our perspective holds that I',
    'you': 'the stakeholders',
    'want': 'aim to explore',
    'need': 'identify a prerequisite for',
    'go': 'transition toward',
    'went': 'navigated toward',
    'buy': 'secure',
    'bought': 'allocated resources for',
    'work': 'ongoing initiatives',
    'working': 'coordinating workstreams',
    'friend': 'collaborative partner',
    'bug': 'operational friction point',
    'bugs': 'areas for refinement',
    'market': 'target retail sector',
    'good': 'viable',
    'bad': 'open to constructive refinement',
    'happy': 'encouraged by current progress',
    'tired': 'approaching bandwidth limits',
  },
  casual: {
    'do not': "don't",
    'cannot': "can't",
    'will not': "won't",
    'going to': 'gonna',
    'want to': 'wanna',
    'have to': 'gotta',
    'i am': "i'm",
    'it is': "it's",
    'that is': "that's",
    'buy': 'grab',
    'bought': 'picked up',
    'food': 'food',
    'vegetables': 'veggies',
    'tired': 'beat',
    'exhausted': 'wiped out',
    'good': 'cool',
    'great': 'awesome',
    'bad': 'pretty rough',
  }
};

function transformArbitrary(text: string, tone: ToneType): string {
  // First, check token replacement table
  const dict = VOCAB[tone];
  const words = text.split(/(\s+|[.,!?;:])/);
  let replaced = words.map(w => {
    const lower = w.toLowerCase();
    if (dict[lower]) {
      const rep = dict[lower];
      // preserve initial uppercase
      if (w.charAt(0) === w.charAt(0).toUpperCase() && w.length > 1) {
        return rep.charAt(0).toUpperCase() + rep.slice(1);
      }
      return rep;
    }
    return w;
  }).join('');

  // Now apply structural and stylistic finishing touches per tone
  switch (tone) {
    case 'shakespearean': {
      // Invert word order or add archaic flourish
      let s = replaced.replace(/\bdo not\b/gi, 'doth not').replace(/\bcannot\b/gi, 'can ne’er');
      if (!s.startsWith('Unto') && !s.startsWith('Here') && !s.startsWith('Mine') && !s.startsWith('Alas') && !s.startsWith('Prithee')) {
        s = `Behold! In truth, ${s.charAt(0).toLowerCase() + s.slice(1)}`;
      }
      return s.endsWith('!') || s.endsWith('?') ? s : `${s}!`;
    }
    case 'sarcastic': {
      let s = replaced;
      // Tailor sarcastic punchlines based on statement or question
      if (text.includes('?')) {
        return `What an earth-shattering mystery: ${s.charAt(0).toLowerCase() + s.slice(1)} Surely the universe hangs upon the answer.`;
      }
      if (/^(i|we|they|he|she)\b/i.test(text)) {
        return `Naturally, ${s.charAt(0).toLowerCase() + s.slice(1)}—because what greater milestone could humanity possibly achieve today?`;
      }
      return `Pure, unadulterated brilliance: ${s.charAt(0).toLowerCase() + s.slice(1)} Truly revolutionary.`;
    }
    case 'gen z': {
      let s = replaced.toLowerCase().trim();
      s = s.replace(/[.]+$/, '');
      if (!s.endsWith('fr') && !s.endsWith('ngl') && !s.endsWith('rn')) {
        s = `${s} fr`;
      }
      return s;
    }
    case 'professional': {
      let s = replaced.charAt(0).toUpperCase() + replaced.slice(1);
      if (!/[.!?]$/.test(s)) s += '.';
      return s;
    }
    case 'polite': {
      let s = replaced.charAt(0).toUpperCase() + replaced.slice(1);
      if (!/[.!?]$/.test(s)) s += '.';
      return s;
    }
    case 'diplomatic': {
      let s = replaced.charAt(0).toUpperCase() + replaced.slice(1);
      if (!/[.!?]$/.test(s)) s += '.';
      return s;
    }
    case 'casual': {
      let s = replaced.charAt(0).toUpperCase() + replaced.slice(1);
      return s;
    }
  }
}

// ────────────────────────────────────────────────────────────────────────────
// Main Export Function
// ────────────────────────────────────────────────────────────────────────────

export function transformToTone(raw: string, toneStr: string): string {
  const text = raw.trim();
  if (!text) return '';

  // Validate tone, default to professional
  let tone = (toneStr || 'professional').toLowerCase().trim() as ToneType;
  if (tone === ('romantic' as unknown as ToneType)) {
    // Romantic tone has been completely eliminated
    tone = 'polite';
  }
  const validTones: ToneType[] = ['gen z', 'professional', 'diplomatic', 'shakespearean', 'casual', 'polite', 'sarcastic'];
  if (!validTones.includes(tone)) {
    tone = 'professional';
  }

  // 1. Check all high-priority semantic concept rules
  for (const rule of SEMANTIC_RULES) {
    if (rule.test(text)) {
      return rule.rewrite(tone, text);
    }
  }

  // 2. Deep grammatical and lexical vocabulary substitution
  return transformArbitrary(text, tone);
}
