export type Sex = "male" | "female" | "other";

export type Goal = "maintain" | "lose" | "gain";

export interface Settings {
  height_cm: number;
  weight_kg: number;
  age: number;
  sex: Sex;
  activity: number; // 1.2 / 1.375 / 1.55 / 1.725 / 1.9
  goal: Goal;
  muscle_goal: boolean; // האם יש מטרה להגדיל מסת שריר
  target: number; // יעד קלורי יומי מחושב (או ידני)
}

export interface Meal {
  id: string;
  date: string; // YYYY-MM-DD
  time: string; // HH:MM
  name: string;
  calories: number;
  protein_g: number;
  carbs_g: number;
  fat_g: number;
  created_at: number;
}

export interface DaySummary {
  date: string;
  meals: Meal[];
  totals: {
    calories: number;
    protein_g: number;
    carbs_g: number;
    fat_g: number;
  };
}

// תוכן הודעה בפורמט Anthropic (טקסט או תמונה) שנשמר בצד לקוח בתוך thread
export type ChatBlock =
  | { type: "text"; text: string }
  | {
      type: "image";
      source: { type: "base64"; media_type: string; data: string };
    };

export interface ChatMessage {
  role: "user" | "assistant";
  content: ChatBlock[];
}

// תשובת המודל לאחר פענוח JSON
export type EstimateResult =
  | { status: "question"; question: string }
  | {
      status: "result";
      name: string;
      calories: number;
      protein_g: number;
      carbs_g: number;
      fat_g: number;
      tip?: string; // טיפ קצר לשיפור מאזן המנה (מותאם למטרת השריר)
    };
