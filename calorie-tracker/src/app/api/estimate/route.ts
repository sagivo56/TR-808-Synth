import { NextRequest, NextResponse } from "next/server";
import Anthropic from "@anthropic-ai/sdk";
import type { ChatMessage, ChatBlock, EstimateResult } from "@/lib/types";

export const runtime = "nodejs";
export const dynamic = "force-dynamic";

const MODEL = process.env.ANTHROPIC_MODEL || "claude-opus-4-8";

function buildSystemPrompt(muscleGoal: boolean): string {
  const goalLine = muscleGoal
    ? "למשתמש יש מטרה להגדיל מסת שריר. בטיפ, שים דגש על חלבון מספק (בערך 0.3-0.4 גרם חלבון לק\"ג משקל גוף לארוחה, מקורות איכותיים), ועל פחמימות סביב אימונים לתמיכה בהתאוששות."
    : "למשתמש אין מטרה מיוחדת של הגדלת מסת שריר. בטיפ, שים דגש על מאזן כללי ושובע (חלבון וסיבים), בלי להפריז בחלבון.";

  return `אתה עוזר תזונה שמעריך את הערך הקלורי והמאקרו-נוטריאנטים של מנה מתוך טקסט חופשי ו/או תמונה, בעברית.

עליך להחזיר JSON תקין בלבד — ללא טקסט נוסף, ללא הסברים, וללא code fences. אחת משתי הצורות בלבד:

1. אם המנה אינה ברורה מספיק והבהרה תשנה מהותית את ההערכה (למשל גודל מנה לא ידוע, רוטב/שמן לא ברור, כמות לא ידועה) — החזר שאלת הבהרה אחת קצרה בעברית:
{"status":"question","question":"..."}

2. אחרת — החזר הערכה סופית. שדה name בעברית ותיאורי בקצרה. כל הערכים המספריים הם מספרים (לא מחרוזות), לכל המנה כפי שתוארה. שדה tip הוא טיפ קצר (משפט אחד-שניים) בעברית כיצד לשפר את מאזן המאקרו של המנה הזו:
{"status":"result","name":"...","calories":N,"protein_g":N,"carbs_g":N,"fat_g":N,"tip":"..."}

לגבי הטיפ:
- ${goalLine}
- התייחס למאזן של המנה הספציפית (חלבון/פחמימות/שומן) והצע שיפור מעשי אחד או שניים (מה להוסיף/להחליף/להפחית).
- קצר, ידידותי ומעשי. אם המנה כבר מאוזנת היטב — ציין זאת בקצרה.

כללים:
- שאל שאלה רק כאשר זה באמת משנה מהותית את ההערכה. אם ניתן להעריך בהנחות סבירות — העדף להחזיר result.
- אל תשאל יותר משאלה אחת בכל פעם.
- החזר תמיד אובייקט JSON חוקי אחד בלבד, ללא טקסט לפניו או אחריו, וללא אובייקט שני.
- אל תוסיף שדות שלא צוינו למעלה. שמור על שם המנה (name) קצר וסביר.`;
}

// המרת בלוק צד-לקוח לבלוק תוכן של Anthropic, עם ולידציה בסיסית.
function toAnthropicBlock(block: ChatBlock): Anthropic.ContentBlockParam | null {
  if (block?.type === "text" && typeof block.text === "string") {
    return { type: "text", text: block.text };
  }
  if (
    block?.type === "image" &&
    block.source?.type === "base64" &&
    typeof block.source.data === "string" &&
    typeof block.source.media_type === "string"
  ) {
    return {
      type: "image",
      source: {
        type: "base64",
        media_type: block.source.media_type as
          | "image/jpeg"
          | "image/png"
          | "image/gif"
          | "image/webp",
        data: block.source.data,
      },
    };
  }
  return null;
}

// חילוץ אובייקט ה-JSON הראשון והמאוזן מתוך הטקסט, תוך התעלמות מטקסט/אובייקט
// נוסף שהמודל עלול להוסיף אחריו. סורק מאזן סוגריים ומכבד מחרוזות ותווי escape.
function extractJson(text: string): string {
  const trimmed = text.trim();
  const fenced = trimmed.match(/```(?:json)?\s*([\s\S]*?)```/i);
  const candidate = fenced ? fenced[1].trim() : trimmed;

  const start = candidate.indexOf("{");
  if (start === -1) return candidate;

  let depth = 0;
  let inStr = false;
  let esc = false;
  for (let i = start; i < candidate.length; i++) {
    const ch = candidate[i];
    if (inStr) {
      if (esc) esc = false;
      else if (ch === "\\") esc = true;
      else if (ch === '"') inStr = false;
    } else if (ch === '"') {
      inStr = true;
    } else if (ch === "{") {
      depth++;
    } else if (ch === "}") {
      depth--;
      if (depth === 0) return candidate.slice(start, i + 1);
    }
  }
  // לא נמצא אובייקט מאוזן (למשל תשובה שנקטעה) - מחזירים כפי שהוא לצורך שגיאה ברורה
  return candidate.slice(start);
}

