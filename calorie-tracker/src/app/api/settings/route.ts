import { NextRequest, NextResponse } from "next/server";
import { resolveUser, attachUserCookie } from "@/lib/user";
import { getSettings, saveSettings, updateTarget } from "@/lib/db";
import { computeTarget } from "@/lib/calc";
import type { Sex, Goal, Settings } from "@/lib/types";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

const VALID_SEX: Sex[] = ["male", "female", "other"];
const VALID_GOAL: Goal[] = ["maintain", "lose", "gain"];
const VALID_ACTIVITY = [1.2, 1.375, 1.55, 1.725, 1.9];

// GET - החזרת הגדרות המשתמש (או null אם עדיין לא הוגדרו)
export async function GET() {
  const { userId, isNew } = resolveUser();
  const settings = isNew ? null : getSettings(userId);
  const res = NextResponse.json({ settings });
  if (isNew) attachUserCookie(res, userId);
  return res;
}

// POST - שמירת הגדרות מלאות וחישוב יעד מחדש
export async function POST(req: NextRequest) {
  const { userId, isNew } = resolveUser();

  let body: unknown;
  try {
    body = await req.json();
  } catch {
    return NextResponse.json({ error: "גוף הבקשה אינו JSON תקין" }, { status: 400 });
  }

  const b = body as Record<string, unknown>;
  const height_cm = Number(b.height_cm);
  const weight_kg = Number(b.weight_kg);
  const age = Number(b.age);
  const sex = b.sex as Sex;
  const activity = Number(b.activity);
  const goal = b.goal as Goal;
  const muscle_goal = b.muscle_goal === true; // ברירת מחדל: לא

  if (!(height_cm > 0 && height_cm < 300))
    return NextResponse.json({ error: "גובה לא תקין (ס\"מ)" }, { status: 400 });
  if (!(weight_kg > 0 && weight_kg < 500))
    return NextResponse.json({ error: "משקל לא תקין (ק\"ג)" }, { status: 400 });
  if (!(age > 0 && age < 130))
    return NextResponse.json({ error: "גיל לא תקין" }, { status: 400 });
  if (!VALID_SEX.includes(sex))
    return NextResponse.json({ error: "מין לא תקין" }, { status: 400 });
  if (!VALID_ACTIVITY.includes(activity))
    return NextResponse.json({ error: "רמת פעילות לא תקינה" }, { status: 400 });
  if (!VALID_GOAL.includes(goal))
    return NextResponse.json({ error: "מטרה לא תקינה" }, { status: 400 });

  const target = computeTarget({ height_cm, weight_kg, age, sex, activity, goal });
  const settings: Settings = {
    height_cm,
    weight_kg,
    age,
    sex,
    activity,
    goal,
    muscle_goal,
    target,
  };
  saveSettings(userId, settings);

  const res = NextResponse.json({ settings });
  if (isNew) attachUserCookie(res, userId);
  return res;
}

// PATCH - עדכון ידני מהיר של מספר היעד בלבד
export async function PATCH(req: NextRequest) {
  const { userId, isNew } = resolveUser();
  if (isNew)
    return NextResponse.json({ error: "לא קיימות הגדרות לעדכון" }, { status: 400 });

  let body: unknown;
  try {
    body = await req.json();
  } catch {
    return NextResponse.json({ error: "גוף הבקשה אינו JSON תקין" }, { status: 400 });
  }

  const target = Math.round(Number((body as Record<string, unknown>).target));
  if (!(target >= 1000 && target <= 20000))
    return NextResponse.json({ error: "יעד לא תקין (מינימום 1000)" }, { status: 400 });

  const ok = updateTarget(userId, target);
  if (!ok)
    return NextResponse.json({ error: "לא קיימות הגדרות לעדכון" }, { status: 400 });

  return NextResponse.json({ settings: getSettings(userId) });
}
