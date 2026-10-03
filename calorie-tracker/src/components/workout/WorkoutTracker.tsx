"use client";

import { useCallback, useEffect, useRef, useState } from "react";
import {
  AMRAP_ROUND,
  LADDER_TOTAL_REPS,
  exerciseName,
  formatInt,
  formatStopwatch,
  getAmrapPB,
  getLadderPB,
  getEquipment,
  saveEquipment,
  type Equipment,
} from "@/lib/workout";
import { WorkoutAudio } from "@/lib/audio";
import AmrapMode from "./AmrapMode";
import LadderMode from "./LadderMode";

type Screen = "menu" | "amrap" | "ladder";

export default function WorkoutTracker() {
  const [screen, setScreen] = useState<Screen>("menu");
  const [equipment, setEquipment] = useState<Equipment>("full");
  const [muted, setMuted] = useState(false);
  const [amrapPB, setAmrapPB] = useState<number | null>(null);
  const [ladderPB, setLadderPB] = useState<number | null>(null);

  // מופע סאונד יחיד שחי כל עוד הלשונית פתוחה.
  const audioRef = useRef<WorkoutAudio | null>(null);
  if (audioRef.current === null) audioRef.current = new WorkoutAudio();
  const audio = audioRef.current;

  const refreshPBs = useCallback(() => {
    setAmrapPB(getAmrapPB());
    setLadderPB(getLadderPB());
  }, []);

  // טעינת העדפות ושיאים מה-localStorage (צד לקוח בלבד).
  useEffect(() => {
    setEquipment(getEquipment());
    refreshPBs();
  }, [refreshPBs]);

  useEffect(() => {
    audio.setMuted(muted);
  }, [audio, muted]);

  const toggleEquipment = (eq: Equipment) => {
    setEquipment(eq);
    saveEquipment(eq);
  };

  const enter = (s: Exclude<Screen, "menu">) => {
    audio.unlock(); // אינטראקציית משתמש — מרשה סאונד
    setScreen(s);
  };

  const backToMenu = () => {
    refreshPBs();
    setScreen("menu");
  };

  if (screen === "amrap") {
    return (
      <AmrapMode audio={audio} equipment={equipment} onExit={backToMenu} />
    );
  }
  if (screen === "ladder") {
    return (
      <LadderMode audio={audio} equipment={equipment} onExit={backToMenu} />
    );
  }

  // ---- מסך תפריט ----
  const roundDesc = AMRAP_ROUND.map(
    (e) => `${e.reps} ${exerciseName(e.key, equipment).he}`
  ).join(" · ");

  return (
    <main className="max-w-xl mx-auto px-4 py-5 pb-10 flex flex-col gap-5">
      <header className="flex items-center justify-between gap-3">
        <div>
          <h1 className="font-display font-bold text-2xl">אימוני כוח</h1>
          <p className="text-text-muted text-sm">Tom Holland · Calisthenics</p>
        </div>
        <button
          onClick={() => setMuted((m) => !m)}
          aria-pressed={muted}
          aria-label={muted ? "הפעל סאונד" : "השתק סאונד"}
          className="h-10 w-10 rounded-lg border border-border text-lg flex items-center justify-center hover:border-border-2"
        >
          {muted ? "🔇" : "🔊"}
        </button>
      </header>

      {/* מתג ציוד */}
      <section className="rounded-xl border border-border bg-panel p-3">
        <div className="flex items-center justify-between mb-2">
          <span className="font-display font-bold text-sm">ציוד זמין</span>
          <span className="text-text-muted text-xs">
            משנה את שמות התרגילים
          </span>
        </div>
        <div className="grid grid-cols-2 gap-1 rounded-lg border border-border bg-bg p-1">
          <button
            onClick={() => toggleEquipment("full")}
            aria-pressed={equipment === "full"}
            className={`rounded-md py-2 text-sm font-display font-bold transition-colors ${
              equipment === "full"
                ? "bg-neon-cyan text-bg"
                : "text-text-muted hover:text-text-main"
            }`}
          >
            מוט מתח + מקבילים
          </button>
          <button
            onClick={() => toggleEquipment("none")}
            aria-pressed={equipment === "none"}
            className={`rounded-md py-2 text-sm font-display font-bold transition-colors ${
              equipment === "none"
                ? "bg-neon-amber text-bg"
                : "text-text-muted hover:text-text-main"
            }`}
          >
            בלי ציוד
          </button>
        </div>
      </section>

      {/* מצב A */}
      <button
        onClick={() => enter("amrap")}
        className="text-right rounded-2xl border-2 border-neon-cyan/60 bg-panel p-4 flex flex-col gap-2 hover:border-neon-cyan transition-colors active:scale-[0.99]"
      >
        <div className="flex items-center justify-between">
          <span className="font-display font-bold text-xl text-neon-cyan">
            20 דקות AMRAP
          </span>
          <span className="text-text-muted text-sm">Cindy</span>
        </div>
        <p className="text-text-muted text-sm">{roundDesc}</p>
        <p className="text-sm">
          🏆 שיא:{" "}
          <span className="num font-display font-bold text-neon-lime">
            {amrapPB != null ? `${amrapPB} סיבובים` : "—"}
          </span>
        </p>
      </button>

      {/* מצב B */}
      <button
        onClick={() => enter("ladder")}
        className="text-right rounded-2xl border-2 border-neon-pink/60 bg-panel p-4 flex flex-col gap-2 hover:border-neon-pink transition-colors active:scale-[0.99]"
      >
        <div className="flex items-center justify-between">
          <span className="font-display font-bold text-xl text-neon-pink">
            סולם {formatInt(LADDER_TOTAL_REPS)} חזרות
          </span>
          <span className="text-text-muted text-sm">Monster</span>
        </div>
        <p className="text-text-muted text-sm">
          רמות 1→10→1 · מתח ×1, מקבילים ×2, שכיבות ×3, בטן ×4, סקוואט ×5
        </p>
        <p className="text-sm">
          🏆 שיא:{" "}
          <span className="num font-display font-bold text-neon-lime">
            {ladderPB != null ? formatStopwatch(ladderPB) : "—"}
          </span>
        </p>
      </button>
    </main>
  );
}
