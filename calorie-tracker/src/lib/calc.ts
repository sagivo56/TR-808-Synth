import type { Sex, Goal } from "./types";

export const ACTIVITY_LEVELS: { value: number; label: string }[] = [
  { value: 1.2, label: "יושבני (מעט או ללא פעילות)" },
  { value: 1.375, label: "פעילות קלה (1-3 אימונים בשבוע)" },
  { value: 1.55, label: "פעילות בינונית (3-5 אימונים בשבוע)" },
  { value: 1.725, label: "פעילות גבוהה (6-7 אימונים בשבוע)" },
  { value: 1.9, label: "פעילות אינטנסיבית מאוד (עבודה פיזית/פעמיים ביום)" },
];

export const GOALS: { value: Goal; label: string; adj: number }[] = [
  { value: "maintain", label: "שמירה על משקל", adj: 0 },
  { value: "lose", label: "ירידה מתונה (‎-500 קק\"ל)", adj: -500 },
  { value: "gain", label: "עלייה מתונה (‎+350 קק\"ל)", adj: 350 },
];

export const SEX_OPTIONS: { value: Sex; label: string }[] = [
  { value: "male", label: "זכר" },
  { value: "female", label: "נקבה" },
  { value: "other", label: "אחר" },
];

/**
 * חישוב BMR לפי נוסחת Mifflin-St Jeor.
 */
export function computeBMR(
  weight_kg: number,
  height_cm: number,
  age: number,
  sex: Sex
): number {
  const base = 10 * weight_kg + 6.25 * height_cm - 5 * age;
  if (sex === "male") return base + 5;
  if (sex === "female") return base - 161;
  return base - 78; // אחר
}

/**
 * חישוב יעד קלורי יומי: TDEE = BMR * מקדם פעילות + התאמת מטרה.
 * מעוגל ל-10 הקרוב, מינימום 1000.
 */
export function computeTarget(input: {
  weight_kg: number;
  height_cm: number;
  age: number;
  sex: Sex;
  activity: number;
  goal: Goal;
}): number {
  const bmr = computeBMR(
    input.weight_kg,
    input.height_cm,
    input.age,
    input.sex
  );
  const goalAdj = GOALS.find((g) => g.value === input.goal)?.adj ?? 0;
  const tdee = bmr * input.activity + goalAdj;
  const rounded = Math.round(tdee / 10) * 10;
  return Math.max(1000, rounded);
}
