// מנוע סאונד 8-ביט מבוסס Web Audio API (ללא תלויות חיצוניות).
// יוצר AudioContext באופן עצל ומפעיל אותו רק אחרי אינטראקציית משתמש
// (דרישת דפדפנים, בעיקר iOS). כל השגיאות נבלעות כדי לא לשבור אימון.

type Wave = "square" | "triangle" | "sawtooth" | "sine";

export class WorkoutAudio {
  private ctx: AudioContext | null = null;
  private muted = false;

  /** יוצר/מחזיר את ה-AudioContext. יש לקרוא בתוך event handler של משתמש. */
  private ensureCtx(): AudioContext | null {
    if (typeof window === "undefined") return null;
    if (!this.ctx) {
      const AC: typeof AudioContext | undefined =
        window.AudioContext ||
        (window as unknown as { webkitAudioContext?: typeof AudioContext })
          .webkitAudioContext;
      if (!AC) return null;
      try {
        this.ctx = new AC();
      } catch {
        return null;
      }
    }
    if (this.ctx.state === "suspended") {
      this.ctx.resume().catch(() => {});
    }
    return this.ctx;
  }

  /** "מחמם" את ההקשר על לחיצת הפעלה ראשונה (כדי שהצליל הבא יישמע מיד). */
  unlock(): void {
    this.ensureCtx();
  }

  setMuted(m: boolean): void {
    this.muted = m;
  }

  isMuted(): boolean {
    return this.muted;
  }

  // צליל בודד עם מעטפת (envelope) בסגנון צ'יפ-טיון.
  private note(
    freq: number,
    startAt: number,
    duration: number,
    wave: Wave = "square",
    peak = 0.22
  ): void {
    const ctx = this.ctx;
    if (!ctx) return;
    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.type = wave;
    osc.frequency.setValueAtTime(freq, startAt);

    gain.gain.setValueAtTime(0.0001, startAt);
    gain.gain.exponentialRampToValueAtTime(peak, startAt + 0.008);
    gain.gain.exponentialRampToValueAtTime(0.0001, startAt + duration);

    osc.connect(gain).connect(ctx.destination);
    osc.start(startAt);
    osc.stop(startAt + duration + 0.02);
  }

  private play(seq: { f: number; d: number; w?: Wave; peak?: number }[]): void {
    if (this.muted) return;
    const ctx = this.ensureCtx();
    if (!ctx) return;
    let t = ctx.currentTime + 0.01;
    for (const n of seq) {
      this.note(n.f, t, n.d, n.w ?? "square", n.peak);
      t += n.d;
    }
  }

  /** לחיצת "סיבוב/רמה הושלמו" — בליפ מתגמל עולה (level-up). */
  blip(): void {
    this.play([
      { f: 660, d: 0.06 }, // E5
      { f: 880, d: 0.06 }, // A5
      { f: 1318, d: 0.1 }, // E6
    ]);
  }

  /** 10 השניות האחרונות — טיק יורד; גובה הצליל יורד ככל שמתקרבים לאפס. */
  tick(secondsLeft: number): void {
    // secondsLeft: 10..1 → תדר יורד מ-~900Hz ל-~500Hz
    const freq = 500 + Math.max(0, Math.min(10, secondsLeft)) * 40;
    this.play([{ f: freq, d: 0.09, w: "square", peak: 0.18 }]);
  }

  /** סיום הטיימר — צ'יים ניצחון ארקייד ארוך. */
  finish(): void {
    this.play([
      { f: 523, d: 0.11 }, // C5
      { f: 659, d: 0.11 }, // E5
      { f: 784, d: 0.11 }, // G5
      { f: 1046, d: 0.16 }, // C6
      { f: 784, d: 0.09 }, // G5
      { f: 1046, d: 0.3, peak: 0.26 }, // C6 (held)
    ]);
  }

  /** צליל "game over" קצר ויורד (למשל סיום בלי שיא). */
  gameOver(): void {
    this.play([
      { f: 440, d: 0.14, w: "triangle" },
      { f: 349, d: 0.14, w: "triangle" },
      { f: 262, d: 0.3, w: "triangle", peak: 0.24 },
    ]);
  }
}
