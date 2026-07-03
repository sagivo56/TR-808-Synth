"use client";

import { useEffect, useMemo, useState } from "react";
import type { Settings, Meal, EstimateResult } from "@/lib/types";
import SetupForm from "@/components/SetupForm";
import Meter from "@/components/Meter";
import MealList from "@/components/MealList";
import Chat from "@/components/Chat";

// מיון ארוחות לפי שעה (ואז לפי סדר הוספה)
function byTime(a: Meal, b: Meal): number {
  return a.time === b.time
    ? a.created_at - b.created_at
    : a.time < b.time
      ? -1
      : 1;
}

export default function Page() {
  const [loading, setLoading] = useState(true);
  const [settings, setSettings] = useState<Settings | null>(null);
  const [meals, setMeals] = useState<Meal[]>([]);
  const [showSetup, setShowSetup] = useState(false);
  const [editingTarget, setEditingTarget] = useState(false);
  const [targetInput, setTargetInput] = useState("");

  useEffect(() => {
    (async () => {
      try {
        const [sRes, mRes] = await Promise.all([
          fetch("/api/settings"),
          fetch("/api/meals"),
        ]);
        const sData = await sRes.json();
        const mData = await mRes.json();
        setSettings(sData.settings ?? null);
        setMeals(mData.meals ?? []);
      } catch {
        // מוצג מסך הגדרה במקרה של כשל טעינה
      } finally {
        setLoading(false);
      }
    })();
  }, []);

  const totals = useMemo(() => {
    return meals.reduce(
      (acc, m) => ({
        calories: acc.calories + m.calories,
        protein_g: acc.protein_g + m.protein_g,
        carbs_g: acc.carbs_g + m.carbs_g,
        fat_g: acc.fat_g + m.fat_g,
      }),
      { calories: 0, protein_g: 0, carbs_g: 0, fat_g: 0 }
    );
  }, [meals]);

  async function handleAddMeal(
    r: Extract<EstimateResult, { status: "result" }>
  ) {
    const res = await fetch("/api/meals", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        name: r.name,
        calories: r.calories,
        protein_g: r.protein_g,
        carbs_g: r.carbs_g,
        fat_g: r.fat_g,
      }),
    });
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || `שגיאה (HTTP ${res.status})`);
    setMeals((prev) => [...prev, data.meal as Meal].sort(byTime));
  }

  async function handleDeleteMeal(id: string) {
    const prev = meals;
    setMeals((m) => m.filter((x) => x.id !== id)); // עדכון אופטימי
    const res = await fetch(`/api/meals?id=${encodeURIComponent(id)}`, {
      method: "DELETE",
    });
    if (!res.ok) setMeals(prev); // החזרה במקרה כשל
  }

  async function handleEditMealTime(id: string, time: string) {
    const prev = meals;
    // עדכון אופטימי + מיון מחדש לפי שעה
    setMeals((m) =>
      m.map((x) => (x.id === id ? { ...x, time } : x)).sort(byTime)
    );
    const res = await fetch("/api/meals", {
      method: "PATCH",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ id, time }),
    });
    if (!res.ok) setMeals(prev); // החזרה במקרה כשל
  }

  async function saveManualTarget() {
    const val = Math.round(Number(targetInput));
    if (!(val >= 1000 && val <= 20000)) {
      setEditingTarget(false);
      return;
    }
    const res = await fetch("/api/settings", {
      method: "PATCH",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ target: val }),
    });
    const data = await res.json();
    if (res.ok) setSettings(data.settings);
    setEditingTarget(false);
  }

  if (loading) {
    return (
      <main className="min-h-dvh flex items-center justify-center">
        <div className="text-text-muted font-display animate-pulse">
          טוען...
        </div>
      </main>
    );
  }

  // הגדרה ראשונית חובה
  if (!settings) {
    return (
      <SetupForm
        initial={null}
        onSaved={(s) => {
          setSettings(s);
        }}
      />
    );
  }

  return (
    <main className="max-w-xl mx-auto px-4 py-5 pb-10 flex flex-col gap-4">
      {/* כותרת + יעד + הגדרות */}
      <header className="flex items-center justify-between gap-3">
        <h1 className="font-display font-bold text-2xl">יומן קלוריות</h1>
        <div className="flex items-center gap-2">
          {editingTarget ? (
            <div className="flex items-center gap-1">
              <input
                type="number"
                autoFocus
                value={targetInput}
                onChange={(e) => setTargetInput(e.target.value)}
                onKeyDown={(e) => {
                  if (e.key === "Enter") saveManualTarget();
                  if (e.key === "Escape") setEditingTarget(false);
                }}
                className="num w-24 bg-bg border border-meter-green rounded-lg px-2 py-1 text-sm outline-none"
              />
              <button
                onClick={saveManualTarget}
                className="text-meter-green text-sm px-2"
                aria-label="שמירת יעד"
              >
                ✓
              </button>
            </div>
          ) : (
            <button
              onClick={() => {
                setTargetInput(String(settings.target));
                setEditingTarget(true);
              }}
              className="flex items-center gap-1 rounded-lg border border-border px-3 py-1.5 hover:border-border-2"
              aria-label="עריכת יעד ידנית"
            >
              <span className="text-text-muted text-xs">יעד</span>
              <span className="font-display text-base num">
                {settings.target}
              </span>
              <span className="text-text-muted text-xs">✎</span>
            </button>
          )}
          <button
            onClick={() => setShowSetup(true)}
            aria-label="פתיחת הגדרות"
            className="h-9 w-9 rounded-lg border border-border text-text-muted hover:text-text-main flex items-center justify-center"
          >
            ⚙
          </button>
        </div>
      </header>

      <Meter totals={totals} target={settings.target} />

      <Chat onAddMeal={handleAddMeal} muscleGoal={settings.muscle_goal} />

      <MealList
        meals={meals}
        onDelete={handleDeleteMeal}
        onEditTime={handleEditMealTime}
      />

      {showSetup && (
        <SetupForm
          initial={settings}
          onSaved={(s) => {
            setSettings(s);
            setShowSetup(false);
          }}
          onClose={() => setShowSetup(false)}
        />
      )}
    </main>
  );
}
