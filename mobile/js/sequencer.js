import { AudioEngine, VOICES } from './audio-engine.js';

const NUM_VOICES = VOICES.length;
const MAX_STEPS = 32;
const NUM_PATTERNS = 8;
const NUM_VARS = 4;

class Sequencer {
  constructor(engine) {
    this.engine = engine;
    this.bpm = 120;
    this.swing = 0;
    this.running = false;
    this.currentStep = -1;
    this.nextStepTime = 0;
    this.scheduleAheadTime = 0.1;
    this.lookahead = 25;
    this.timerID = null;

    this.patterns = [];
    for (let p = 0; p < NUM_PATTERNS; p++) {
      this.patterns.push(this._emptyPattern());
    }
    this.currentPattern = 0;
    this.currentVar = 0;
    this.playMode = 'a';

    this.muted = new Array(NUM_VOICES).fill(false);
    this.soloed = new Array(NUM_VOICES).fill(false);

    this.chainEnabled = false;
    this.chain = [];
    this.chainIndex = 0;
    this.varCycleIndex = 0;

    this.onStep = null;
  }

  _emptyVariation() {
    const grid = [];
    for (let v = 0; v < NUM_VOICES; v++) {
      grid.push(new Array(MAX_STEPS).fill(false));
    }
    return {
      grid,
      accent: new Array(MAX_STEPS).fill(false),
      bassNotes: new Array(MAX_STEPS).fill(-1),
      length: 16,
      stepDiv: 0.25,
    };
  }

  _emptyPattern() {
    const vars = [];
    for (let v = 0; v < NUM_VARS; v++) vars.push(this._emptyVariation());
    return { vars };
  }

  get _var() { return this.patterns[this.currentPattern].vars[this.currentVar]; }
  get steps() { return this._var.length; }
  set steps(v) { this._var.length = Math.max(1, Math.min(MAX_STEPS, v)); }
  get triplet() { return this._var.stepDiv < 0.22; }
  set triplet(t) { this._var.stepDiv = t ? (1 / 6) : 0.25; }

  setStep(voice, step, on) { this._var.grid[voice][step] = on; }
  getStep(voice, step) { return this._var.grid[voice][step]; }
  toggleStep(voice, step) {
    this._var.grid[voice][step] = !this._var.grid[voice][step];
    return this._var.grid[voice][step];
  }

  setAccent(step, on) { this._var.accent[step] = on; }
  getAccent(step) { return this._var.accent[step]; }
  toggleAccent(step) {
    this._var.accent[step] = !this._var.accent[step];
    return this._var.accent[step];
  }

  setBassNote(step, midiNote) { this._var.bassNotes[step] = midiNote; }
  getBassNote(step) { return this._var.bassNotes[step]; }

  setMute(voice, m) { this.muted[voice] = m; }
  getMute(voice) { return this.muted[voice]; }
  toggleMute(voice) { this.muted[voice] = !this.muted[voice]; return this.muted[voice]; }

  setSolo(voice, s) { this.soloed[voice] = s; }
  getSolo(voice) { return this.soloed[voice]; }
  toggleSolo(voice) { this.soloed[voice] = !this.soloed[voice]; return this.soloed[voice]; }

  isVoicePlayable(voice) {
    const anySolo = this.soloed.some(s => s);
    if (this.muted[voice]) return false;
    if (anySolo && !this.soloed[voice]) return false;
    return true;
  }

  clearPattern() {
    const len = this._var.length;
    const div = this._var.stepDiv;
    this.patterns[this.currentPattern].vars[this.currentVar] = this._emptyVariation();
    this._var.length = len;
    this._var.stepDiv = div;
  }

  copyVariation(from, to) {
    const pat = this.patterns[this.currentPattern];
    const src = pat.vars[from];
    const dst = this._emptyVariation();
    for (let v = 0; v < NUM_VOICES; v++) {
      dst.grid[v] = [...src.grid[v]];
    }
    dst.accent = [...src.accent];
    dst.bassNotes = [...src.bassNotes];
    dst.length = src.length;
    dst.stepDiv = src.stepDiv;
    pat.vars[to] = dst;
  }

  copyPattern(from, to) {
    const dst = this._emptyPattern();
    const src = this.patterns[from];
    for (let vi = 0; vi < NUM_VARS; vi++) {
      const sv = src.vars[vi];
      const dv = dst.vars[vi];
      for (let v = 0; v < NUM_VOICES; v++) dv.grid[v] = [...sv.grid[v]];
      dv.accent = [...sv.accent];
      dv.bassNotes = [...sv.bassNotes];
      dv.length = sv.length;
      dv.stepDiv = sv.stepDiv;
    }
    this.patterns[to] = dst;
  }

  _getPlayVar() {
    switch (this.playMode) {
      case 'a': return 0;
      case 'b': return 1;
      case 'c': return 2;
      case 'd': return 3;
      case 'cycle': return this.varCycleIndex;
      default: return 0;
    }
  }

  start() {
    if (this.running) return;
    this.running = true;
    this.currentStep = -1;
    this.chainIndex = 0;
    this.varCycleIndex = 0;
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
    const tripMul = this.triplet ? (2 / 3) : 1;
    const adj = baseInterval * tripMul;
    if (this.swing > 0 && step % 2 === 1) return adj * (1 + this.swing);
    if (this.swing > 0 && step % 2 === 0) return adj * (1 - this.swing);
    return adj;
  }

  _schedule() {
    if (!this.running) return;

    while (this.nextStepTime < this.engine.currentTime + this.scheduleAheadTime) {
      const playPat = this.chainEnabled && this.chain.length > 0
        ? this.chain[this.chainIndex % this.chain.length]
        : this.currentPattern;
      const playVar = this._getPlayVar();
      const variation = this.patterns[playPat].vars[playVar];
      const len = variation.length;

      this.currentStep = (this.currentStep + 1) % len;
      const accent = variation.accent[this.currentStep];

      for (let v = 0; v < NUM_VOICES; v++) {
        if (variation.grid[v][this.currentStep] && this.isVoicePlayable(v)) {
          this.engine.trigger(v, this.nextStepTime, accent);
        }
      }

      const bassNote = variation.bassNotes[this.currentStep];
      if (bassNote >= 0) {
        this.engine.triggerBass(bassNote, this.nextStepTime);
      }

      if (this.onStep) {
        const step = this.currentStep;
        const now = this.engine.currentTime;
        const delay = Math.max(0, (this.nextStepTime - now) * 1000);
        setTimeout(() => this.onStep(step), delay);
      }

      this.nextStepTime += this._getStepDuration(this.currentStep);

      if (this.currentStep === len - 1) {
        if (this.chainEnabled && this.chain.length > 0) {
          this.chainIndex = (this.chainIndex + 1) % this.chain.length;
        }
        if (this.playMode === 'cycle') {
          this.varCycleIndex = (this.varCycleIndex + 1) % NUM_VARS;
        }
      }
    }

    this.timerID = setTimeout(() => this._schedule(), this.lookahead);
  }
}

export { Sequencer, NUM_VOICES, MAX_STEPS, NUM_PATTERNS, NUM_VARS };
