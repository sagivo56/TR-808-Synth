import { cookies } from "next/headers";
import { NextResponse } from "next/server";

export const USER_COOKIE = "uid";
const ONE_YEAR = 60 * 60 * 24 * 365;

/**
 * מזהה משתמש נקבע דרך cookie. אם עדיין אין - נוצר מזהה חדש (isNew=true),
 * ואז על ה-route להצמיד אותו לתגובה עם attachUserCookie.
 */
export function resolveUser(): { userId: string; isNew: boolean } {
  const existing = cookies().get(USER_COOKIE)?.value;
  if (existing) return { userId: existing, isNew: false };
  return { userId: crypto.randomUUID(), isNew: true };
}

export function attachUserCookie(res: NextResponse, userId: string): NextResponse {
  res.cookies.set(USER_COOKIE, userId, {
    httpOnly: true,
    sameSite: "lax",
    path: "/",
    maxAge: ONE_YEAR,
  });
  return res;
}
