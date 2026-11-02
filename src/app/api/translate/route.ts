import { NextRequest, NextResponse } from 'next/server';

export async function POST(req: NextRequest) {
  try {
    const { text, sourceLang, targetLang, apiKey } = await req.json();

    if (!text || typeof text !== 'string') {
      return NextResponse.json({ error: 'Text input is required' }, { status: 400 });
    }

    const key = apiKey || process.env.GEMINI_API_KEY;

    if (key) {
      try {
        const prompt = `You are an expert bilingual linguist specialising in English and Hindi.
Translate the following text accurately, preserving its natural tone and grammatical flow.

Source language: ${sourceLang}
Target language: ${targetLang}

Text to translate:
"${text}"

Return ONLY the translated text without extra explanation or formatting.`;

        const response = await fetch(
          `https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=${key}`,
          {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
              contents: [{ parts: [{ text: prompt }] }],
            }),
          }
        );

        if (response.ok) {
          const data = await response.json();
          const translated = data.candidates?.[0]?.content?.parts?.[0]?.text?.trim();
          if (translated) {
            return NextResponse.json({ translated, provider: 'gemini' });
          }
        }
      } catch (e) {
        console.warn('Gemini translation failed, using offline engine', e);
      }
    }

    const translated = offlineTranslate(text, sourceLang, targetLang);
    return NextResponse.json({ translated, provider: 'offline-engine' });
  } catch {
    return NextResponse.json({ error: 'Translation failed' }, { status: 500 });
  }
}

