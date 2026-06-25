const VOICES = [
  { id: 'bd', name: 'BD',  fullName: 'Bass Drum',  gmNote: 36, tone: true,  decay: true,  snappy: false, tune: true  },
  { id: 'rs', name: 'RS',  fullName: 'Rim Shot',   gmNote: 37, tone: false, decay: false, snappy: false, tune: false },
  { id: 'sd', name: 'SD',  fullName: 'Snare',      gmNote: 38, tone: true,  decay: false, snappy: true,  tune: false },
  { id: 'cp', name: 'CP',  fullName: 'Hand Clap',  gmNote: 39, tone: false, decay: false, snappy: false, tune: false },
  { id: 'lt', name: 'LT',  fullName: 'Low Tom',    gmNote: 41, tone: false, decay: false, snappy: false, tune: true  },
  { id: 'mt', name: 'MT',  fullName: 'Mid Tom',    gmNote: 47, tone: false, decay: false, snappy: false, tune: true  },
  { id: 'ht', name: 'HT',  fullName: 'Hi Tom',     gmNote: 50, tone: false, decay: false, snappy: false, tune: true  },
  { id: 'ma', name: 'MA',  fullName: 'Maracas',    gmNote: 70, tone: false, decay: false, snappy: false, tune: false },
  { id: 'cb', name: 'CB',  fullName: 'Cowbell',    gmNote: 56, tone: false, decay: false, snappy: false, tune: false },
  { id: 'cy', name: 'CY',  fullName: 'Cymbal',     gmNote: 49, tone: true,  decay: true,  snappy: false, tune: false },
  { id: 'oh', name: 'OH',  fullName: 'Open Hat',   gmNote: 46, tone: false, decay: true,  snappy: false, tune: false },
  { id: 'ch', name: 'CH',  fullName: 'Closed Hat', gmNote: 42, tone: false, decay: false, snappy: false, tune: false },
  { id: 'lc', name: 'LC',  fullName: 'Low Conga',  gmNote: 64, tone: false, decay: false, snappy: false, tune: true  },
  { id: 'mc', name: 'MC',  fullName: 'Mid Conga',  gmNote: 63, tone: false, decay: false, snappy: false, tune: true  },
  { id: 'hc', name: 'HC',  fullName: 'Hi Conga',   gmNote: 62, tone: false, decay: false, snappy: false, tune: true  },
  { id: 'cl', name: 'CL',  fullName: 'Claves',     gmNote: 75, tone: false, decay: false, snappy: false, tune: false },
];

