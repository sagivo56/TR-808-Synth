import type { Meal } from "../types";
import { getDb } from "./client";

// שכבת גישה לארוחות (טבלת meals).

export function getMeals(userId: string, date: string): Meal[] {
  const rows = getDb()
    .prepare(
      `SELECT id, date, time, name, calories, protein_g, carbs_g, fat_g, created_at
       FROM meals WHERE user_id = ? AND date = ? ORDER BY time ASC, created_at ASC`
    )
    .all(userId, date) as Meal[];
  return rows;
}

/** הזזת ארוחה לזמן אחר: שעה, ואופציונלית גם תאריך. מחזיר את הארוחה המעודכנת או null. */
export function updateMeal(
  userId: string,
  mealId: string,
  time: string,
  date?: string
): Meal | null {
  const res = date
    ? getDb()
        .prepare("UPDATE meals SET time = ?, date = ? WHERE user_id = ? AND id = ?")
        .run(time, date, userId, mealId)
    : getDb()
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