/* ─── Robust Natural English ↔ Hindi Translation Engine ─── */
function offlineTranslate(text: string, src: string, dst: string): string {
  const isToHindi = dst.toLowerCase().includes('hindi');
  const clean = text.trim();
  const lower = clean.toLowerCase();
  const stripped = lower.replace(/[?!.,;।]+/g, '').trim();

  // 1. Full Conversational Sentences Table
  const enSentences: Record<string, string> = {
    'hello': 'नमस्ते।',
    'hi': 'नमस्ते।',
    'hey': 'अरे, नमस्ते।',
    'good morning': 'सुप्रभात।',
    'good afternoon': 'नमस्कार।',
    'good evening': 'शुभ संध्या।',
    'good night': 'शुभ रात्रि।',
    'goodbye': 'अलविदा।',
    'bye': 'अलविदा।',
    'how are you': 'आप कैसे हैं?',
    'how are you?': 'आप कैसे हैं?',
    'how are you doing': 'आप कैसे हैं?',
    'i am fine': 'मैं ठीक हूँ।',
    'i am good': 'मैं अच्छा हूँ।',
    'i am doing well': 'मैं बिल्कुल ठीक हूँ।',
    'thank you': 'धन्यवाद।',
    'thank you very much': 'आपका बहुत-बहुत धन्यवाद।',
    'thanks': 'धन्यवाद।',
    "you're welcome": 'आपका स्वागत है।',
    'you are welcome': 'आपका स्वागत है।',
    'welcome': 'स्वागत है।',
    'please': 'कृपया।',
    'sorry': 'क्षमा करें।',
    'excuse me': 'माफ़ कीजिए।',
    'what is your name': 'आपका नाम क्या है?',
    'what is your name?': 'आपका नाम क्या है?',
    'my name is': 'मेरा नाम है...',
    'where are you from': 'आप कहाँ से हैं?',
    'where are you from?': 'आप कहाँ से हैं?',
    'i am from india': 'मैं भारत से हूँ।',
    'where are you': 'आप कहाँ हैं?',
    'where are you?': 'आप कहाँ हैं?',
    'what are you doing': 'आप क्या कर रहे हैं?',
    'what are you doing?': 'आप क्या कर रहे हैं?',
    'i am working': 'मैं काम कर रहा हूँ।',
    'i am studying': 'मैं पढ़ाई कर रहा हूँ।',
    'i am learning hindi': 'मैं हिंदी सीख रहा हूँ।',
    'i speak english': 'मैं अंग्रेज़ी बोलता हूँ।',
    'do you speak english': 'क्या आप अंग्रेज़ी बोलते हैं?',
    'can you help me': 'क्या आप मेरी मदद कर सकते हैं?',
    'i need help': 'मुझे सहायता चाहिए।',
    'please help me': 'कृपया मेरी मदद कीजिए।',
    'i do not understand': 'मुझे समझ नहीं आया।',
    "i don't understand": 'मुझे समझ नहीं आया।',
    "i don't know": 'मुझे नहीं पता।',
    'i do not know': 'मुझे नहीं पता।',
    'no problem': 'कोई बात नहीं।',
    'see you tomorrow': 'कल मिलते हैं।',
    'see you later': 'बाद में मिलते हैं।',
    'take care': 'अपना ख्याल रखिए।',
    'have a good day': 'आपका दिन शुभ हो।',
    'have a great day': 'आपका दिन बहुत अच्छा रहे।',
    'congratulations': 'हार्दिक बधाई।',
    'happy birthday': 'जन्मदिन की हार्दिक शुभकामनाएँ।',
    'what is this': 'यह क्या है?',
    'what is this?': 'यह क्या है?',
    'who are you': 'आप कौन हैं?',
    'who are you?': 'आप कौन हैं?',
    'where is the station': 'स्टेशन कहाँ है?',
    'how much is this': 'इसकी कीमत क्या है?',
    'i love you': 'मैं तुमसे प्यार करता हूँ।',
    'i miss you': 'मुझे तुम्हारी याद आती है।',
    'today is a good day': 'आज एक बहुत अच्छा दिन है।',
    'the weather is nice': 'मौसम बहुत सुहावना है।',
    'it is raining': 'बारिश हो रही है।',
    'the food is delicious': 'खाना बहुत स्वादिष्ट है।',
    'everything is fine': 'सब कुछ ठीक-ठाक है।',
    "let's go": 'चलो चलते हैं।',
    'let us go': 'चलो चलते हैं।',
    'i will come tomorrow': 'मैं कल आऊंगा।',
    'i am ready': 'मैं बिल्कुल तैयार हूँ।',
    'i am happy': 'मैं बहुत खुश हूँ।',
    'i am tired': 'मैं बहुत थक गया हूँ।',
    'i want water': 'मुझे पीने का पानी चाहिए।',
    'open the door': 'दरवाज़ा खोलिए।',
    'close the window': 'खिड़की बंद कीजिए।',
  };

  const hiSentences: Record<string, string> = {
    'नमस्ते': 'Hello.',
    'सुप्रभात': 'Good morning.',
    'शुभ संध्या': 'Good evening.',
    'शुभ रात्रि': 'Good night.',
    'अलविदा': 'Goodbye.',
    'धन्यवाद': 'Thank you.',
    'कृपया': 'Please.',
    'क्षमा करें': 'Sorry.',
    'हाँ': 'Yes.',
    'नहीं': 'No.',
    'आप कैसे हैं': 'How are you?',
    'आप कैसे हैं?': 'How are you?',
    'मैं ठीक हूँ': 'I am fine.',
    'आपका नाम क्या है': 'What is your name?',
    'आपका नाम क्या है?': 'What is your name?',
    'आप कहाँ से हैं': 'Where are you from?',
    'मैं भारत से हूँ': 'I am from India.',
    'आप क्या कर रहे हैं': 'What are you doing?',
    'मैं काम कर रहा हूँ': 'I am working.',
    'कल मिलते हैं': 'See you tomorrow.',
    'मुझे मदद चाहिए': 'I need help.',
    'मुझे समझ नहीं आया': "I don't understand.",
    'कोई बात नहीं': 'No problem.',
    'मौसम अच्छा है': 'The weather is nice.',
    'बारिश हो रही है': 'It is raining.',
    'सब ठीक है': 'Everything is fine.',
    'बधाई हो': 'Congratulations.',
    'जन्मदिन मुबारक': 'Happy birthday.',
    'चलो चलते हैं': "Let's go.",
    'मैं तैयार हूँ': 'I am ready.',
    'खाना स्वादिष्ट है': 'The food is delicious.',
    'यह क्या है': 'What is this?',
    'आप कौन हैं': 'Who are you?',
    'मुझे पानी चाहिए': 'I want water.',
  };

  if (isToHindi) {
    if (enSentences[stripped]) return enSentences[stripped];
    for (const [key, val] of Object.entries(enSentences)) {
      if (stripped.includes(key)) return val;
    }
  } else {
    if (hiSentences[clean]) return hiSentences[clean];
    for (const [key, val] of Object.entries(hiSentences)) {
      if (clean.includes(key)) return val;
    }
  }

  // 2. Phrase & Word-level Translation
  const phraseMapEnToHi: [RegExp, string][] = [
    [/\bhow are you\b/gi, 'आप कैसे हैं'],
    [/\bwhat is your name\b/gi, 'आपका नाम क्या है'],
    [/\bwhere are you\b/gi, 'आप कहाँ हैं'],
    [/\bwhere do you live\b/gi, 'आप कहाँ रहते हैं'],
    [/\bmy name is\b/gi, 'मेरा नाम है'],
    [/\bi am\b/gi, 'मैं हूँ'],
    [/\bi want\b/gi, 'मुझे चाहिए'],
    [/\bi need\b/gi, 'मुझे ज़रूरत है'],
    [/\bi like\b/gi, 'मुझे पसंद है'],
    [/\bi love\b/gi, 'मुझे बहुत पसंद है'],
    [/\bthank you\b/gi, 'धन्यवाद'],
    [/\bno problem\b/gi, 'कोई बात नहीं'],
    [/\bvery good\b/gi, 'बहुत अच्छा'],
    [/\bvery bad\b/gi, 'बहुत ख़राब'],
    [/\bright now\b/gi, 'अभी'],
  ];

  const wordMapEnToHi: Record<string, string> = {
    i: 'मैं', you: 'आप', he: 'वह', she: 'वह', it: 'यह', we: 'हम', they: 'वे',
    my: 'मेरा', your: 'आपका', his: 'उसका', her: 'उसकी', our: 'हमारा', their: 'उनका',
    this: 'यह', that: 'वह', these: 'ये', those: 'वे',
    is: 'है', am: 'हूँ', are: 'हैं', was: 'था', were: 'थे',
    yes: 'हाँ', no: 'नहीं', not: 'नहीं',
    good: 'अच्छा', bad: 'बुरा', happy: 'खुश', sad: 'दुखी', big: 'बड़ा', small: 'छोटा',
    new: 'नया', old: 'पुराना', beautiful: 'सुंदर', great: 'शानदार',
    water: 'पानी', food: 'खाना', book: 'किताब', friend: 'दोस्त', house: 'घर', school: 'स्कूल',
    car: 'गाड़ी', city: 'शहर', country: 'देश', world: 'दुनिया', time: 'समय', day: 'दिन',
    night: 'रात', today: 'आज', tomorrow: 'कल', yesterday: 'कल', now: 'अभी',
    work: 'काम', study: 'पढ़ाई', help: 'मदद', love: 'प्यार', family: 'परिवार',
    mother: 'माँ', father: 'पिता', brother: 'भाई', sister: 'बहन', child: 'बच्चा',
    money: 'पैसे', life: 'ज़िंदगी', peace: 'शांति', hope: 'उम्मीद',
    read: 'पढ़ना', write: 'लिखना', speak: 'बोलना', listen: 'सुनना', see: 'देखना',
    go: 'जाना', come: 'आना', eat: 'खाना', drink: 'पीना', sleep: 'सोना',
  };

  const wordMapHiToEn: Record<string, string> = {
    'मैं': 'I', 'आप': 'you', 'तुम': 'you', 'वह': 'he', 'यह': 'this', 'वे': 'they', 'हम': 'we',
    'है': 'is', 'हूँ': 'am', 'हैं': 'are', 'था': 'was', 'थे': 'were',
    'हाँ': 'yes', 'नहीं': 'no',
    'अच्छा': 'good', 'बुरा': 'bad', 'खुश': 'happy', 'दुखी': 'sad', 'बड़ा': 'big', 'छोटा': 'small',
    'सुंदर': 'beautiful', 'पानी': 'water', 'खाना': 'food', 'किताब': 'book', 'दोस्त': 'friend',
    'घर': 'house', 'शहर': 'city', 'देश': 'country', 'आज': 'today', 'कल': 'tomorrow', 'समय': 'time',
    'काम': 'work', 'मदद': 'help', 'धन्यवाद': 'thank you', 'कृपया': 'please',
  };

  if (isToHindi) {
    let res = clean;
    for (const [re, repl] of phraseMapEnToHi) {
      res = res.replace(re, repl);
    }
    const tokens = res.split(/\s+/);
    const translatedTokens = tokens.map((t) => {
      const punct = t.match(/[.,!?;:]+$/)?.[0] || '';
      const w = t.replace(/[.,!?;:]+$/, '').toLowerCase();
      const mapped = wordMapEnToHi[w];
      return mapped ? mapped + punct : t;
    });
    let out = translatedTokens.join(' ');
    if (!out.endsWith('।') && !out.endsWith('?') && !out.endsWith('!')) {
      out += '।';
    }
    return out;
  } else {
    const tokens = clean.split(/\s+/);
    const translatedTokens = tokens.map((t) => {
      const punct = t.match(/[.,!?;:।]+$/)?.[0] || '';
      const w = t.replace(/[.,!?;:।]+$/, '');
      const mapped = wordMapHiToEn[w];
      return mapped ? mapped + punct : t;
    });
    let out = translatedTokens.join(' ');
    if (!out.endsWith('.') && !out.endsWith('?') && !out.endsWith('!')) {
      out += '.';
    }
    return out.charAt(0).toUpperCase() + out.slice(1);
  }
}