const DEEP_PARAMS = {
  bd: [
    { id: 'freq',      label: 'Pitch',     min: 40,  max: 80,   def: 55,   skew: false },
    { id: 'bodydecay', label: 'Body Decay', min: 50,  max: 1500, def: 650,  skew: true  },
    { id: 'punch',     label: 'Punch',      min: 1,   max: 3,    def: 2,    skew: false },
    { id: 'retrig',    label: 'Retrig',     min: 0,   max: 1,    def: 0.5,  skew: false },
    { id: 'sustain',   label: 'Sustain',    min: 0,   max: 1,    def: 0,    skew: false },
    { id: 'drive',     label: 'Drive',      min: 1,   max: 10,   def: 1,    skew: true  },
  ],
  sd: [
    { id: 'o1freq',     label: 'Osc1 Freq',    min: 100, max: 300,  def: 180,  skew: false },
    { id: 'o2freq',     label: 'Osc2 Freq',    min: 200, max: 500,  def: 330,  skew: false },
    { id: 'oscmix',     label: 'Osc Mix',      min: 0,   max: 1,    def: 0.5,  skew: false },
    { id: 'shelldecay', label: 'Shell Decay',   min: 20,  max: 400,  def: 130,  skew: true  },
    { id: 'nbpfreq',    label: 'Noise BP',     min: 800, max: 4000, def: 1800, skew: true  },
    { id: 'nbpq',       label: 'Noise Q',      min: 0.3, max: 3,    def: 1.1,  skew: false },
    { id: 'ndecay',     label: 'Noise Decay',  min: 20,  max: 600,  def: 200,  skew: true  },
    { id: 'balance',    label: 'Shell/Noise',  min: 0,   max: 1,    def: 0.5,  skew: false },
  ],
  cp: [
    { id: 'bpfreq',    label: 'BP Centre',     min: 500,  max: 2000, def: 1000, skew: true  },
    { id: 'bpq',       label: 'BP Q',          min: 0.5,  max: 4,    def: 1.3,  skew: false },
    { id: 'npulses',   label: 'Pulses',        min: 1,    max: 6,    def: 3,    skew: false },
    { id: 'spacing',   label: 'Pulse Spacing', min: 3,    max: 30,   def: 10,   skew: false },
    { id: 'taildecay', label: 'Tail Decay',    min: 20,   max: 400,  def: 120,  skew: true  },
  ],
  rs: [
    { id: 'freq',      label: 'Tune',    min: 1000, max: 3000, def: 1700, skew: true  },
    { id: 'decaytime', label: 'Decay',   min: 10,   max: 200,  def: 60,   skew: true  },
  ],
  cl: [
    { id: 'freq',      label: 'Tune',    min: 1500, max: 4000, def: 2500, skew: true  },
    { id: 'decaytime', label: 'Decay',   min: 10,   max: 200,  def: 50,   skew: true  },
  ],
  cb: [
    { id: 'o1freq',    label: 'Osc1 Freq',  min: 400,  max: 700,  def: 540,  skew: false },
    { id: 'o2freq',    label: 'Osc2 Freq',  min: 600,  max: 1000, def: 800,  skew: false },
    { id: 'bpfreq',    label: 'BP Centre',  min: 1000, max: 4000, def: 2640, skew: true  },
    { id: 'bpq',       label: 'BP Q',       min: 0.3,  max: 3,    def: 0.8,  skew: false },
    { id: 'decaytime', label: 'Decay',      min: 100,  max: 1000, def: 400,  skew: true  },
  ],
  cy: [
    { id: 'hpf',       label: 'HPF',         min: 2000, max: 9000, def: 5000, skew: true  },
    { id: 'bpfreq',    label: 'Clang Band',  min: 1500, max: 6000, def: 3200, skew: true  },
    { id: 'decaytime', label: 'Decay',       min: 400,  max: 4000, def: 1500, skew: true  },
    { id: 'balance',   label: 'Band Balance', min: 0,   max: 1,    def: 0.5,  skew: false },
  ],
  oh: [
    { id: 'hpf',       label: 'HPF',    min: 2000, max: 12000, def: 3000,  skew: true  },
    { id: 'lpf',       label: 'Color',  min: 5000, max: 16000, def: 12000, skew: true  },
    { id: 'decaytime', label: 'Decay',  min: 150,  max: 1500,  def: 600,   skew: true  },
  ],
  ch: [
    { id: 'hpf',       label: 'HPF',    min: 2000, max: 12000, def: 3000,  skew: true  },
    { id: 'lpf',       label: 'Color',  min: 5000, max: 16000, def: 11000, skew: true  },
    { id: 'decaytime', label: 'Decay',  min: 20,   max: 300,   def: 180,   skew: true  },
  ],
  ma: [
    { id: 'hpf',       label: 'HPF',    min: 3000, max: 12000, def: 6000, skew: true  },
    { id: 'decaytime', label: 'Decay',  min: 10,   max: 100,   def: 30,   skew: true  },
  ],
};

