import { NextRequest, NextResponse } from 'next/server';
import { transformToTone } from '@/lib/toneEngine';

export async function POST(req: NextRequest) {
  try {
    const { text, tone, customInstruction, apiKey } = await req.json();

    if (!text || typeof text !== 'string') {
      return NextResponse.json({ error: 'Text input is required' }, { status: 400 });
    }

    const key = apiKey || process.env.GEMINI_API_KEY;

    // 1. If Gemini API key is available, call Gemini for rich AI-generated transformation
    if (key) {
      try {
        const prompt = `You are an expert linguistic stylist in the TypeAssist writing platform.
Rewrite the following input text into a distinct, authentic ${tone || 'Professional'} tone.
Completely transform the vocabulary, sentence structure, idioms, and voice to match the requested style.
Preserve the core meaning, but do not just append a fixed suffix or prefix.
Do not wrap the text in quotation marks.
Do not repeat the original sentence inside fixed templates.

Style requested: ${tone}
${customInstruction ? `Additional notes: ${customInstruction}` : ''}

Original text:
"${text}"

Return ONLY the transformed text. Do not add quotation marks, explanations, or markdown.`;

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
          const transformed = data.candidates?.[0]?.content?.parts?.[0]?.text?.trim();
          if (transformed) {
            return NextResponse.json({ transformed, provider: 'gemini' });
          }
        }
      } catch (e) {
        console.warn('Gemini API call failed, falling back to local engine', e);
      }
    }

    // 2. High-precision offline rule & semantic transformation engine
    const transformed = transformToTone(text, tone);
    return NextResponse.json({ transformed, provider: 'local-engine' });
  } catch {
    return NextResponse.json({ error: 'Failed to transform tone' }, { status: 500 });
  }
}
