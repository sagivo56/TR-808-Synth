"use client";

import { useState } from "react";
import type { Meal } from "@/lib/types";
import {
  shiftDateISO,
  isFutureLocal,
  dayLabel,
  localTimeHM,
} from "@/lib/date";

export default function MealList({
  meals,
  today,
  dayLabel: headerDayLabel,
  onDelete,
  onEditMeal,
}: {
  meals: Meal[];
  today: string;
  dayLabel: string;
  onDelete: (id: string) => void;
  onEditMeal: (id: string, time: string, date: string) => void;
}) {
  const [editingId, setEditingId] = useState<string | null>(null);
  const [editTime, setEditTime] = useState("");
  const [editDate, setEditDate] = useState("");
  const nowHM = localTimeHM();

  function startEdit(m: Meal) {
    setEditingId(m.id);
    setEditTime(m.time);
    setEditDate(m.date);
  }

  function commitEdit(m: Meal) {
    if (isFutureLocal(editDate, editTime)) return; // מניעת זמן עתידי
    setEditingId(null);
    if (editTime !== m.time || editDate !== m.date) {
      onEditMeal(m.id, editTime, editDate);
    }
  }

  const future = isFutureLocal(editDate, editTime);
  const canGoForward = editDate < today; // אי אפשר קדימה לעתיד

  return (
    <div className="rounded-2xl border border-border bg-panel p-4 sm:p-5">
      <h2 className="font-display font-bold text-lg mb-3">
        הארוחות — {headerDayLabel}
      </h2>
      {meals.length === 0 ? (
        <p className="text-text-muted text-sm py-4 text-center">
          לא הוזנו ארוחות ביום זה.
        </p>
      ) : (
        <ul className="flex flex-col gap-2">
          {meals.map((m) => (
            <li
              key={m.id}
              className="rounded-xl border border-border bg-bg/40 px-3 py-2.5"
            >
              {editingId === m.id ? (
                /* עורך שעה + יום (הזזה לזמן אחר, ללא עתיד) */
                <div className="flex flex-col gap-2">
                  <div className="truncate text-sm">{m.name}</div>
                  <div className="flex items-center gap-2 flex-wrap">
                    <input
                      type="time"
                      autoFocus
                      value={editTime}
                      max={editDate === today ? nowHM : undefined}
                      onChange={(e) => setEditTime(e.target.value)}
                      className="num w-[4.8rem] bg-bg border border-meter-green rounded-md px-1 py-1 text-xs text-text-main outline-none"
                    />
                    <div className="flex items-center gap-1 rounded-lg border border-border-2 px-1 py-0.5">
                      <button
                        onClick={() => setEditDate((d) => shiftDateISO(d, -1))}
                        className="px-2 py-1 text-sm text-text-main hover:text-meter-green"
                        aria-label="יום אחורה"
                      >
                        −יום
                      </button>
                      <span className="num text-[11px] text-text-muted min-w-[3.5rem] text-center">
                        {dayLabel(editDate)}
                      </span>
                      <button
                        onClick={() => setEditDate((d) => shiftDateISO(d, 1))}
                        disabled={!canGoForward}
                        className="px-2 py-1 text-sm text-text-main hover:text-meter-green disabled:opacity-30 disabled:hover:text-text-main"
                        aria-label="יום קדימה"
                      >
                        +יום
                      </button>
                    </div>
                    <button
                      onClick={() => commitEdit(m)}
                      disabled={future}
                      aria-label="שמירה"
                      className="h-8 w-8 rounded-lg border border-border-2 text-meter-green disabled:opacity-40 flex items-center justify-center"
                    >
                      ✓
                    </button>
                    <button
                      onClick={() => setEditingId(null)}
                      aria-label="ביטול"
                      className="h-8 w-8 rounded-lg border border-border-2 text-text-muted flex items-center justify-center"
                    >
                      ✕
                    </button>
                  </div>
                  {future && (
                    <span className="text-meter-red text-[11px]">
                      אי אפשר לקבוע זמן עתידי
                    </span>
                  )}
                </div>
              ) : (
                /* שורה רגילה */
                <div className="flex items-center gap-3">
                  <button
                    onClick={() => startEdit(m)}
                    aria-label="שינוי שעה / יום"
                    className="num text-text-muted text-xs w-[4.6rem] shrink-0 flex items-center gap-1 hover:text-text-main"
                  >
                    {m.time}
                    <span aria-hidden className="text-[10px]">
                      ✎
                    </span>
                  </button>
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
                </div>
              )}
            </li>
          ))}
        </ul>
      )}
    </div>
  );
}