const TOM_DEEP = {
  lt: [
    { id: 'freq',      label: 'Tune',           min: 40,  max: 200,  def: 90,  skew: true  },
    { id: 'penvamt',   label: 'Pitch Env Amt',  min: 0,   max: 1,    def: 0.6, skew: false },
    { id: 'penvtime',  label: 'Pitch Env Time', min: 5,   max: 150,  def: 40,  skew: true  },
    { id: 'decaytime', label: 'Decay',           min: 50,  max: 2000, def: 600, skew: true  },
    { id: 'atknoise',  label: 'Attack Noise',   min: 0,   max: 1,    def: 0.3, skew: false },
  ],
  mt: [
    { id: 'freq',      label: 'Tune',           min: 50,  max: 260,  def: 130, skew: true  },
    { id: 'penvamt',   label: 'Pitch Env Amt',  min: 0,   max: 1,    def: 0.6, skew: false },
    { id: 'penvtime',  label: 'Pitch Env Time', min: 5,   max: 150,  def: 40,  skew: true  },
    { id: 'decaytime', label: 'Decay',           min: 50,  max: 2000, def: 500, skew: true  },
    { id: 'atknoise',  label: 'Attack Noise',   min: 0,   max: 1,    def: 0.3, skew: false },
  ],
  ht: [
    { id: 'freq',      label: 'Tune',           min: 60,  max: 320,  def: 180, skew: true  },
    { id: 'penvamt',   label: 'Pitch Env Amt',  min: 0,   max: 1,    def: 0.6, skew: false },
    { id: 'penvtime',  label: 'Pitch Env Time', min: 5,   max: 150,  def: 40,  skew: true  },
    { id: 'decaytime', label: 'Decay',           min: 50,  max: 1500, def: 450, skew: true  },
    { id: 'atknoise',  label: 'Attack Noise',   min: 0,   max: 1,    def: 0.3, skew: false },
  ],
  lc: [
    { id: 'freq',      label: 'Tune',           min: 120, max: 400,  def: 220, skew: true  },
    { id: 'penvamt',   label: 'Pitch Env Amt',  min: 0,   max: 1,    def: 0.6, skew: false },
    { id: 'penvtime',  label: 'Pitch Env Time', min: 5,   max: 150,  def: 40,  skew: true  },
    { id: 'decaytime', label: 'Decay',           min: 30,  max: 800,  def: 250, skew: true  },
    { id: 'atknoise',  label: 'Attack Noise',   min: 0,   max: 1,    def: 0.3, skew: false },
  ],
  mc: [
    { id: 'freq',      label: 'Tune',           min: 150, max: 460,  def: 280, skew: true  },
    { id: 'penvamt',   label: 'Pitch Env Amt',  min: 0,   max: 1,    def: 0.6, skew: false },
    { id: 'penvtime',  label: 'Pitch Env Time', min: 5,   max: 150,  def: 40,  skew: true  },
    { id: 'decaytime', label: 'Decay',           min: 30,  max: 700,  def: 220, skew: true  },
    { id: 'atknoise',  label: 'Attack Noise',   min: 0,   max: 1,    def: 0.3, skew: false },
  ],
  hc: [
    { id: 'freq',      label: 'Tune',           min: 200, max: 560,  def: 370, skew: true  },
    { id: 'penvamt',   label: 'Pitch Env Amt',  min: 0,   max: 1,    def: 0.6, skew: false },
    { id: 'penvtime',  label: 'Pitch Env Time', min: 5,   max: 150,  def: 40,  skew: true  },
    { id: 'decaytime', label: 'Decay',           min: 30,  max: 600,  def: 200, skew: true  },
    { id: 'atknoise',  label: 'Attack Noise',   min: 0,   max: 1,    def: 0.3, skew: false },
  ],
};

for (const [k, v] of Object.entries(TOM_DEEP)) DEEP_PARAMS[k] = v;

function centeredScale(macro, factor) {
  const t = Math.max(0, Math.min(1, macro));
  const lo = 1.0 / factor, hi = factor;
  return lo * Math.pow(hi / lo, t);
}

function centeredPitch(macro, semis) {
  return Math.pow(2, (Math.max(0, Math.min(1, macro)) - 0.5) * 2 * semis / 12);
}

class AudioEngine {
  constructor() {
    this.ctx = null;
    this.masterGain = null;
    this.compressor = null;
    this.reverbSend = null;
    this.reverbReturn = null;
    this.delaySend = null;
    this.delayReturn = null;
    this.convolver = null;

    this.params = {};
    for (const v of VOICES) {
      this.params[v.id] = { level: 0.8, tone: 0.5, decay: 0.5, snappy: 0.5, tune: 0.5 };
    }

    this.deep = {};
    for (const [voiceId, params] of Object.entries(DEEP_PARAMS)) {
      this.deep[voiceId] = {};
      for (const p of params) this.deep[voiceId][p.id] = p.def;
    }

    this.masterDrive = 1.0;
    this.masterLevel = 0.85;
    this.accentLevel = 1.3;
    this.reverbMix = 0;
    this.delayMix = 0;

    this.bassParams = { level: 0.7, tone: 0.5, decay: 0.5, punch: 0.5, drive: 1.0 };

    this.voicePan = {};
    this.voiceRevSend = {};
    this.voiceDlySend = {};
    for (const v of VOICES) {
      this.voicePan[v.id] = 0;
      this.voiceRevSend[v.id] = 0;
      this.voiceDlySend[v.id] = 0;
    }
  }

