import { NextRequest, NextResponse } from 'next/server';

export async function POST(req: NextRequest) {
  try {
    const { message, apiKey } = await req.json();

    if (!message || typeof message !== 'string') {
      return NextResponse.json({ error: 'Incoming message is required' }, { status: 400 });
    }

    const key = apiKey || process.env.GEMINI_API_KEY;

    if (key) {
      try {
        const prompt = `You are Smart Reply in TypeAssist. Given the incoming message, generate 4 contextual replies in distinct tones:
1. short
2. friendly
3. professional
4. polite

Incoming message:
"${message}"

Respond ONLY with valid JSON:
{
  "short": "...",
  "friendly": "...",
  "professional": "...",
  "polite": "..."
}`;

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
          let raw = data.candidates?.[0]?.content?.parts?.[0]?.text?.trim() || '';
          raw = raw.replace(/^```json\s*/i, '').replace(/```\s*$/i, '');
          const replies = JSON.parse(raw);
          return NextResponse.json({ replies, provider: 'gemini' });
        }
      } catch (e) {
        console.warn('Gemini smart reply failed, using template generator', e);
      }
    }

    // Dynamic, context-aware offline smart replies
    const replies = generateContextualReplies(message);
    return NextResponse.json({ replies, provider: 'local-fallback' });
  } catch (error) {
    return NextResponse.json({ error: 'Smart reply generation failed' }, { status: 500 });
  }
}

function generateContextualReplies(msg: string): {
  short: string;
  friendly: string;
  professional: string;
  polite: string;
} {
  const lower = msg.toLowerCase();

  // 1. Meeting / Scheduling
  if (lower.includes('meet') || lower.includes('call') || lower.includes('zoom') || lower.includes('schedule') || lower.includes('available')) {
    return {
      short: 'Sure, what time works?',
      friendly: 'I’d love to connect! When are you free for a quick chat? 😊',
      professional: 'Thank you for reaching out. I am available tomorrow between 2:00 PM and 5:00 PM to discuss this.',
      polite: 'I would be delighted to schedule a discussion. Please let me know the time that suits your schedule best.'
    };
  }

  // 2. Gratitude / Thank You
  if (lower.includes('thank') || lower.includes('thx') || lower.includes('appreciate')) {
    return {
      short: 'Glad to help!',
      friendly: 'Anytime at all! Always happy to help out! 🙌',
      professional: 'You are very welcome. Please feel free to reach out should you require any additional assistance.',
      polite: 'It was truly my pleasure to be of assistance. Please do not hesitate to contact me if there is anything more I can do.'
    };
  }

  // 3. Question / Inquiry / Status Check
  if (lower.includes('status') || lower.includes('update') || lower.includes('progress') || lower.includes('when') || lower.includes('?')) {
    return {
      short: 'On it, updating shortly.',
      friendly: 'Hey! Making great progress on this, I’ll share a full update in just a bit! 👍',
      professional: 'Thank you for your inquiry. The work is progressing according to schedule and an interim report will follow shortly.',
      polite: 'Thank you for checking in on this matter. I am currently finalizing the details and will share an update promptly.'
    };
  }

  // 4. Agreement / Proposal / Review
  if (lower.includes('review') || lower.includes('check') || lower.includes('look at') || lower.includes('feedback')) {
    return {
      short: 'Looks good to me.',
      friendly: 'Just went through this, looks super solid! Left a couple of quick thoughts. 🎉',
      professional: 'I have reviewed the material in detail. The direction is well aligned with our core requirements and ready to proceed.',
      polite: 'I have thoroughly examined the provided documentation and believe it is in excellent shape.'
    };
  }

  // Default General Contextual Replies
  return {
    short: 'Received and confirmed, thanks.',
    friendly: 'Hey, got your message! Really appreciate you keeping me posted 😊',
    professional: 'Thank you for your communication. I acknowledge receipt and will take the necessary next steps.',
    polite: 'Thank you kindly for reaching out. Please let me know if there is anything further required on my end.'
  };
}
