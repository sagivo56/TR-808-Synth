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

class AudioEngine {
  constructor() {
    this.ctx = null;
    this.masterGain = null;
    this.compressor = null;
    this.params = {};
    for (const v of VOICES) {
      this.params[v.id] = { level: 0.8, tone: 0.5, decay: 0.5, snappy: 0.5, tune: 0.5 };
    }
  }

  async init() {
    if (this.ctx) return;
    this.ctx = new (window.AudioContext || window.webkitAudioContext)({ sampleRate: 44100 });

    this.compressor = this.ctx.createDynamicsCompressor();
    this.compressor.threshold.value = -12;
    this.compressor.knee.value = 6;
    this.compressor.ratio.value = 4;
    this.compressor.attack.value = 0.003;
    this.compressor.release.value = 0.15;
    this.compressor.connect(this.ctx.destination);

    this.masterGain = this.ctx.createGain();
    this.masterGain.gain.value = 0.85;
    this.masterGain.connect(this.compressor);

    if (this.ctx.state === 'suspended') {
      await this.ctx.resume();
    }
  }

  get currentTime() {
    return this.ctx ? this.ctx.currentTime : 0;
  }

  trigger(voiceIndex, time, accent) {
    if (!this.ctx) return;
    const v = VOICES[voiceIndex];
    const p = this.params[v.id];
    const t = time || this.ctx.currentTime;
    const vel = accent ? 1.0 : 0.75;

    switch (v.id) {
      case 'bd': this._bassDrum(t, p, vel); break;
      case 'rs': this._rimShot(t, p, vel); break;
      case 'sd': this._snare(t, p, vel); break;
      case 'cp': this._clap(t, p, vel); break;
      case 'lt': this._tom(t, p, vel, 100); break;
      case 'mt': this._tom(t, p, vel, 160); break;
      case 'ht': this._tom(t, p, vel, 240); break;
      case 'ma': this._maracas(t, p, vel); break;
      case 'cb': this._cowbell(t, p, vel); break;
      case 'cy': this._cymbal(t, p, vel); break;
      case 'oh': this._hihat(t, p, vel, true); break;
      case 'ch': this._hihat(t, p, vel, false); break;
      case 'lc': this._conga(t, p, vel, 170); break;
      case 'mc': this._conga(t, p, vel, 240); break;
      case 'hc': this._conga(t, p, vel, 320); break;
      case 'cl': this._claves(t, p, vel); break;
    }
  }

  _bassDrum(t, p, vel) {
    const ctx = this.ctx;
    const tuneMulti = 0.7 + p.tune * 0.6;
    const baseFreq = 55 * tuneMulti;
    const toneFreq = baseFreq * (1.0 + p.tone * 0.5);
    const decayTime = 0.15 + p.decay * 0.6;

    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(toneFreq * 4, t);
    osc.frequency.exponentialRampToValueAtTime(baseFreq, t + 0.04);

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 1.2, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    const click = ctx.createOscillator();
    click.type = 'sine';
    click.frequency.setValueAtTime(4000, t);
    click.frequency.exponentialRampToValueAtTime(200, t + 0.01);
    const clickGain = ctx.createGain();
    clickGain.gain.setValueAtTime(p.level * vel * 0.5, t);
    clickGain.gain.exponentialRampToValueAtTime(0.001, t + 0.02);

    osc.connect(gain).connect(this.masterGain);
    click.connect(clickGain).connect(this.masterGain);
    osc.start(t); osc.stop(t + decayTime + 0.01);
    click.start(t); click.stop(t + 0.03);
  }

  _rimShot(t, p, vel) {
    const ctx = this.ctx;
    const duration = 0.04;

    const osc1 = ctx.createOscillator();
    osc1.type = 'triangle';
    osc1.frequency.setValueAtTime(1700, t);

    const osc2 = ctx.createOscillator();
    osc2.type = 'triangle';
    osc2.frequency.setValueAtTime(340, t);

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.6, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + duration);

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = 1200;
    bp.Q.value = 3;

