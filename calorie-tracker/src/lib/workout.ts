// לוגיקה וטיפוסים למעקב האימונים (Tom Holland Calisthenics).
// כל הלוגיקה טהורה כאן; רכיבי ה-UI יושבים ב-src/components/workout.

export type Equipment = "full" | "none";

export type ExerciseKey =
  | "pullups"
  | "dips"
  | "pushups"
  | "airsquats"
  | "situps"
  | "squats";

interface ExerciseName {
  he: string;
  en: string;
}

// שמות התרגילים, עם החלפה דינמית כשאין ציוד (מוט מתח / מקבילים).
const NAMES: Record<ExerciseKey, { full: ExerciseName; none: ExerciseName }> = {
  pullups: {
    full: { he: "מתח", en: "Pull-ups" },
    none: { he: "משיכות סופרמן", en: "Superman Pull-downs" },
  },
  dips: {
    full: { he: "מקבילים", en: "Dips" },
    none: { he: "מקבילים בכיסא", en: "Chair Dips" },
  },
  pushups: {
    full: { he: "שכיבות סמיכה", en: "Push-ups" },
    none: { he: "שכיבות סמיכה", en: "Push-ups" },
  },
  airsquats: {
    full: { he: "סקוואט אוויר", en: "Air Squats" },
    none: { he: "סקוואט אוויר", en: "Air Squats" },
  },
  situps: {
    full: { he: "כפיפות בטן", en: "Sit-ups" },
    none: { he: "כפיפות בטן", en: "Sit-ups" },
  },
  squats: {
    full: { he: "סקוואט", en: "Squats" },
    none: { he: "סקוואט", en: "Squats" },
  },
};

export function exerciseName(key: ExerciseKey, eq: Equipment): ExerciseName {
  return NAMES[key][eq];
}

// ---- מצב A: AMRAP 20 דקות (Cindy) ----

export const AMRAP_SECONDS = 20 * 60;

export interface RoundExercise {
  key: ExerciseKey;
  reps: number;
}

// סיבוב קבוע: 5 מתח, 10 שכיבות סמיכה, 15 סקוואט אוויר.
export const AMRAP_ROUND: RoundExercise[] = [
  { key: "pullups", reps: 5 },
  { key: "pushups", reps: 10 },
  { key: "airsquats", reps: 15 },
];

export const AMRAP_REPS_PER_ROUND = AMRAP_ROUND.reduce((s, e) => s + e.reps, 0); // 30

// ---- מצב B: סולם 1,500 חזרות (Monster Ladder) ----

// מכפיל חזרות לכל תרגיל, לפי הרמה.
export const LADDER_MULTIPLIERS: { key: ExerciseKey; mult: number }[] = [
  { key: "pullups", mult: 1 },
  { key: "dips", mult: 2 },
  { key: "pushups", mult: 3 },
  { key: "situps", mult: 4 },
  { key: "squats", mult: 5 },
];

// רצף הרמות: עולים 1→10 ואז יורדים 9→1 (19 שלבים, סה"כ 1,500 חזרות).
export const LADDER_SEQUENCE: number[] = (() => {
  const up = Array.from({ length: 10 }, (_, i) => i + 1); // 1..10
  const down = Array.from({ length: 9 }, (_, i) => 9 - i); // 9..1
  return [...up, ...down];
})();

/** החזרות הנדרשות בכל תרגיל עבור רמה נתונה. */
export function ladderReps(level: number): RoundExercise[] {
  return LADDER_MULTIPLIERS.map(({ key, mult }) => ({
    key,
    reps: level * mult,
  }));
}

/** סך החזרות ברמה נתונה (15 × הרמה). */
export function ladderRepsTotal(level: number): number {
  return ladderReps(level).reduce((s, e) => s + e.reps, 0);
}

export const LADDER_TOTAL_REPS = LADDER_SEQUENCE.reduce(
  (s, lvl) => s + ladderRepsTotal(lvl),
  0
); // 1500

// ---- עיצוב זמן ----

/** מספר שלם עם מפרידי אלפים — דטרמיניסטי (זהה בשרת ובדפדפן, בלי toLocaleString). */
export function formatInt(n: number): string {
  return Math.round(n)
    .toString()
    .replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}

/** שניות → MM:SS */
export function formatClock(totalSeconds: number): string {
  const s = Math.max(0, Math.round(totalSeconds));
  const mm = Math.floor(s / 60);
  const ss = s % 60;
  return `${String(mm).padStart(2, "0")}:${String(ss).padStart(2, "0")}`;
}

/** מילישניות → MM:SS.d (לשעון עצר הסולם) */
export function formatStopwatch(ms: number): string {
  const totalTenths = Math.floor(Math.max(0, ms) / 100);
  const tenths = totalTenths % 10;
  const totalSeconds = Math.floor(totalTenths / 10);
  const mm = Math.floor(totalSeconds / 60);
  const ss = totalSeconds % 60;
  return `${String(mm).padStart(2, "0")}:${String(ss).padStart(2, "0")}.${tenths}`;
}

// ---- שיאים אישיים (localStorage) ----

const KEY_PB_AMRAP = "workout_pb_amrap_rounds";
const KEY_PB_LADDER = "workout_pb_ladder_ms";
const KEY_EQUIPMENT = "workout_equipment";

function readNumber(key: string): number | null {
  try {
    const raw = localStorage.getItem(key);
    if (raw == null) return null;
    const n = Number(raw);
    return Number.isFinite(n) ? n : null;
  } catch {
    return null;
  }
}

function writeNumber(key: string, value: number): void {
  try {
    localStorage.setItem(key, String(value));
  } catch {
    /* localStorage חסום (למשל גלישה פרטית) - מתעלמים */
  }
}

/** שיא AMRAP = מספר הסיבובים המרבי. */
export function getAmrapPB(): number | null {
  return readNumber(KEY_PB_AMRAP);
}

/** שומר שיא חדש אם טוב מהקודם. מחזיר true אם נשבר שיא. */
export function saveAmrapPB(rounds: number): boolean {
  const prev = getAmrapPB();
  if (prev == null || rounds > prev) {
    writeNumber(KEY_PB_AMRAP, rounds);
    return true;
  }
  return false;
}

/** שיא הסולם = הזמן המהיר ביותר במילישניות. */
export function getLadderPB(): number | null {
  return readNumber(KEY_PB_LADDER);
}

/** שומר זמן חדש אם מהיר מהקודם. מחזיר true אם נשבר שיא. */
export function saveLadderPB(ms: number): boolean {
  const prev = getLadderPB();
  if (prev == null || ms < prev) {
    writeNumber(KEY_PB_LADDER, ms);
    return true;
  }
  return false;
}

export function getEquipment(): Equipment {
  try {
    return localStorage.getItem(KEY_EQUIPMENT) === "none" ? "none" : "full";
  } catch {
    return "full";
  }
}

export function saveEquipment(eq: Equipment): void {
  try {
    localStorage.setItem(KEY_EQUIPMENT, eq);
  } catch {
    /* ignore */
  }
}