  setPan(id, val) { this.voicePan[id] = Math.max(-1, Math.min(1, val)); }
  getPan(id) { return this.voicePan[id] || 0; }
  setRevSend(id, val) { this.voiceRevSend[id] = Math.max(0, Math.min(1, val)); }
  getRevSend(id) { return this.voiceRevSend[id] || 0; }
  setDlySend(id, val) { this.voiceDlySend[id] = Math.max(0, Math.min(1, val)); }
  getDlySend(id) { return this.voiceDlySend[id] || 0; }

  async init() {
    if (this.ctx && this.ctx.state === 'running') return;

    if (!this.ctx) {
      const AC = window.AudioContext || window.webkitAudioContext;
      this.ctx = new AC();

      this.compressor = this.ctx.createDynamicsCompressor();
      this.compressor.threshold.value = -10;
      this.compressor.knee.value = 6;
      this.compressor.ratio.value = 4;
      this.compressor.attack.value = 0.003;
      this.compressor.release.value = 0.12;
      this.compressor.connect(this.ctx.destination);

      this.masterGain = this.ctx.createGain();
      this.masterGain.gain.value = this.masterLevel;
      this.masterGain.connect(this.compressor);

      this._setupDelay();
      this._setupReverb();
    }

    if (this.ctx.state === 'suspended') {
      await this.ctx.resume();
    }

    // iOS requires playing a silent buffer from a user gesture to unlock audio
    const silent = this.ctx.createBuffer(1, 1, this.ctx.sampleRate);
    const src = this.ctx.createBufferSource();
    src.buffer = silent;
    src.connect(this.ctx.destination);
    src.start();
  }

  _setupDelay() {
    const ctx = this.ctx;
    this.delayNode = ctx.createDelay(2.0);
    this.delayNode.delayTime.value = 0.375;
    this.delayFeedback = ctx.createGain();
    this.delayFeedback.gain.value = 0.35;
    this.delayFilter = ctx.createBiquadFilter();
    this.delayFilter.type = 'lowpass';
    this.delayFilter.frequency.value = 4000;
    this.delaySend = ctx.createGain();
    this.delaySend.gain.value = 1.0;
    this.delayReturn = ctx.createGain();
    this.delayReturn.gain.value = 0.5;

    this.delaySend.connect(this.delayNode);
    this.delayNode.connect(this.delayFilter);
    this.delayFilter.connect(this.delayFeedback);
    this.delayFeedback.connect(this.delayNode);
    this.delayFilter.connect(this.delayReturn);
    this.delayReturn.connect(this.masterGain);
  }

  _setupReverb() {
    const ctx = this.ctx;
    const sr = ctx.sampleRate;
    const len = sr * 2.5;
    const buf = ctx.createBuffer(2, len, sr);
    for (let ch = 0; ch < 2; ch++) {
      const d = buf.getChannelData(ch);
      for (let i = 0; i < len; i++) {
        d[i] = (Math.random() * 2 - 1) * Math.pow(1 - i / len, 2.5);
      }
    }
    this.convolver = ctx.createConvolver();
    this.convolver.buffer = buf;

    this.reverbSend = ctx.createGain();
    this.reverbSend.gain.value = 1.0;
    this.reverbReturn = ctx.createGain();
    this.reverbReturn.gain.value = 0.35;
    this.reverbHpf = ctx.createBiquadFilter();
    this.reverbHpf.type = 'highpass';
    this.reverbHpf.frequency.value = 200;

    this.reverbSend.connect(this.reverbHpf);
    this.reverbHpf.connect(this.convolver);
    this.convolver.connect(this.reverbReturn);
    this.reverbReturn.connect(this.masterGain);
  }

