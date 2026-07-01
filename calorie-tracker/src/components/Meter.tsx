"use client";

import type { DaySummary } from "@/lib/types";

const SEGMENTS = 28;

function colorForPosition(posPercent: number): string {
  if (posPercent <= 85) return "#8FBF6B"; // ירוק
  if (posPercent <= 100) return "#E0A94C"; // כתום
  return "#D9634B"; // אדום
}

export default function Meter({
  totals,
  target,
}: {
  totals: DaySummary["totals"];
  target: number;
}) {
  const cals = Math.round(totals.calories);
  const pct = target > 0 ? (cals / target) * 100 : 0;
  const over = pct > 100;
  const remaining = target - cals;

  // כמות מקטעים דלוקים
  const litCount = over
    ? SEGMENTS
    : Math.min(SEGMENTS, Math.max(0, Math.round((pct / 100) * SEGMENTS)));

  const segments = Array.from({ length: SEGMENTS }, (_, i) => {
    const lit = i < litCount;
    const posPercent = ((i + 1) / SEGMENTS) * 100;
    // אם חריגה מעל 100% - כל הפס אדום
    const color = over ? "#D9634B" : colorForPosition(posPercent);
    return { lit, color };
  });

  return (
    <div className="rounded-2xl border border-border bg-panel p-4 sm:p-5">
      {/* פס מפולח בסגנון VU meter */}
      <div
        className="flex gap-[3px] h-9 sm:h-11"
        role="meter"
        aria-valuenow={cals}
        aria-valuemin={0}
        aria-valuemax={target}
        aria-label="אחוז מהיעד היומי"
      >
        {segments.map((s, i) => (
          <div
            key={i}
            className="flex-1 rounded-[2px] transition-colors duration-200"
            style={{
              backgroundColor: s.lit ? s.color : "#242A30",
              boxShadow: s.lit ? `0 0 6px ${s.color}66` : "none",
            }}
          />
        ))}
      </div>

      {/* סיכום מספרי */}
      <div className="mt-4 flex items-end justify-between gap-3 flex-wrap">
        <div>
          <div className="text-text-muted text-xs mb-1">סה"כ קלוריות היום</div>
          <div className="font-display text-3xl sm:text-4xl leading-none">
            <span className="num">{cals}</span>
            <span className="text-text-muted text-base mr-1"> / </span>
            <span className="num text-text-muted text-lg">{target}</span>
          </div>
        </div>
        <div className="text-left">
          {over ? (
            <div>
              <div className="text-meter-red text-xs mb-1">חריגה</div>
              <div className="font-display text-2xl sm:text-3xl text-meter-red leading-none">
                <span className="num">{Math.abs(remaining)}</span>
                <span className="text-sm"> קק"ל</span>
              </div>
            </div>
          ) : (
            <div>
              <div className="text-text-muted text-xs mb-1">נשארו</div>
              <div className="font-display text-2xl sm:text-3xl text-meter-green leading-none">
                <span className="num">{remaining}</span>
                <span className="text-sm"> קק"ל</span>
              </div>
            </div>
          )}
        </div>
      </div>

      {/* מאקרו: חלבון / פחמימות / שומן */}
      <div className="mt-4 grid grid-cols-3 gap-2">
        <MacroBox label="חלבון" value={totals.protein_g} />
        <MacroBox label="פחמימות" value={totals.carbs_g} />
        <MacroBox label="שומן" value={totals.fat_g} />
      </div>
    </div>
  );
}

function MacroBox({ label, value }: { label: string; value: number }) {
  return (
    <div className="rounded-xl border border-border bg-bg/40 px-3 py-2 text-center">
      <div className="text-text-muted text-[11px] mb-0.5">{label}</div>
      <div className="font-display text-lg">
        <span className="num">{Math.round(value)}</span>
        <span className="text-text-muted text-xs"> ג׳</span>
      </div>
    </div>
  );
}
