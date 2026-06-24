import { AudioEngine, VOICES } from './audio-engine.js';

const NUM_VOICES = VOICES.length;
const MAX_STEPS = 32;

class Sequencer {
  constructor(engine) {
    this.engine = engine;
    this.steps = 16;
    this.bpm = 120;
    this.swing = 0;
    this.running = false;
    this.currentStep = -1;
    this.nextStepTime = 0;
    this.scheduleAheadTime = 0.1;
    this.lookahead = 25;
    this.timerID = null;

    this.patterns = [];
    for (let p = 0; p < 8; p++) {
      this.patterns.push(this._emptyPattern());
    }
    this.currentPattern = 0;

    this.onStep = null;
  }

  _emptyPattern() {
    const grid = [];
    for (let v = 0; v < NUM_VOICES; v++) {
      grid.push(new Array(MAX_STEPS).fill(false));
    }
    const accent = new Array(MAX_STEPS).fill(false);
    const bassNotes = new Array(MAX_STEPS).fill(-1);
    return { grid, accent, bassNotes, length: 16 };
  }

  setStep(voice, step, on) {
    this.patterns[this.currentPattern].grid[voice][step] = on;
  }

  getStep(voice, step) {
    return this.patterns[this.currentPattern].grid[voice][step];
  }

  toggleStep(voice, step) {
    const pat = this.patterns[this.currentPattern];
    pat.grid[voice][step] = !pat.grid[voice][step];
    return pat.grid[voice][step];
  }

  setAccent(step, on) {
    this.patterns[this.currentPattern].accent[step] = on;
  }

  getAccent(step) {
    return this.patterns[this.currentPattern].accent[step];
  }

  toggleAccent(step) {
    const pat = this.patterns[this.currentPattern];
    pat.accent[step] = !pat.accent[step];
    return pat.accent[step];
  }

  setBassNote(step, midiNote) {
    this.patterns[this.currentPattern].bassNotes[step] = midiNote;
  }

  getBassNote(step) {
    return this.patterns[this.currentPattern].bassNotes[step];
  }

  clearPattern() {
    this.patterns[this.currentPattern] = this._emptyPattern();
    this.patterns[this.currentPattern].length = this.steps;
  }

  start() {
    if (this.running) return;
    this.running = true;
    this.currentStep = -1;
    this.nextStepTime = this.engine.currentTime + 0.05;
    this._schedule();
  }

  stop() {
    this.running = false;
    if (this.timerID !== null) {
      clearTimeout(this.timerID);
      this.timerID = null;
    }
    this.currentStep = -1;
    if (this.onStep) this.onStep(-1);
  }

  toggle() {
    if (this.running) this.stop();
    else this.start();
    return this.running;
  }

  _getStepDuration(step) {
    const baseInterval = 60.0 / this.bpm / 4;
    if (this.swing > 0 && step % 2 === 1) {
      return baseInterval * (1 + this.swing);
    }
    if (this.swing > 0 && step % 2 === 0) {
      return baseInterval * (1 - this.swing);
    }
    return baseInterval;
  }

  _schedule() {
    if (!this.running) return;

    while (this.nextStepTime < this.engine.currentTime + this.scheduleAheadTime) {
      this.currentStep = (this.currentStep + 1) % this.steps;
      const pat = this.patterns[this.currentPattern];
      const accent = pat.accent[this.currentStep];

      for (let v = 0; v < NUM_VOICES; v++) {
        if (pat.grid[v][this.currentStep]) {
          this.engine.trigger(v, this.nextStepTime, accent);
        }
      }

      const bassNote = pat.bassNotes[this.currentStep];
      if (bassNote >= 0) {
        this.engine.triggerBass(bassNote, this.nextStepTime);
      }

      if (this.onStep) {
        const step = this.currentStep;
        const time = this.nextStepTime;
        const now = this.engine.currentTime;
        const delay = Math.max(0, (time - now) * 1000);
        setTimeout(() => this.onStep(step), delay);
      }

      this.nextStepTime += this._getStepDuration(this.currentStep);
    }

    this.timerID = setTimeout(() => this._schedule(), this.lookahead);
  }
}

export { Sequencer, NUM_VOICES, MAX_STEPS };
