"use client";

import type { Meal } from "@/lib/types";

export default function MealList({
  meals,
  onDelete,
}: {
  meals: Meal[];
  onDelete: (id: string) => void;
}) {
  return (
    <div className="rounded-2xl border border-border bg-panel p-4 sm:p-5">
      <h2 className="font-display font-bold text-lg mb-3">הארוחות של היום</h2>
      {meals.length === 0 ? (
        <p className="text-text-muted text-sm py-4 text-center">
          עדיין לא הוזנו ארוחות היום.
        </p>
      ) : (
        <ul className="flex flex-col gap-2">
          {meals.map((m) => (
            <li
              key={m.id}
              className="flex items-center gap-3 rounded-xl border border-border bg-bg/40 px-3 py-2.5"
            >
              <span className="num text-text-muted text-xs w-10 shrink-0">
                {m.time}
              </span>
              <div className="flex-1 min-w-0">
                <div className="truncate text-sm">{m.name}</div>
                <div className="text-text-muted text-[11px] mt-0.5">
                  <span className="num">{Math.round(m.protein_g)}</span> ח׳ ·{" "}
                  <span className="num">{Math.round(m.carbs_g)}</span> פ׳ ·{" "}
                  <span className="num">{Math.round(m.fat_g)}</span> ש׳
                </div>
              </div>
              <div className="font-display text-base shrink-0">
                <span className="num">{Math.round(m.calories)}</span>
                <span className="text-text-muted text-xs"> קק"ל</span>
              </div>
              <button
                onClick={() => onDelete(m.id)}
                aria-label="מחיקת ארוחה"
                className="shrink-0 h-8 w-8 rounded-lg border border-border-2 text-text-muted hover:text-meter-red hover:border-meter-red transition-colors flex items-center justify-center"
              >
                ✕
              </button>
            </li>
          ))}
        </ul>
      )}
    </div>
  );
}
