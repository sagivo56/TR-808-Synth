import Database from "better-sqlite3";
import fs from "node:fs";
import path from "node:path";
import type { Settings, Meal, Sex, Goal } from "./types";

// שמירה מתמשכת אמיתית בצד שרת עם SQLite.
// מפתח פר משתמש (userId מזוהה דרך cookie) ופר יום (date=YYYY-MM-DD).

let db: Database.Database | null = null;

function getDb(): Database.Database {
  if (db) return db;

  const dbPath = process.env.DATABASE_PATH || path.join(process.cwd(), "data", "app.db");
  fs.mkdirSync(path.dirname(dbPath), { recursive: true });

  db = new Database(dbPath);
  db.pragma("journal_mode = WAL");

  db.exec(`
    CREATE TABLE IF NOT EXISTS users (
      id          TEXT PRIMARY KEY,
      height_cm   REAL NOT NULL,
      weight_kg   REAL NOT NULL,
      age         INTEGER NOT NULL,
      sex         TEXT NOT NULL,
      activity    REAL NOT NULL,
      goal        TEXT NOT NULL,
      muscle_goal INTEGER NOT NULL DEFAULT 0,
      target      INTEGER NOT NULL,
      created_at  INTEGER NOT NULL,
      updated_at  INTEGER NOT NULL
    );

    CREATE TABLE IF NOT EXISTS meals (
      id          TEXT PRIMARY KEY,
      user_id     TEXT NOT NULL,
      date        TEXT NOT NULL,
      time        TEXT NOT NULL,
      name        TEXT NOT NULL,
      calories    INTEGER NOT NULL,
      protein_g   REAL NOT NULL,
      carbs_g     REAL NOT NULL,
      fat_g       REAL NOT NULL,
      created_at  INTEGER NOT NULL
    );

    CREATE INDEX IF NOT EXISTS idx_meals_user_date ON meals (user_id, date);
  `);

  // מיגרציה: הוספת עמודת muscle_goal לטבלאות קיימות שנוצרו לפני הפיצ'ר
  const hasMuscleGoal = (
    db.prepare("PRAGMA table_info(users)").all() as { name: string }[]
  ).some((c) => c.name === "muscle_goal");
  if (!hasMuscleGoal) {
    db.exec("ALTER TABLE users ADD COLUMN muscle_goal INTEGER NOT NULL DEFAULT 0");
  }

  return db;
}

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

export function getMeals(userId: string, date: string): Meal[] {
  const rows = getDb()
    .prepare(
      `SELECT id, date, time, name, calories, protein_g, carbs_g, fat_g, created_at
       FROM meals WHERE user_id = ? AND date = ? ORDER BY time ASC, created_at ASC`
    )
    .all(userId, date) as Meal[];
  return rows;
}

/** עדכון שעת ארוחה (הזזה לזמן אחר). מחזיר את הארוחה המעודכנת או null. */
export function updateMealTime(
  userId: string,
  mealId: string,
  time: string
): Meal | null {
  const res = getDb()
    .prepare("UPDATE meals SET time = ? WHERE user_id = ? AND id = ?")
    .run(time, userId, mealId);
  if (res.changes === 0) return null;
  return getDb()
    .prepare(
      `SELECT id, date, time, name, calories, protein_g, carbs_g, fat_g, created_at
       FROM meals WHERE user_id = ? AND id = ?`
    )
    .get(userId, mealId) as Meal;
}

export function addMeal(
  userId: string,
  meal: Omit<Meal, "id" | "created_at"> & { id?: string }
): Meal {
  const id = meal.id ?? crypto.randomUUID();
  const created_at = Date.now();
  getDb()
    .prepare(
      `INSERT INTO meals (id, user_id, date, time, name, calories, protein_g, carbs_g, fat_g, created_at)
       VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)`
    )
    .run(
      id,
      userId,
      meal.date,
      meal.time,
      meal.name,
      Math.round(meal.calories),
      meal.protein_g,
      meal.carbs_g,
      meal.fat_g,
      created_at
    );
  return {
    id,
    date: meal.date,
    time: meal.time,
    name: meal.name,
    calories: Math.round(meal.calories),
    protein_g: meal.protein_g,
    carbs_g: meal.carbs_g,
    fat_g: meal.fat_g,
    created_at,
  };
}

export function deleteMeal(userId: string, mealId: string): boolean {
  const res = getDb()
    .prepare("DELETE FROM meals WHERE user_id = ? AND id = ?")
    .run(userId, mealId);
  return res.changes > 0;
}
