import { NextRequest, NextResponse } from "next/server";
import { resolveUser, attachUserCookie } from "@/lib/user";
import { getMeals, addMeal, deleteMeal, updateMealTime } from "@/lib/db";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

function todayISO(): string {
  // תאריך מקומי בפורמט YYYY-MM-DD
  const d = new Date();
  const off = d.getTimezoneOffset() * 60000;
  return new Date(d.getTime() - off).toISOString().slice(0, 10);
}

// GET ?date=YYYY-MM-DD - רשימת הארוחות ליום מסוים (ברירת מחדל: היום)
export async function GET(req: NextRequest) {
  const { userId, isNew } = resolveUser();
  const date = req.nextUrl.searchParams.get("date") || todayISO();
  const meals = isNew ? [] : getMeals(userId, date);
  const res = NextResponse.json({ meals, date });
  if (isNew) attachUserCookie(res, userId);
  return res;
}

// POST - הוספת ארוחה
export async function POST(req: NextRequest) {
  const { userId, isNew } = resolveUser();

  let body: unknown;
  try {
    body = await req.json();
  } catch {
    return NextResponse.json({ error: "גוף הבקשה אינו JSON תקין" }, { status: 400 });
  }

  const b = body as Record<string, unknown>;
  const name = String(b.name ?? "").trim();
  const calories = Number(b.calories);
  if (!name)
    return NextResponse.json({ error: "שם הארוחה חסר" }, { status: 400 });
  if (!Number.isFinite(calories) || calories < 0)
    return NextResponse.json({ error: "מספר קלוריות לא תקין" }, { status: 400 });

  const now = new Date();
  const time =
    typeof b.time === "string" && /^\d{2}:\d{2}$/.test(b.time)
      ? b.time
      : `${String(now.getHours()).padStart(2, "0")}:${String(now.getMinutes()).padStart(2, "0")}`;

  const meal = addMeal(userId, {
    date: typeof b.date === "string" ? b.date : todayISO(),
    time,
    name,
    calories,
    protein_g: Number(b.protein_g) || 0,
    carbs_g: Number(b.carbs_g) || 0,
    fat_g: Number(b.fat_g) || 0,
  });

  const res = NextResponse.json({ meal });
  if (isNew) attachUserCookie(res, userId);
  return res;
}

// DELETE ?id=... - מחיקת ארוחה
export async function DELETE(req: NextRequest) {
  const { userId, isNew } = resolveUser();
  if (isNew)
    return NextResponse.json({ error: "לא נמצאה ארוחה" }, { status: 404 });

  const id = req.nextUrl.searchParams.get("id");
  if (!id)
    return NextResponse.json({ error: "מזהה ארוחה חסר" }, { status: 400 });

  const ok = deleteMeal(userId, id);
  if (!ok)
    return NextResponse.json({ error: "הארוחה לא נמצאה" }, { status: 404 });

  return NextResponse.json({ ok: true });
}

// PATCH - עדכון שעת ארוחה (הזזה לזמן אחר)
export async function PATCH(req: NextRequest) {
  const { userId, isNew } = resolveUser();
  if (isNew)
    return NextResponse.json({ error: "הארוחה לא נמצאה" }, { status: 404 });

  let body: unknown;
  try {
    body = await req.json();
  } catch {
    return NextResponse.json({ error: "גוף הבקשה אינו JSON תקין" }, { status: 400 });
  }

  const b = body as Record<string, unknown>;
  const id = typeof b.id === "string" ? b.id : "";
  const time = typeof b.time === "string" ? b.time : "";
  if (!id)
    return NextResponse.json({ error: "מזהה ארוחה חסר" }, { status: 400 });
  if (!/^\d{2}:\d{2}$/.test(time))
    return NextResponse.json({ error: "שעה לא תקינה (HH:MM)" }, { status: 400 });

  const meal = updateMealTime(userId, id, time);
  if (!meal)
    return NextResponse.json({ error: "הארוחה לא נמצאה" }, { status: 404 });

  return NextResponse.json({ meal });
}
