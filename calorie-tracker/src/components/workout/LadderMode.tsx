"use client";

import { useEffect, useRef, useState } from "react";
import {
  LADDER_SEQUENCE,
  LADDER_TOTAL_REPS,
  ladderReps,
  ladderRepsTotal,
  exerciseName,
  formatInt,
  formatStopwatch,
  saveLadderPB,
  getLadderPB,
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

const LAST = LADDER_SEQUENCE.length - 1;

// סך החזרות שהושלמו עד (לא כולל) שלב נתון.
function repsBefore(stepIndex: number): number {
  let sum = 0;
  for (let i = 0; i < stepIndex; i++) sum += ladderRepsTotal(LADDER_SEQUENCE[i]);
  return sum;
}

export default function LadderMode({ audio, equipment, onExit }: Props) {
  const [phase, setPhase] = useState<Phase>("ready");
  const [stepIndex, setStepIndex] = useState(0);
  const [elapsedMs, setElapsedMs] = useState(0);
  const [finalMs, setFinalMs] = useState(0);
  const [isPB, setIsPB] = useState(false);

  const startTimeRef = useRef(0);
  const stepRef = useRef(0);
  const finishedRef = useRef(false);
  const intervalRef = useRef<ReturnType<typeof setInterval> | null>(null);

  useWakeLock(phase === "running");

  const stopInterval = () => {
    if (intervalRef.current) {
      clearInterval(intervalRef.current);
      intervalRef.current = null;
    }
  };

  useEffect(() => stopInterval, []);

  const start = () => {
    audio.unlock();
    stepRef.current = 0;
    finishedRef.current = false;
    setStepIndex(0);
    setIsPB(false);
    startTimeRef.current = performance.now();
    setElapsedMs(0);
    setPhase("running");

    stopInterval();
    intervalRef.current = setInterval(() => {
      setElapsedMs(performance.now() - startTimeRef.current);
    }, 100);
  };

  const finishWorkout = () => {
    if (finishedRef.current) return;
    finishedRef.current = true;
    stopInterval();
    const ms = performance.now() - startTimeRef.current;
    setFinalMs(ms);
    setElapsedMs(ms);
    setPhase("done");
    setIsPB(saveLadderPB(ms));
    audio.finish();
  };

  const advance = () => {
    if (phase !== "running") return;
    if (stepRef.current >= LAST) {
      finishWorkout();
      return;
    }
    stepRef.current += 1;
    setStepIndex(stepRef.current);
    audio.blip();
  };

  const abandon = () => {
    stopInterval();
    onExit();
  };

  const level = LADDER_SEQUENCE[stepIndex];
  const reps = ladderReps(level);
  const isLast = stepIndex >= LAST;
  const nextLevel = isLast ? null : LADDER_SEQUENCE[stepIndex + 1];
  const goingUp = nextLevel != null && nextLevel > level;
  const repsDone = repsBefore(stepIndex);

  // ---- מסך מוכן ----
  if (phase === "ready") {
    const pb = getLadderPB();
    return (
      <main className="max-w-xl mx-auto px-4 py-5 flex flex-col gap-5 min-h-[calc(100dvh-56px)]">
        <TopBar
          title={`סולם ${formatInt(LADDER_TOTAL_REPS)}`}
          subtitle="Monster Ladder"
          onExit={onExit}
        />

        <div className="rounded-2xl border border-border bg-panel p-5 flex flex-col gap-3">
          <p className="text-text-muted text-sm">
            רמות 1→10→1 ({LADDER_SEQUENCE.length} שלבים). בכל רמה:
          </p>
          <ul className="flex flex-col gap-1.5 text-sm">
            {reps.map((e) => {
              const n = exerciseName(e.key, equipment);
              const mult = e.reps / level; // = המכפיל
              return (
                <li key={e.key} className="flex items-center justify-between">
                  <span className="font-display font-bold">{n.he}</span>
                  <span className="num text-text-muted">
                    רמה × {mult}
                  </span>
                </li>
              );
            })}
          </ul>
          <p className="text-text-muted text-xs">
            דוגמה — רמה 2: 2 מתח, 4 מקבילים, 6 שכיבות, 8 בטן, 10 סקוואט.
          </p>
        </div>

        <p className="text-center text-sm">
          🏆 הזמן המהיר ביותר:{" "}
          <span className="num font-display font-bold text-neon-lime">
            {pb != null ? formatStopwatch(pb) : "—"}
          </span>
        </p>

        <button
          onClick={start}
          className="mt-auto w-full min-h-[40dvh] rounded-3xl bg-neon-pink text-bg font-display font-bold text-5xl shadow-[0_0_40px_-8px] shadow-neon-pink active:scale-[0.98] transition-transform"
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
        <TopBar
          title={`סולם ${formatInt(LADDER_TOTAL_REPS)}`}
          subtitle="Monster Ladder"
          onExit={onExit}
        />
        <div className="flex-1 flex flex-col items-center justify-center gap-4">
          {isPB && (
            <div className="font-display font-bold text-2xl text-neon-lime animate-pulse">
              🎉 שיא חדש!
            </div>
          )}
          <div className="text-6xl">🏁</div>
          <div className="text-text-muted">זמן סופי</div>
          <div className="num font-display font-bold text-6xl text-neon-pink">
            {formatStopwatch(finalMs)}
          </div>
          <div className="text-text-muted text-sm">
            <span className="num">{formatInt(LADDER_TOTAL_REPS)}</span>{" "}
            חזרות הושלמו 💪
          </div>
        </div>

        <div className="grid grid-cols-2 gap-3">
          <button
            onClick={() => setPhase("ready")}
            className="rounded-2xl border-2 border-neon-pink text-neon-pink font-display font-bold text-xl py-5 active:scale-[0.98] transition-transform"
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
    <main className="max-w-xl mx-auto px-4 py-4 flex flex-col gap-3 min-h-[calc(100dvh-56px)]">
      <div className="flex items-center justify-between">
        <button
          onClick={abandon}
          className="h-10 px-3 rounded-lg border border-border text-text-muted text-sm hover:text-text-main"
        >
          ← יציאה
        </button>
        <div className="num font-display font-bold text-2xl text-neon-cyan">
          {formatStopwatch(elapsedMs)}
        </div>
      </div>

      {/* התקדמות בשלבים */}
      <div className="flex items-center justify-center gap-1 flex-wrap">
        {LADDER_SEQUENCE.map((lvl, i) => (
          <span
            key={i}
            title={`רמה ${lvl}`}
            className={`h-2 rounded-full transition-colors ${
              i < stepIndex
                ? "bg-neon-lime w-2"
                : i === stepIndex
                  ? "bg-neon-pink w-5"
                  : "bg-border-2 w-2"
            }`}
          />
        ))}
      </div>

      <div className="text-center">
        <div className="text-text-muted text-xs">
          שלב <span className="num">{stepIndex + 1}</span>/
          <span className="num">{LADDER_SEQUENCE.length}</span> ·{" "}
          {goingUp || isLast ? "" : "ירידה "}
          {repsDone > 0 && (
            <>
              <span className="num">{formatInt(repsDone)}</span>/
              <span className="num">{formatInt(LADDER_TOTAL_REPS)}</span>{" "}
              חזרות
            </>
          )}
        </div>
        <div className="text-text-muted text-xs mt-1">רמה</div>
        <div className="num font-display font-bold text-7xl text-neon-pink leading-none">
          {level}
        </div>
      </div>

      <div className="rounded-2xl border border-border bg-panel p-3">
        <ul className="grid grid-cols-1 gap-1.5">
          {reps.map((e) => {
            const n = exerciseName(e.key, equipment);
            return (
              <li
                key={e.key}
                className="flex items-center justify-between text-sm"
              >
                <span className="font-display font-bold">{n.he}</span>
                <span className="num font-display font-bold text-xl text-neon-cyan">
                  ×{e.reps}
                </span>
              </li>
            );
          })}
        </ul>
      </div>

      <button
        onClick={advance}
        className={`mt-auto w-full min-h-[38dvh] rounded-3xl font-display font-bold text-bg active:scale-[0.97] transition-transform flex flex-col items-center justify-center gap-1 ${
          isLast
            ? "bg-neon-lime shadow-[0_0_50px_-10px] shadow-neon-lime"
            : "bg-neon-pink shadow-[0_0_50px_-10px] shadow-neon-pink"
        }`}
      >
        {isLast ? (
          <span className="text-5xl">סיום 🏁</span>
        ) : (
          <>
            <span className="text-5xl">
              רמה הבאה {goingUp ? "↑" : "↓"}
            </span>
            <span className="num text-2xl opacity-80">רמה {nextLevel}</span>
          </>
        )}
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