function validateResult(obj: unknown): EstimateResult | null {
  if (!obj || typeof obj !== "object") return null;
  const o = obj as Record<string, unknown>;
  if (o.status === "question" && typeof o.question === "string") {
    return { status: "question", question: o.question };
  }
  if (
    o.status === "result" &&
    typeof o.name === "string" &&
    typeof o.calories === "number" &&
    typeof o.protein_g === "number" &&
    typeof o.carbs_g === "number" &&
    typeof o.fat_g === "number"
  ) {
    return {
      status: "result",
      name: o.name,
      calories: o.calories,
      protein_g: o.protein_g,
      carbs_g: o.carbs_g,
      fat_g: o.fat_g,
      tip: typeof o.tip === "string" ? o.tip : undefined,
    };
  }
  return null;
}

export async function POST(req: NextRequest) {
  const apiKey = process.env.ANTHROPIC_API_KEY;
  if (!apiKey) {
    return NextResponse.json(
      {
        error:
          "ANTHROPIC_API_KEY לא הוגדר בשרת. יש להגדיר את משתנה הסביבה (ראה README).",
      },
      { status: 500 }
    );
  }

  let body: unknown;
  try {
    body = await req.json();
  } catch {
    return NextResponse.json({ error: "גוף הבקשה אינו JSON תקין" }, { status: 400 });
  }

  const muscleGoal = (body as { muscle_goal?: boolean })?.muscle_goal === true;

  const messages = (body as { messages?: ChatMessage[] })?.messages;
  if (!Array.isArray(messages) || messages.length === 0) {
    return NextResponse.json(
      { error: "לא נשלחו הודעות (thread ריק)" },
      { status: 400 }
    );
  }

  // בניית ה-messages בפורמט Anthropic
  const apiMessages: Anthropic.MessageParam[] = [];
  for (const m of messages) {
    if (m.role !== "user" && m.role !== "assistant") continue;
    const blocks = (m.content || [])
      .map(toAnthropicBlock)
      .filter((b): b is Anthropic.ContentBlockParam => b !== null);
    if (blocks.length === 0) continue;
    apiMessages.push({ role: m.role, content: blocks });
  }

  if (apiMessages.length === 0 || apiMessages[0].role !== "user") {
    return NextResponse.json(
      { error: "הודעות לא תקינות (על ההודעה הראשונה להיות של המשתמש)" },
      { status: 400 }
    );
  }

  const client = new Anthropic({ apiKey });

  let response: Anthropic.Message;
  try {
    response = await client.messages.create({
      model: MODEL,
      max_tokens: 1000,
      system: buildSystemPrompt(muscleGoal),
      messages: apiMessages,
    });
  } catch (err) {
    // טיפול שגיאות מפורט: קוד סטטוס HTTP והודעת השגיאה האמיתית
    if (err instanceof Anthropic.APIError) {
      const status = err.status ?? 502;
      return NextResponse.json(
        {
          error: `שגיאה מ-Anthropic API (HTTP ${status}): ${err.message}`,
          status,
        },
        { status: 502 }
      );
    }
    if (err instanceof Anthropic.APIConnectionError) {
      return NextResponse.json(
        { error: `שגיאת חיבור ל-Anthropic API: ${err.message}` },
        { status: 502 }
      );
    }
    const msg = err instanceof Error ? err.message : String(err);
    return NextResponse.json(
      { error: `שגיאה בלתי צפויה בקריאה ל-API: ${msg}` },
      { status: 500 }
    );
  }

  // איסוף הטקסט מהתשובה
  const rawText = response.content
    .filter((b): b is Anthropic.TextBlock => b.type === "text")
    .map((b) => b.text)
    .join("")
    .trim();

  if (!rawText) {
    return NextResponse.json(
      {
        error: `המודל החזיר תשובה ללא טקסט (stop_reason: ${response.stop_reason ?? "לא ידוע"})`,
      },
      { status: 502 }
    );
  }

  const jsonStr = extractJson(rawText);
  let parsed: unknown;
  try {
    parsed = JSON.parse(jsonStr);
  } catch (e) {
    // שגיאת פענוח JSON - להציג את הסיבה האמיתית ואת הטקסט שהתקבל
    const msg = e instanceof Error ? e.message : String(e);
    return NextResponse.json(
      {
        error: `כשל בפענוח JSON מתשובת המודל: ${msg}`,
        raw: rawText.slice(0, 500),
      },
      { status: 502 }
    );
  }

  const result = validateResult(parsed);
  if (!result) {
    return NextResponse.json(
      {
        error: "תשובת המודל אינה בפורמט הצפוי (status question/result)",
        raw: rawText.slice(0, 500),
      },
      { status: 502 }
    );
  }

  return NextResponse.json({ result });
}