    osc1.connect(bp); osc2.connect(bp);
    bp.connect(gain).connect(this.masterGain);
    osc1.start(t); osc1.stop(t + duration + 0.01);
    osc2.start(t); osc2.stop(t + duration + 0.01);
  }

  _snare(t, p, vel) {
    const ctx = this.ctx;
    const toneFreq = 150 + p.tone * 80;
    const noiseAmt = 0.3 + p.snappy * 0.7;
    const decayTime = 0.12 + p.decay * 0.2;

    const osc = ctx.createOscillator();
    osc.type = 'triangle';
    osc.frequency.setValueAtTime(toneFreq * 2, t);
    osc.frequency.exponentialRampToValueAtTime(toneFreq, t + 0.03);

    const oscGain = ctx.createGain();
    oscGain.gain.setValueAtTime(p.level * vel * 0.5, t);
    oscGain.gain.exponentialRampToValueAtTime(0.001, t + decayTime * 0.8);

    osc.connect(oscGain).connect(this.masterGain);
    osc.start(t); osc.stop(t + decayTime + 0.01);

    this._noiseHit(t, p.level * vel * noiseAmt, decayTime, 2000, 8000);
  }

  _clap(t, p, vel) {
    const ctx = this.ctx;
    const amp = p.level * vel * 0.5;
    for (let i = 0; i < 3; i++) {
      const offset = i * 0.012;
      this._noiseHit(t + offset, amp * 0.6, 0.02, 1000, 3000);
    }
    this._noiseHit(t + 0.04, amp, 0.18, 1000, 3000);
  }

  _tom(t, p, vel, baseFreq) {
    const ctx = this.ctx;
    const tuneMulti = 0.7 + p.tune * 0.6;
    const freq = baseFreq * tuneMulti;
    const decayTime = 0.15 + p.decay * 0.4;

    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(freq * 2.5, t);
    osc.frequency.exponentialRampToValueAtTime(freq, t + 0.04);

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.8, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    osc.connect(gain).connect(this.masterGain);
    osc.start(t); osc.stop(t + decayTime + 0.01);
  }

  _maracas(t, p, vel) {
    this._noiseHit(t, p.level * vel * 0.35, 0.04, 6000, 14000);
  }

  _cowbell(t, p, vel) {
    const ctx = this.ctx;
    const decayTime = 0.25;

    const osc1 = ctx.createOscillator();
    osc1.type = 'square';
    osc1.frequency.value = 540;

    const osc2 = ctx.createOscillator();
    osc2.type = 'square';
    osc2.frequency.value = 800;

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = 680;
    bp.Q.value = 3;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.35, t);
    gain.gain.setValueAtTime(p.level * vel * 0.25, t + 0.02);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    osc1.connect(bp); osc2.connect(bp);
    bp.connect(gain).connect(this.masterGain);
    osc1.start(t); osc1.stop(t + decayTime + 0.01);
    osc2.start(t); osc2.stop(t + decayTime + 0.01);
  }

  _cymbal(t, p, vel) {
    const ctx = this.ctx;
    const toneFreq = 3000 + p.tone * 4000;
    const decayTime = 0.3 + p.decay * 1.5;

    const ratios = [1.0, 1.483, 1.932, 2.546, 2.987, 3.412];
    const merger = ctx.createGain();
    merger.gain.value = 1.0;

    for (const r of ratios) {
      const osc = ctx.createOscillator();
      osc.type = 'square';
      osc.frequency.value = 240 * r;
      const g = ctx.createGain();
      g.gain.value = 0.1;
      osc.connect(g).connect(merger);
      osc.start(t); osc.stop(t + decayTime + 0.01);
    }

    const hp = ctx.createBiquadFilter();
    hp.type = 'highpass';
    hp.frequency.value = toneFreq;

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = toneFreq * 1.5;
    bp.Q.value = 1;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.3, t);
    gain.gain.setValueAtTime(p.level * vel * 0.2, t + 0.01);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    merger.connect(hp).connect(bp).connect(gain).connect(this.masterGain);
  }

  _hihat(t, p, vel, open) {
    const ctx = this.ctx;
    const decayTime = open ? (0.15 + p.decay * 0.5) : 0.04;

    const ratios = [1.0, 1.483, 1.932, 2.546, 2.987, 3.412];
    const merger = ctx.createGain();
    merger.gain.value = 1.0;

    for (const r of ratios) {
      const osc = ctx.createOscillator();
      osc.type = 'square';
      osc.frequency.value = 320 * r;
      const g = ctx.createGain();
      g.gain.value = 0.08;
      osc.connect(g).connect(merger);
      osc.start(t); osc.stop(t + decayTime + 0.01);
    }

    const hp = ctx.createBiquadFilter();
    hp.type = 'highpass';
    hp.frequency.value = 7000;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.3, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    merger.connect(hp).connect(gain).connect(this.masterGain);
  }

  _conga(t, p, vel, baseFreq) {
    const ctx = this.ctx;
    const tuneMulti = 0.7 + p.tune * 0.6;
    const freq = baseFreq * tuneMulti;
    const decayTime = 0.12;

    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.setValueAtTime(freq * 1.8, t);
    osc.frequency.exponentialRampToValueAtTime(freq, t + 0.02);

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.7, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + decayTime);

    osc.connect(gain).connect(this.masterGain);
    osc.start(t); osc.stop(t + decayTime + 0.01);
  }

  _claves(t, p, vel) {
    const ctx = this.ctx;
    const osc = ctx.createOscillator();
    osc.type = 'sine';
    osc.frequency.value = 2500;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(p.level * vel * 0.4, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + 0.03);

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = 2500;
    bp.Q.value = 15;

    osc.connect(bp).connect(gain).connect(this.masterGain);
    osc.start(t); osc.stop(t + 0.04);
  }

  _noiseHit(t, amp, duration, lowFreq, highFreq) {
    const ctx = this.ctx;
    const bufferSize = Math.ceil(ctx.sampleRate * (duration + 0.05));
    const buffer = ctx.createBuffer(1, bufferSize, ctx.sampleRate);
    const data = buffer.getChannelData(0);
    for (let i = 0; i < bufferSize; i++) data[i] = Math.random() * 2 - 1;

    const src = ctx.createBufferSource();
    src.buffer = buffer;

    const bp = ctx.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = (lowFreq + highFreq) / 2;
    bp.Q.value = 1.2;

    const gain = ctx.createGain();
    gain.gain.setValueAtTime(amp, t);
    gain.gain.exponentialRampToValueAtTime(0.001, t + duration);

    src.connect(bp).connect(gain).connect(this.masterGain);
    src.start(t); src.stop(t + duration + 0.01);
  }
}

export { AudioEngine, VOICES };
