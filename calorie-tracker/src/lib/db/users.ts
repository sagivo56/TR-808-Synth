import type { Settings, Sex, Goal } from "../types";
import { getDb } from "./client";

// שכבת גישה לנתוני המשתמש וההגדרות (טבלת users).

interface UserRow {
  id: string;
  height_cm: number;
  weight_kg: number;
  age: number;
  sex: Sex;
  activity: number;
  goal: Goal;
  muscle_goal: number;
  target: number;
}

export function getSettings(userId: string): Settings | null {
  const row = getDb()
    .prepare(
      "SELECT height_cm, weight_kg, age, sex, activity, goal, muscle_goal, target FROM users WHERE id = ?"
    )
    .get(userId) as UserRow | undefined;
  if (!row) return null;
  return {
    height_cm: row.height_cm,
    weight_kg: row.weight_kg,
    age: row.age,
    sex: row.sex,
    activity: row.activity,
    goal: row.goal,
    muscle_goal: !!row.muscle_goal,
    target: row.target,
  };
}

export function saveSettings(userId: string, s: Settings): void {
  const now = Date.now();
  getDb()
    .prepare(
      `INSERT INTO users (id, height_cm, weight_kg, age, sex, activity, goal, muscle_goal, target, created_at, updated_at)
       VALUES (@id, @height_cm, @weight_kg, @age, @sex, @activity, @goal, @muscle_goal, @target, @now, @now)
       ON CONFLICT(id) DO UPDATE SET
         height_cm = @height_cm,
         weight_kg = @weight_kg,
         age = @age,
         sex = @sex,
         activity = @activity,
         goal = @goal,
         muscle_goal = @muscle_goal,
         target = @target,
         updated_at = @now`
    )
    // better-sqlite3 אינו כובל boolean ישירות - המרה ל-0/1
    .run({ id: userId, ...s, muscle_goal: s.muscle_goal ? 1 : 0, now });
}

/** עדכון ידני מהיר של מספר היעד בלבד. */
export function updateTarget(userId: string, target: number): boolean {
  const res = getDb()
    .prepare("UPDATE users SET target = ?, updated_at = ? WHERE id = ?")
    .run(target, Date.now(), userId);
  return res.changes > 0;
}