  get currentTime() {
    return this.ctx ? this.ctx.currentTime : 0;
  }

  trigger(voiceIndex, time, accent) {
    if (!this.ctx) return;
    const v = VOICES[voiceIndex];
    const p = this.params[v.id];
    const d = this.deep[v.id] || {};
    const t = time || this.ctx.currentTime;
    const vel = accent ? Math.min(1.0, 0.75 * this.accentLevel) : 0.75;

    const id = v.id;
    switch (id) {
      case 'bd': this._bassDrum(t, p, d, vel, id); break;
      case 'rs': this._rimShot(t, p, d, vel, id); break;
      case 'sd': this._snare(t, p, d, vel, id); break;
      case 'cp': this._clap(t, p, d, vel, id); break;
      case 'lt': case 'mt': case 'ht':
      case 'lc': case 'mc': case 'hc':
        this._tomConga(t, p, d, vel, id); break;
      case 'ma': this._maracas(t, p, d, vel, id); break;
      case 'cb': this._cowbell(t, p, d, vel, id); break;
      case 'cy': this._cymbal(t, p, d, vel, id); break;
      case 'oh': this._hihat(t, p, d, vel, true, id); break;
      case 'ch': this._hihat(t, p, d, vel, false, id); break;
      case 'cl': this._claves(t, p, d, vel, id); break;
    }
  }

  triggerBass(midiNote, time) {
    if (!this.ctx) return;
    const freq = 440 * Math.pow(2, (midiNote - 69) / 12);
    const p = this.bassParams;
    const t = time || this.ctx.currentTime;

    const ctx = this.ctx;
    const decayTime = 0.15 + p.decay * 0.8;
    const driveAmt = p.drive;
    const punchMult = 1.5 + p.punch;

    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(freq * punchMult, t);
    osc.frequency.exponentialRampToValueAtTime(freq, t + 0.04);

    let output;
    if (driveAmt > 1.1) {
      const ws = ctx.createWaveShaper();
      ws.curve = this._tanhCurve(driveAmt);
      const preGain = ctx.createGain();
      preGain.gain.value = driveAmt;
      osc.connect(preGain).connect(ws);
      output = ws;
    } else {
      output = osc;
    }

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * 1.0, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    const toneFilter = ctx.createBiquadFilter();
    toneFilter.type = 'lowpass';
    toneFilter.frequency.value = 200 + p.tone * 2000;

    output.connect(toneFilter).connect(gain).connect(this.masterGain);
    osc.start(t); osc.stop(t + decayTime + 0.05);
  }

  _tanhCurve(drive) {
    const n = 256;
    const curve = new Float32Array(n);
    for (let i = 0; i < n; i++) {
      const x = (i / (n - 1)) * 2 - 1;
      curve[i] = Math.tanh(drive * x);
    }
    return curve;
  }

  _connectToFx(node, voiceId) {
    if (voiceId && this.voicePan[voiceId] !== 0) {
      const pan = this.ctx.createStereoPanner();
      pan.pan.value = this.voicePan[voiceId];
      node.connect(pan).connect(this.masterGain);
    } else {
      node.connect(this.masterGain);
    }
    if (this.reverbSend && voiceId) {
      const revAmt = this.voiceRevSend[voiceId] || 0;
      if (revAmt > 0.01) {
        const revGain = this.ctx.createGain();
        revGain.gain.value = revAmt;
        node.connect(revGain).connect(this.reverbSend);
      }
    }
    if (this.delaySend && voiceId) {
      const dlyAmt = this.voiceDlySend[voiceId] || 0;
      if (dlyAmt > 0.01) {
        const dlyGain = this.ctx.createGain();
        dlyGain.gain.value = dlyAmt;
        node.connect(dlyGain).connect(this.delaySend);
      }
    }
  }

