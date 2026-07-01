"use client";

import { useRef, useState } from "react";
import type { ChatMessage, ChatBlock, EstimateResult } from "@/lib/types";

const MAX_DIM = 1024;
const JPEG_QUALITY = 0.82;

// הקטנה ודחיסה של תמונה בצד לקוח לפני שליחה (מקס' 1024px, JPEG ~0.82)
async function compressImage(
  file: File
): Promise<{ data: string; media_type: string; dataUrl: string }> {
  const dataUrl = await new Promise<string>((resolve, reject) => {
    const reader = new FileReader();
    reader.onload = () => resolve(reader.result as string);
    reader.onerror = () => reject(new Error("קריאת הקובץ נכשלה"));
    reader.readAsDataURL(file);
  });

  const img = await new Promise<HTMLImageElement>((resolve, reject) => {
    const i = new Image();
    i.onload = () => resolve(i);
    i.onerror = () => reject(new Error("טעינת התמונה נכשלה"));
    i.src = dataUrl;
  });

  let { width, height } = img;
  if (width > MAX_DIM || height > MAX_DIM) {
    const scale = MAX_DIM / Math.max(width, height);
    width = Math.round(width * scale);
    height = Math.round(height * scale);
  }

  const canvas = document.createElement("canvas");
  canvas.width = width;
  canvas.height = height;
  const ctx = canvas.getContext("2d");
  if (!ctx) throw new Error("הדפדפן אינו תומך בעיבוד תמונה");
  ctx.drawImage(img, 0, 0, width, height);

  const outUrl = canvas.toDataURL("image/jpeg", JPEG_QUALITY);
  const data = outUrl.split(",")[1] ?? "";
  return { data, media_type: "image/jpeg", dataUrl: outUrl };
}

function blockImageUrl(b: ChatBlock): string | null {
  if (b.type === "image" && b.source.type === "base64") {
    return `data:${b.source.media_type};base64,${b.source.data}`;
  }
  return null;
}

