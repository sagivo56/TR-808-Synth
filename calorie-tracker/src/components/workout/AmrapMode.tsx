"use client";

import { useEffect, useRef, useState } from "react";
import {
  AMRAP_ROUND,
  AMRAP_REPS_PER_ROUND,
  AMRAP_SECONDS,
  exerciseName,
  formatClock,
  saveAmrapPB,
  getAmrapPB,
  type Equipment,
} from "@/lib/workout";
import type { WorkoutAudio } from "@/lib/audio";
import { useWakeLock } from "./useWakeLock";

type Phase = "ready" | "running" | "done";

interface Props {
  audio: WorkoutAudio;
  equipment: Equipment;
  onExit: () => void;
}

export default function AmrapMode({ audio, equipment, onExit }: Props) {
  const [phase, setPhase] = useState<Phase>("ready");
  const [rounds, setRounds] = useState(0);
  const [msLeft, setMsLeft] = useState(AMRAP_SECONDS * 1000);
  const [isPB, setIsPB] = useState(false);

  const endTimeRef = useRef(0);
  const prevTickRef = useRef(-1);
  const roundsRef = useRef(0);
  const finishedRef = useRef(false);
  const intervalRef = useRef<ReturnType<typeof setInterval> | null>(null);

  useWakeLock(phase === "running");

  const stopInterval = () => {
    if (intervalRef.current) {
      clearInterval(intervalRef.current);
      intervalRef.current = null;
    }
  };

  // ניקוי בעת פירוק הרכיב
  useEffect(() => stopInterval, []);

  const finishWorkout = () => {
    if (finishedRef.current) return;
    finishedRef.current = true;
    stopInterval();
    const finalRounds = roundsRef.current;
    setMsLeft(0);
    setPhase("done");
    const pb = saveAmrapPB(finalRounds);
    setIsPB(pb && finalRounds > 0);
    if (finalRounds > 0) audio.finish();
    else audio.gameOver();
  };

  const start = () => {
    audio.unlock();
    roundsRef.current = 0;
    finishedRef.current = false;
    prevTickRef.current = -1;
    setRounds(0);
    setIsPB(false);
    endTimeRef.current = performance.now() + AMRAP_SECONDS * 1000;
    setMsLeft(AMRAP_SECONDS * 1000);
    setPhase("running");

    stopInterval();
    intervalRef.current = setInterval(() => {
      const remaining = endTimeRef.current - performance.now();
      if (remaining <= 0) {
        finishWorkout();
        return;
      }
      setMsLeft(remaining);
      const secsLeft = Math.ceil(remaining / 1000);
      // טיק ל-10 השניות האחרונות, פעם אחת לכל שנייה
      if (secsLeft <= 10 && secsLeft >= 1 && secsLeft !== prevTickRef.current) {
        prevTickRef.current = secsLeft;
        audio.tick(secsLeft);
      }
    }, 100);
  };

  const logRound = () => {
    if (phase !== "running") return;
    roundsRef.current += 1;
    setRounds(roundsRef.current);
    audio.blip();
  };

  const abandon = () => {
    stopInterval();
    onExit();
  };

  const secsLeft = Math.ceil(msLeft / 1000);
  const urgent = phase === "running" && secsLeft <= 10;

  // ---- מסך מוכן ----
  if (phase === "ready") {
    const pb = getAmrapPB();
    return (
      <main className="max-w-xl mx-auto px-4 py-5 flex flex-col gap-5 min-h-[calc(100dvh-56px)]">
        <TopBar title="20 דקות AMRAP" subtitle="Cindy" onExit={onExit} />

        <div className="rounded-2xl border border-border bg-panel p-5 flex flex-col gap-3">
          <p className="text-text-muted text-sm">כל סיבוב:</p>
          <ul className="flex flex-col gap-2">
            {AMRAP_ROUND.map((e) => {
              const n = exerciseName(e.key, equipment);
              return (
                <li key={e.key} className="flex items-baseline justify-between">
                  <span className="font-display font-bold text-lg">{n.he}</span>
                  <span className="flex items-baseline gap-2">
                    <span className="text-text-muted text-xs">{n.en}</span>
                    <span className="num font-display font-bold text-2xl text-neon-cyan">
                      ×{e.reps}
                    </span>
                  </span>
                </li>
              );
            })}
          </ul>
        </div>

        <p className="text-center text-sm">
          🏆 שיא נוכחי:{" "}
          <span className="num font-display font-bold text-neon-lime">
            {pb != null ? `${pb} סיבובים` : "—"}
          </span>
        </p>

        <button
          onClick={start}
          className="mt-auto w-full min-h-[40dvh] rounded-3xl bg-neon-cyan text-bg font-display font-bold text-5xl shadow-[0_0_40px_-8px] shadow-neon-cyan active:scale-[0.98] transition-transform"
        >
          התחל ▶
        </button>
      </main>
    );
  }

  // ---- מסך סיום ----
  if (phase === "done") {
    return (
      <main className="max-w-xl mx-auto px-4 py-5 flex flex-col gap-5 min-h-[calc(100dvh-56px)] text-center">
        <TopBar title="20 דקות AMRAP" subtitle="Cindy" onExit={onExit} />

        <div className="flex-1 flex flex-col items-center justify-center gap-4">
          {isPB && (
            <div className="font-display font-bold text-2xl text-neon-lime animate-pulse">
              🎉 שיא חדש!
            </div>
          )}
          <div className="text-text-muted">סיבובים שהושלמו</div>
          <div className="num font-display font-bold text-8xl text-neon-cyan">
            {rounds}
          </div>
          <div className="text-text-muted text-sm">
            = <span className="num">{rounds * AMRAP_REPS_PER_ROUND}</span> חזרות
            סה״כ
          </div>
        </div>

        <div className="grid grid-cols-2 gap-3">
          <button
            onClick={() => setPhase("ready")}
            className="rounded-2xl border-2 border-neon-cyan text-neon-cyan font-display font-bold text-xl py-5 active:scale-[0.98] transition-transform"
          >
            שוב ↺
          </button>
          <button
            onClick={onExit}
            className="rounded-2xl border border-border text-text-main font-display font-bold text-xl py-5 active:scale-[0.98] transition-transform"
          >
            תפריט
          </button>
        </div>
      </main>
    );
  }

  // ---- מסך ריצה ----
  return (
    <main className="max-w-xl mx-auto px-4 py-4 flex flex-col gap-4 min-h-[calc(100dvh-56px)]">
      <div className="flex items-center justify-between">
        <button
          onClick={abandon}
          className="h-10 px-3 rounded-lg border border-border text-text-muted text-sm hover:text-text-main"
        >
          ← יציאה
        </button>
        <button
          onClick={finishWorkout}
          className="h-10 px-3 rounded-lg border border-border text-text-muted text-sm hover:text-text-main"
        >
          סיום מוקדם
        </button>
      </div>

      <div className="text-center">
        <div className="text-text-muted text-xs mb-1">זמן נותר</div>
        <div
          className={`num font-display font-bold leading-none ${
            urgent ? "text-neon-pink animate-pulse" : "text-text-main"
          }`}
          style={{ fontSize: "clamp(3.5rem, 22vw, 7rem)" }}
        >
          {formatClock(msLeft / 1000)}
        </div>
      </div>

      <div className="flex items-center justify-center gap-6 text-center">
        <div>
          <div className="num font-display font-bold text-4xl text-neon-cyan">
            {rounds}
          </div>
          <div className="text-text-muted text-xs">סיבובים</div>
        </div>
        <div className="text-text-muted text-sm leading-tight text-right">
          {AMRAP_ROUND.map((e) => (
            <div key={e.key}>
              <span className="num">{e.reps}</span>{" "}
              {exerciseName(e.key, equipment).he}
            </div>
          ))}
        </div>
      </div>

      <button
        onClick={logRound}
        className="mt-auto w-full min-h-[40dvh] rounded-3xl bg-neon-lime text-bg font-display font-bold shadow-[0_0_50px_-10px] shadow-neon-lime active:scale-[0.97] transition-transform flex flex-col items-center justify-center gap-1"
      >
        <span className="text-5xl">סיבוב הושלם ✓</span>
        <span className="num text-2xl opacity-80">סיבוב #{rounds + 1}</span>
      </button>
    </main>
  );
}

function TopBar({
  title,
  subtitle,
  onExit,
}: {
  title: string;
  subtitle: string;
  onExit: () => void;
}) {
  return (
    <header className="flex items-center justify-between gap-3">
      <div>
        <h1 className="font-display font-bold text-xl">{title}</h1>
        <p className="text-text-muted text-sm">{subtitle}</p>
      </div>
      <button
        onClick={onExit}
        className="h-10 px-3 rounded-lg border border-border text-text-muted text-sm hover:text-text-main"
      >
        ← תפריט
      </button>
    </header>
  );
}