  _bassDrum(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const baseFreq = (d.freq || 55) * centeredPitch(p.tune, 12);
    const punchMult = Math.max(1, Math.min(4, d.punch || 2));
    const startFreq = baseFreq * punchMult;
    const bodyDecay = (d.bodydecay || 650) * 0.001 * centeredScale(p.decay, 4) * (1 + (d.sustain || 0) * 8);
    const driveAmt = d.drive || 1;
    const retrigAmt = d.retrig || 0.5;
    const switchTime = 0.5 / startFreq;

    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(startFreq, t);
    osc.frequency.exponentialRampToValueAtTime(Math.max(baseFreq, 20), t + switchTime);
    osc.frequency.setValueAtTime(baseFreq * (1 + retrigAmt * 0.3), t + switchTime);
    osc.frequency.exponentialRampToValueAtTime(Math.max(baseFreq, 20), t + switchTime + 0.02);

    let output;
    if (driveAmt > 1.05) {
      const ws = ctx.createWaveShaper();
      ws.curve = this._tanhCurve(driveAmt);
      osc.connect(ws);
      output = ws;
    } else {
      output = osc;
    }

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 1.1, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + bodyDecay);

    output.connect(gain);
    this._connectToFx(gain, vid);
    osc.start(t); osc.stop(t + bodyDecay + 0.05);