export default function Chat({
  onAddMeal,
}: {
  onAddMeal: (r: Extract<EstimateResult, { status: "result" }>) => Promise<void>;
}) {
  const [thread, setThread] = useState<ChatMessage[]>([]);
  const [input, setInput] = useState("");
  const [pendingImage, setPendingImage] = useState<{
    data: string;
    media_type: string;
    dataUrl: string;
  } | null>(null);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);
  const [result, setResult] = useState<
    Extract<EstimateResult, { status: "result" }> | null
  >(null);
  const [adding, setAdding] = useState(false);
  const fileRef = useRef<HTMLInputElement>(null);

  async function pickImage(e: React.ChangeEvent<HTMLInputElement>) {
    const file = e.target.files?.[0];
    e.target.value = ""; // מאפשר לבחור שוב את אותו קובץ
    if (!file) return;
    setError(null);
    try {
      const compressed = await compressImage(file);
      setPendingImage(compressed);
    } catch (err) {
      setError(err instanceof Error ? err.message : "עיבוד התמונה נכשל");
    }
  }

  async function runEstimate(currentThread: ChatMessage[]) {
    setLoading(true);
    setError(null);
    try {
      const res = await fetch("/api/estimate", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ messages: currentThread }),
      });
      let data: {
        result?: EstimateResult;
        error?: string;
        raw?: string;
        status?: number;
      };
      try {
        data = await res.json();
      } catch {
        throw new Error(`תשובת השרת אינה JSON תקין (HTTP ${res.status})`);
      }
      if (!res.ok || !data.result) {
        const detail = data.raw ? `\n(פלט המודל: ${data.raw})` : "";
        throw new Error((data.error || `שגיאה (HTTP ${res.status})`) + detail);
      }

      const r = data.result;
      if (r.status === "question") {
        // המשך ה-thread: מוסיפים את שאלת המודל כהודעת assistant
        setThread((t) => [
          ...t,
          { role: "assistant", content: [{ type: "text", text: r.question }] },
        ]);
      } else {
        // תוצאה סופית
        setResult(r);
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : "שגיאה בהערכה");
    } finally {
      setLoading(false);
    }
  }

  async function send() {
    const text = input.trim();
    if (!text && !pendingImage) return;
    if (loading) return;

    const blocks: ChatBlock[] = [];
    if (pendingImage) {
      blocks.push({
        type: "image",
        source: {
          type: "base64",
          media_type: pendingImage.media_type,
          data: pendingImage.data,
        },
      });
    }
    if (text) blocks.push({ type: "text", text });

    const userMsg: ChatMessage = { role: "user", content: blocks };
    const next = [...thread, userMsg];
    setThread(next);
    setInput("");
    setPendingImage(null);
    setResult(null);
    await runEstimate(next);
  }

  function retry() {
    if (thread.length === 0) return;
    runEstimate(thread);
  }

  function reset() {
    setThread([]);
    setInput("");
    setPendingImage(null);
    setResult(null);
    setError(null);
  }

  async function addMeal() {
    if (!result) return;
    setAdding(true);
    try {
      await onAddMeal(result);
      reset();
    } catch (err) {
      setError(err instanceof Error ? err.message : "הוספת הארוחה נכשלה");
    } finally {
      setAdding(false);
    }
  }

  const hasConversation = thread.length > 0 || result !== null;

  return (
    <div className="rounded-2xl border border-border bg-panel p-4 sm:p-5 flex flex-col">
      <div className="flex items-center justify-between mb-3">
        <h2 className="font-display font-bold text-lg">דיווח על אוכל</h2>
        {hasConversation && (
          <button
            onClick={reset}
            className="text-text-muted text-xs hover:text-text-main"
          >
            שיחה חדשה
          </button>
        )}
      </div>

      {/* היסטוריית השיחה */}
      {thread.length > 0 && (
        <div className="flex flex-col gap-2 mb-3 max-h-72 overflow-y-auto">
          {thread.map((m, i) => (
            <div
              key={i}
              className={`flex ${m.role === "user" ? "justify-start" : "justify-end"}`}
            >
              <div
                className={`max-w-[85%] rounded-2xl px-3 py-2 text-sm ${
                  m.role === "user"
                    ? "bg-bg/50 border border-border"
                    : "bg-meter-amber/15 border border-meter-amber/40"
                }`}
              >
                {m.content.map((b, j) => {
                  const url = blockImageUrl(b);
                  if (url) {
                    return (
                      // eslint-disable-next-line @next/next/no-img-element
                      <img
                        key={j}
                        src={url}
                        alt="תמונת ארוחה"
                        className="rounded-lg max-h-40 mb-1"
                      />
                    );
                  }
                  if (b.type === "text")
                    return (
                      <p key={j} className="whitespace-pre-wrap">
                        {b.text}
                      </p>
                    );
                  return null;
                })}
              </div>
            </div>
          ))}
        </div>
      )}

      {loading && (
        <div className="text-text-muted text-sm mb-3 flex items-center gap-2">
          <span className="inline-block h-2 w-2 rounded-full bg-meter-amber animate-pulse" />
          מעריך...
        </div>
      )}

      {/* תוצאה סופית */}
      {result && (
        <div className="mb-3 rounded-xl border border-meter-green/50 bg-meter-green/10 p-4">
          <div className="font-display font-bold text-base mb-2">
            {result.name}
          </div>
          <div className="grid grid-cols-4 gap-2 text-center mb-3">
            <ResCell label='קק"ל' value={Math.round(result.calories)} big />
            <ResCell label="חלבון" value={Math.round(result.protein_g)} />
            <ResCell label="פחמ׳" value={Math.round(result.carbs_g)} />
            <ResCell label="שומן" value={Math.round(result.fat_g)} />
          </div>
          <button
            onClick={addMeal}
            disabled={adding}
            className="w-full rounded-lg bg-meter-green text-bg font-display font-bold py-2.5 disabled:opacity-50"
          >
            {adding ? "מוסיף..." : "הוספה ליומן"}
          </button>
        </div>
      )}

      {error && (
        <div className="mb-3 rounded-xl border border-meter-red/50 bg-meter-red/10 p-3">
          <p className="text-meter-red text-sm whitespace-pre-wrap">{error}</p>
          {thread.length > 0 && !result && (
            <button
              onClick={retry}
              className="mt-2 text-xs text-text-main underline"
            >
              נסה שוב
            </button>
          )}
        </div>
      )}

      {/* תצוגה מקדימה של תמונה מצורפת */}
      {pendingImage && (
        <div className="mb-2 flex items-center gap-2">
          {/* eslint-disable-next-line @next/next/no-img-element */}
          <img
            src={pendingImage.dataUrl}
            alt="תצוגה מקדימה"
            className="h-14 w-14 object-cover rounded-lg border border-border"
          />
          <span className="text-text-muted text-xs flex-1">תמונה מצורפת</span>
          <button
            onClick={() => setPendingImage(null)}
            className="text-text-muted hover:text-meter-red text-sm"
          >
            הסר
          </button>
        </div>
      )}

      {/* שורת קלט */}
      <div className="flex items-end gap-2">
        <input
          ref={fileRef}
          type="file"
          accept="image/*"
          capture="environment"
          onChange={pickImage}
          className="hidden"
        />
        <button
          onClick={() => fileRef.current?.click()}
          aria-label="צירוף תמונה"
          className="shrink-0 h-11 w-11 rounded-xl border border-border-2 text-text-muted hover:text-text-main flex items-center justify-center text-lg"
        >
          📷
        </button>
        <textarea
          value={input}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={(e) => {
            if (e.key === "Enter" && !e.shiftKey) {
              e.preventDefault();
              send();
            }
          }}
          rows={1}
          placeholder={
            thread.length > 0 ? "תשובה / פירוט נוסף..." : "מה אכלת? (טקסט חופשי)"
          }
          className="flex-1 resize-none bg-bg border border-border-2 rounded-xl px-3 py-2.5 text-sm outline-none focus:border-meter-green max-h-32"
        />
        <button
          onClick={send}
          disabled={loading || (!input.trim() && !pendingImage)}
          className="shrink-0 h-11 px-4 rounded-xl bg-meter-amber text-bg font-display font-bold disabled:opacity-40"
        >
          שלח
        </button>
      </div>
    </div>
  );
}

function ResCell({
  label,
  value,
  big,
}: {
  label: string;
  value: number;
  big?: boolean;
}) {
  return (
    <div className="rounded-lg bg-bg/40 border border-border py-1.5">
      <div
        className={`font-display ${big ? "text-xl text-meter-green" : "text-base"}`}
      >
        <span className="num">{value}</span>
      </div>
      <div className="text-text-muted text-[10px]">{label}</div>
    </div>
  );
}
