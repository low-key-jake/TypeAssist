/**
 * TypeAssist — LocalStorage Persistence Layer
 */

const STORAGE_KEYS = {
  PERSONAL_DICTIONARY: 'typeassist_personal_dict',
  HISTORY: 'typeassist_history',
  THEME: 'typeassist_theme',
  SETTINGS: 'typeassist_settings',
};

export interface UserSettings {
  autoCorrect: boolean;
  autoComplete: boolean;
  nextWordPrediction: boolean;
  highlightTypos: boolean;
  theme: 'dark' | 'light';
  apiKey?: string;
}

const DEFAULT_SETTINGS: UserSettings = {
  autoCorrect: true,
  autoComplete: true,
  nextWordPrediction: true,
  highlightTypos: true,
  theme: 'dark',
};

export function getStoredPersonalDictionary(): string[] {
  if (typeof window === 'undefined') return [];
  try {
    const raw = localStorage.getItem(STORAGE_KEYS.PERSONAL_DICTIONARY);
    return raw ? JSON.parse(raw) : [];
  } catch {
    return [];
  }
}

export function saveStoredPersonalDictionary(words: string[]): void {
  if (typeof window === 'undefined') return;
  try {
    localStorage.setItem(STORAGE_KEYS.PERSONAL_DICTIONARY, JSON.stringify(words));
  } catch (e) {
    console.error('Failed to save personal dictionary', e);
  }
}

export function getStoredSettings(): UserSettings {
  if (typeof window === 'undefined') return DEFAULT_SETTINGS;
  try {
    const raw = localStorage.getItem(STORAGE_KEYS.SETTINGS);
    return raw ? { ...DEFAULT_SETTINGS, ...JSON.parse(raw) } : DEFAULT_SETTINGS;
  } catch {
    return DEFAULT_SETTINGS;
  }
}

export function saveStoredSettings(settings: Partial<UserSettings>): void {
  if (typeof window === 'undefined') return;
  try {
    const current = getStoredSettings();
    localStorage.setItem(STORAGE_KEYS.SETTINGS, JSON.stringify({ ...current, ...settings }));
  } catch (e) {
    console.error('Failed to save settings', e);
  }
}