    if (p.tone > 0.01) {
      const click = ctx.createOscillator();
      click.type = 'sine';
      click.frequency.value = 2000;
      const clickGain = ctx.createGain();
      const clickDecayTime = 0.0012;
      clickGain.gain.setValueAtTime(p.tone * p.level * vel * 0.5, t);
      clickGain.gain.exponentialRampToValueAtTime(0.001, t + clickDecayTime * 8);
      click.connect(clickGain).connect(this.masterGain);
      click.start(t); click.stop(t + 0.015);
    }
  }

  _rimShot(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const freq = d.freq || 1700;
    const decayTime = (d.decaytime || 60) * 0.001;

    const osc1 = ctx.createOscillator();
    osc1.type = 'triangle';
    osc1.frequency.value = freq;

    const osc2 = ctx.createOscillator();
    osc2.type = 'triangle';
    osc2.frequency.value = freq * 0.56;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.55, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = freq * 0.7;
    bp.Q.value = 4;

    osc1.connect(bp); osc2.connect(bp);
    bp.connect(gain);
    this._connectToFx(gain, vid);
    osc1.start(t); osc1.stop(t + decayTime + 0.01);
    osc2.start(t); osc2.stop(t + decayTime + 0.01);
  }

  _snare(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const o1freq = d.o1freq || 180;
    const o2freq = d.o2freq || 330;
    const oscMix = Math.max(0, Math.min(1, (d.oscmix || 0.5) + (p.tone - 0.5)));
    const shellDecay = (d.shelldecay || 130) * 0.001;
    const noiseDecay = (d.ndecay || 200) * 0.001 * centeredScale(p.snappy, 2.5);
    const balance = d.balance || 0.5;
    const snappyMod = centeredScale(p.snappy, 2);

    const osc1 = ctx.createOscillator();
    osc1.type = 'triangle';
    osc1.frequency.setValueAtTime(o1freq * 1.7, t);
    osc1.frequency.exponentialRampToValueAtTime(o1freq, t + 0.028);

    const osc2 = ctx.createOscillator();
    osc2.type = 'triangle';
    osc2.frequency.setValueAtTime(o2freq * 1.5, t);
    osc2.frequency.exponentialRampToValueAtTime(o2freq, t + 0.028);

    const osc1Gain = ctx.createGain();
    osc1Gain.gain.value = (1 - oscMix) * 1.5;
    const osc2Gain = ctx.createGain();
    osc2Gain.gain.value = oscMix * 1.5;

    const shellGain = ctx.createGain();
    shellGain.gain.setValueAtTime(p.level * vel * (1 - balance) * 0.7, t);
    shellGain.gain.exponentialRampToValueAtTime(0.001, t + shellDecay);

    osc1.connect(osc1Gain).connect(shellGain);
    osc2.connect(osc2Gain).connect(shellGain);

    const hpf = ctx.createBiquadFilter();
    hpf.type = 'highpass';
    hpf.frequency.value = d.hpf || 300;
    shellGain.connect(hpf);
    this._connectToFx(hpf, vid);

    osc1.start(t); osc1.stop(t + shellDecay + 0.02);
    osc2.start(t); osc2.stop(t + shellDecay + 0.02);

    this._noiseHit(t, p.level * vel * balance * snappyMod * 0.7, noiseDecay,
                   d.nbpfreq || 1800, d.nbpq || 1.1, d.hpf || 300, vid);
  }

  _clap(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const bpFreq = d.bpfreq || 1000;
    const bpQ = d.bpq || 1.3;
    const numPulses = Math.round(d.npulses || 3);
    const spacing = (d.spacing || 10) * 0.001;
    const tailDecay = (d.taildecay || 120) * 0.001;
    const amp = p.level * vel * 0.5;

    for (let i = 0; i < numPulses; i++) {
      this._filteredNoiseHit(t + i * spacing, amp * 0.6, 0.005, bpFreq, bpQ, vid);
    }
    this._filteredNoiseHit(t + numPulses * spacing, amp, tailDecay, bpFreq, bpQ, vid);
  }

  _tomConga(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const baseFreq = (d.freq || 130) * centeredPitch(p.tune, 12);
    const decayTime = (d.decaytime || 500) * 0.001;
    const penvAmt = d.penvamt || 0.6;
    const penvTime = (d.penvtime || 40) * 0.001;
    const atkNoise = d.atknoise || 0.3;
    const driveAmt = d.drive || 1;

    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(baseFreq * (1 + penvAmt), t);
    osc.frequency.exponentialRampToValueAtTime(Math.max(baseFreq, 20), t + penvTime);

    let output;
    if (driveAmt > 1.05) {
      const ws = ctx.createWaveShaper();
      ws.curve = this._tanhCurve(driveAmt);
      osc.connect(ws);
      output = ws;
    } else {
      output = osc;
    }

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.8, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    output.connect(gain);
    this._connectToFx(gain, vid);
    osc.start(t); osc.stop(t + decayTime + 0.05);

    if (atkNoise > 0.01) {
      this._noiseHit(t, p.level * vel * atkNoise * 0.3, 0.004, 2000, 1.0, 0, vid);
    }
  }

  _maracas(t, p, d, vel, vid) {
    const hpf = d.hpf || 6000;
    const decayTime = (d.decaytime || 30) * 0.001;
    this._noiseHit(t, p.level * vel * 0.35, decayTime, hpf, 1.0, hpf, vid);
  }

  _cowbell(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const o1freq = d.o1freq || 540;
    const o2freq = d.o2freq || 800;
    const bpFreq = d.bpfreq || 2640;
    const bpQ = d.bpq || 0.8;
    const decayTime = (d.decaytime || 400) * 0.001;

    const osc1 = ctx.createOscillator();
    osc1.type = 'square';
    osc1.frequency.value = o1freq;

    const osc2 = ctx.createOscillator();
    osc2.type = 'square';
    osc2.frequency.value = o2freq;

    const mix = ctx.createGain();
    mix.gain.value = 0.5;
    osc1.connect(mix); osc2.connect(mix);

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = bpFreq;
    bp.Q.value = bpQ;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.3, t);
    gain.gain.setValueAtTime(p.level * vel * 0.22, t + 0.002);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    mix.connect(bp).connect(gain);
    this._connectToFx(gain, vid);
    osc1.start(t); osc1.stop(t + decayTime + 0.01);
    osc2.start(t); osc2.stop(t + decayTime + 0.01);
  }

  _cymbal(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const hpfFreq = d.hpf || 5000;
    const bpFreq = d.bpfreq || 3200;
    const decayTime = (d.decaytime || 1500) * 0.001 * centeredScale(p.decay, 3);
    const balance = Math.max(0, Math.min(1, (d.balance || 0.5) + (p.tone - 0.5)));

    const ratios = [1.0, 1.483, 1.932, 2.546, 2.987, 3.412];
    const merger = ctx.createGain();
    merger.gain.value = 1.0;

    for (const r of ratios) {
      const osc = ctx.createOscillator();
      osc.type = 'square';
      osc.frequency.value = 240 * r;
      const g = ctx.createGain();
      g.gain.value = 0.08;
      osc.connect(g).connect(merger);
      osc.start(t); osc.stop(t + decayTime + 0.01);
    }

    const hp = ctx.createBiquadFilter();
    hp.type = 'highpass';
    hp.frequency.value = hpfFreq;

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = bpFreq;
    bp.Q.value = 0.8;

    const hpGain = ctx.createGain();
    hpGain.gain.value = balance;
    const bpGain = ctx.createGain();
    bpGain.gain.value = 1 - balance;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.25, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    const sum = ctx.createGain();
    sum.gain.value = 1;
    merger.connect(hp).connect(hpGain).connect(sum);
    merger.connect(bp).connect(bpGain).connect(sum);
    sum.connect(gain);
    this._connectToFx(gain, vid);
  }

  _hihat(t, p, d, vel, open, vid) {
    const ctx = this.ctx;
    const hpfFreq = d.hpf || 3000;
    const lpfFreq = d.lpf || (open ? 12000 : 11000);
    const decayTime = (d.decaytime || (open ? 600 : 180)) * 0.001 * (open ? centeredScale(p.decay, 3) : 1);

    const ratios = [1.0, 1.483, 1.932, 2.546, 2.987, 3.412];
    const merger = ctx.createGain();
    merger.gain.value = 1.0;

    for (const r of ratios) {
      const osc = ctx.createOscillator();
      osc.type = 'square';
      osc.frequency.value = 320 * r;
      const g = ctx.createGain();
      g.gain.value = 0.06;
      osc.connect(g).connect(merger);
      osc.start(t); osc.stop(t + decayTime + 0.01);
    }

    const hp = ctx.createBiquadFilter();
    hp.type = 'highpass';
    hp.frequency.value = hpfFreq;
    hp.Q.value = open ? 0.7 : 1.3;

    const lp = ctx.createBiquadFilter();
    lp.type = 'lowpass';
    lp.frequency.value = lpfFreq;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.28, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    merger.connect(hp).connect(lp).connect(gain);
    this._connectToFx(gain, vid);
  }

  _claves(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const freq = d.freq || 2500;
    const decayTime = (d.decaytime || 50) * 0.001;

    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.value = freq;

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = freq;
    bp.Q.value = 15;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.4, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    osc.connect(bp).connect(gain);
    this._connectToFx(gain, vid);
    osc.start(t); osc.stop(t + decayTime + 0.02);
  }

  _noiseHit(t, amp, duration, freq, q, hpfFreq, vid) {
    const ctx = this.ctx;
    const bufferSize = Math.ceil(ctx.sampleRate * (duration + 0.05));
    const buffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
    const data = buffer.getChannelData(0);
    for (let i = 0; i < bufferSize; i++) data[i] = Math.random() * 2 - 1;

    const src = ctx.createBufferSource();
    src.buffer = buffer;

    const chain = [];

    if (hpfFreq > 0) {
      const hp = ctx.createBiquadFilter();
      hp.type = 'highpass';
      hp.frequency.value = hpfFreq;
      chain.push(hp);
    }

    if (freq > 0) {
      const bp = ctx.createBiquadFilter();
      bp.type = 'bandpass';
      bp.frequency.value = freq;
      bp.Q.value = q || 1.2;
      chain.push(bp);
    }

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(amp, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + duration);
    chain.push(gain);

    let prev = src;
    for (const node of chain) { prev.connect(node); prev = node; }
    this._connectToFx(prev, vid);

    src.start(t); src.stop(t + duration + 0.01);
  }

  _filteredNoiseHit(t, amp, duration, bpFreq, bpQ, vid) {
    const ctx = this.ctx;
    const bufferSize = Math.ceil(ctx.sampleRate * (duration + 0.05));
    const buffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
    const data = buffer.getChannelData(0);
    for (let i = 0; i < bufferSize; i++) data[i] = Math.random() * 2 - 1;

    const src = ctx.createBufferSource();
    src.buffer = buffer;

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = bpFreq;
    bp.Q.value = bpQ;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(amp, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + duration);

    src.connect(bp).connect(gain);
    this._connectToFx(gain, vid);
    src.start(t); src.stop(t + duration + 0.02);
  }
}

export { AudioEngine, VOICES, DEEP_PARAMS };
