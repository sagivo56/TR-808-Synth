"use client";

import { useState } from "react";
import type { Settings, Sex, Goal } from "@/lib/types";
import { ACTIVITY_LEVELS, GOALS, SEX_OPTIONS, computeTarget } from "@/lib/calc";

export default function SetupForm({
  initial,
  onSaved,
  onClose,
}: {
  initial: Settings | null;
  onSaved: (s: Settings) => void;
  onClose?: () => void;
}) {
  const [height, setHeight] = useState(initial ? String(initial.height_cm) : "");
  const [weight, setWeight] = useState(initial ? String(initial.weight_kg) : "");
  const [age, setAge] = useState(initial ? String(initial.age) : "");
  const [sex, setSex] = useState<Sex>(initial?.sex ?? "male");
  const [activity, setActivity] = useState<number>(initial?.activity ?? 1.375);
  const [goal, setGoal] = useState<Goal>(initial?.goal ?? "maintain");
  const [muscleGoal, setMuscleGoal] = useState<boolean>(
    initial?.muscle_goal ?? false
  );
  const [saving, setSaving] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const h = Number(height);
  const w = Number(weight);
  const a = Number(age);
  const valid = h > 0 && h < 300 && w > 0 && w < 500 && a > 0 && a < 130;
  const preview = valid
    ? computeTarget({ height_cm: h, weight_kg: w, age: a, sex, activity, goal })
    : null;

  async function submit() {
    if (!valid) {
      setError("יש למלא גובה, משקל וגיל תקינים.");
      return;
    }
    setSaving(true);
    setError(null);
    try {
      const res = await fetch("/api/settings", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          height_cm: h,
          weight_kg: w,
          age: a,
          sex,
          activity,
          goal,
          muscle_goal: muscleGoal,
        }),
      });
      const data = await res.json();
      if (!res.ok) throw new Error(data.error || `שגיאה (HTTP ${res.status})`);
      onSaved(data.settings as Settings);
    } catch (e) {
      setError(e instanceof Error ? e.message : "שגיאה בשמירה");
    } finally {
      setSaving(false);
    }
  }

  return (
    <div
      className="fixed inset-0 z-50 flex items-start sm:items-center justify-center bg-black/70 backdrop-blur-sm overflow-y-auto p-4"
      style={{
        paddingTop: "max(1rem, env(safe-area-inset-top))",
        paddingBottom: "max(1rem, env(safe-area-inset-bottom))",
      }}
    >
      <div className="w-full max-w-md rounded-2xl border border-border-2 bg-panel p-5 my-4">
        <div className="flex items-center justify-between mb-4">
          <h2 className="font-display font-bold text-xl">
            {initial ? "עדכון הגדרות" : "הגדרה ראשונית"}
          </h2>
          {onClose && (
            <button
              onClick={onClose}
              aria-label="סגירה"
              className="h-8 w-8 rounded-lg border border-border-2 text-text-muted hover:text-text-main flex items-center justify-center"
            >
              ✕
            </button>
          )}
        </div>

        <div className="grid grid-cols-3 gap-3 mb-3">
          <Field label='גובה (ס"מ)'>
            <input
              type="number"
              inputMode="numeric"
              value={height}
              onChange={(e) => setHeight(e.target.value)}
              className="input num"
              placeholder="175"
            />
          </Field>
          <Field label='משקל (ק"ג)'>
            <input
              type="number"
              inputMode="decimal"
              value={weight}
              onChange={(e) => setWeight(e.target.value)}
              className="input num"
              placeholder="70"
            />
          </Field>
          <Field label="גיל">
            <input
              type="number"
              inputMode="numeric"
              value={age}
              onChange={(e) => setAge(e.target.value)}
              className="input num"
              placeholder="30"
            />
          </Field>
        </div>

        <Field label="מין">
          <div className="grid grid-cols-3 gap-2">
            {SEX_OPTIONS.map((o) => (
              <Choice
                key={o.value}
                active={sex === o.value}
                onClick={() => setSex(o.value)}
              >
                {o.label}
              </Choice>
            ))}
          </div>
        </Field>

        <Field label="רמת פעילות">
          <select
            value={activity}
            onChange={(e) => setActivity(Number(e.target.value))}
            className="input"
          >
            {ACTIVITY_LEVELS.map((l) => (
              <option key={l.value} value={l.value}>
                {l.label} ({l.value})
              </option>
            ))}
          </select>
        </Field>

        <Field label="מטרה">
          <div className="grid grid-cols-1 gap-2">
            {GOALS.map((g) => (
              <Choice
                key={g.value}
                active={goal === g.value}
                onClick={() => setGoal(g.value)}
              >
                {g.label}
              </Choice>
            ))}
          </div>
        </Field>

        <Field label="מטרה: הגדלת מסת שריר">
          <div className="grid grid-cols-2 gap-2">
            <Choice active={muscleGoal} onClick={() => setMuscleGoal(true)}>
              כן
            </Choice>
            <Choice active={!muscleGoal} onClick={() => setMuscleGoal(false)}>
              לא
            </Choice>
          </div>
        </Field>

        {preview !== null && (
          <div className="mt-4 rounded-xl border border-border bg-bg/40 px-4 py-3 flex items-center justify-between">
            <span className="text-text-muted text-sm">יעד קלורי יומי מחושב</span>
            <span className="font-display text-2xl text-meter-green">
              <span className="num">{preview}</span>
              <span className="text-sm text-text-muted"> קק"ל</span>
            </span>
          </div>
        )}

        {error && (
          <p className="mt-3 text-meter-red text-sm text-center">{error}</p>
        )}

        <button
          onClick={submit}
          disabled={saving || !valid}
          className="mt-4 w-full rounded-xl bg-meter-green text-bg font-display font-bold py-3 disabled:opacity-40 transition-opacity"
        >
          {saving ? "שומר..." : initial ? "עדכון וחישוב מחדש" : "התחלה"}
        </button>
      </div>

      <style jsx>{`
        .input {
          width: 100%;
          background: #14171a;
          border: 1px solid #3a4046;
          border-radius: 0.6rem;
          padding: 0.55rem 0.7rem;
          color: #edede6;
          font-size: 0.95rem;
          outline: none;
        }
        .input:focus {
          border-color: #8fbf6b;
        }
      `}</style>
    </div>
  );
}

function Field({
  label,
  children,
}: {
  label: string;
  children: React.ReactNode;
}) {
  return (
    <label className="block mb-3">
      <span className="block text-text-muted text-xs mb-1.5">{label}</span>
      {children}
    </label>
  );
}

function Choice({
  active,
  onClick,
  children,
}: {
  active: boolean;
  onClick: () => void;
  children: React.ReactNode;
}) {
  return (
    <button
      type="button"
      onClick={onClick}
      className={`rounded-lg border px-3 py-2 text-sm transition-colors ${
        active
          ? "border-meter-green bg-meter-green/15 text-text-main"
          : "border-border-2 text-text-muted hover:text-text-main"
      }`}
    >
      {children}
    </button>
  );
}
