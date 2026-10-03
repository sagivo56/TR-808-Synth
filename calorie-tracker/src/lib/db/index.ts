// נקודת הכניסה היחידה לשכבת הנתונים. ייבאו תמיד מ-"@/lib/db".
//
// מבנה התיקייה:
//   client.ts  - חיבור ה-SQLite (singleton)
//   schema.ts  - הגדרת כל הטבלאות והמיגרציות
//   users.ts   - גישה להגדרות/פרופיל המשתמש
//   meals.ts   - גישה לארוחות
//
// פיצ'ר חדש = קובץ repository חדש (למשל steps.ts) + export שורה אחת כאן.

export { getDb } from "./client";
export * from "./users";
export * from "./meals";
