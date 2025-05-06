import en from "./en.json";
import pt from "./pt.json";

const translations = { en, pt };

export function t(lang: "en" | "pt", key: string): string {
  return translations[lang][key] || key;
}

