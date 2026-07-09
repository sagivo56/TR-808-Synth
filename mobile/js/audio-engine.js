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

const WAVE_LABELS   = ['SIN', 'TRI', 'SQR', 'SAW'];
const FILTER_LABELS = ['LPF', 'HPF', 'BPF'];

// Common param blocks — inlined per-voice to keep section membership clear.
// section values: 'AMP' | 'OSC' | 'CLICK' | 'NOISE' | 'METAL' | 'FILTER' | 'ENV'

const DEEP_PARAMS = {
  bd: [
    { id: 'velSens',      label: 'Vel Sens',   min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'freq',         label: 'Pitch',      min: 40,   max: 80,    def: 55,    skew: false, section: 'OSC'    },
    { id: 'finetune',     label: 'Fine Tune',  min: -100, max: 100,   def: 0,     skew: false, section: 'OSC'    },
    { id: 'wave',         label: 'Waveform',   min: 0,    max: 3,     def: 0,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
    { id: 'punch',        label: 'Punch',      min: 1,    max: 3,     def: 1.4,   skew: false, section: 'OSC'    },
    { id: 'retrig',       label: 'Retrig',     min: 0,    max: 1,     def: 0.5,   skew: false, section: 'OSC'    },
    { id: 'clickfreq',    label: 'Click Freq', min: 200,  max: 5000,  def: 1200,  skew: true,  section: 'CLICK'  },
    { id: 'clickamt',     label: 'Click Amt',  min: 0,    max: 1,     def: 0.12,  skew: false, section: 'CLICK'  },
    { id: 'filtertype',   label: 'Type',       min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',     min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',     min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',    min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',     min: 0,    max: 20,    def: 1,     skew: false, section: 'ENV'    },
    { id: 'bodydecay',    label: 'Decay',      min: 50,   max: 1500,  def: 650,   skew: true,  section: 'ENV'    },
    { id: 'sustain',      label: 'Hold',       min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',    min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',    min: 0,    max: 500,   def: 50,    skew: true,  section: 'ENV'    },
    { id: 'drive',        label: 'Drive',      min: 1,    max: 10,    def: 1,     skew: true,  section: 'ENV'    },
  ],
  sd: [
    { id: 'velSens',      label: 'Vel Sens',    min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'o1freq',       label: 'Osc1 Freq',   min: 100,  max: 300,   def: 180,   skew: false, section: 'OSC'    },
    { id: 'o1wave',       label: 'Osc1 Wave',   min: 0,    max: 3,     def: 1,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
    { id: 'o2freq',       label: 'Osc2 Freq',   min: 200,  max: 500,   def: 330,   skew: false, section: 'OSC'    },
    { id: 'o2wave',       label: 'Osc2 Wave',   min: 0,    max: 3,     def: 1,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
    { id: 'oscmix',       label: 'Osc Mix',     min: 0,    max: 1,     def: 0.5,   skew: false, section: 'OSC'    },
    { id: 'shelldecay',   label: 'Shell Decay', min: 20,   max: 400,   def: 130,   skew: true,  section: 'OSC'    },
    { id: 'nbpfreq',      label: 'Noise BP',    min: 800,  max: 4000,  def: 1800,  skew: true,  section: 'NOISE'  },
    { id: 'nbpq',         label: 'Noise Q',     min: 0.3,  max: 3,     def: 1.1,   skew: false, section: 'NOISE'  },
    { id: 'ndecay',       label: 'Noise Decay', min: 20,   max: 600,   def: 200,   skew: true,  section: 'NOISE'  },
    { id: 'balance',      label: 'Shell/Noise', min: 0,    max: 1,     def: 0.5,   skew: false, section: 'NOISE'  },
    { id: 'filtertype',   label: 'Type',        min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',      min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',      min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',     min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',      min: 0,    max: 20,    def: 1,     skew: false, section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',     min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',     min: 0,    max: 500,   def: 30,    skew: true,  section: 'ENV'    },
  ],
  cp: [
    { id: 'velSens',      label: 'Vel Sens',      min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'bpfreq',       label: 'BP Centre',     min: 500,  max: 2000,  def: 1000,  skew: true,  section: 'NOISE'  },
    { id: 'bpq',          label: 'BP Q',          min: 0.5,  max: 4,     def: 1.3,   skew: false, section: 'NOISE'  },
    { id: 'npulses',      label: 'Pulses',        min: 1,    max: 6,     def: 3,     skew: false, section: 'NOISE'  },
    { id: 'spacing',      label: 'Pulse Spacing', min: 3,    max: 30,    def: 10,    skew: false, section: 'NOISE'  },
    { id: 'noiseamt',     label: 'Noise Amt',     min: 0,    max: 1,     def: 0.5,   skew: false, section: 'NOISE'  },
    { id: 'filtertype',   label: 'Type',          min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',        min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',        min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',       min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',        min: 0,    max: 10,    def: 0,     skew: false, section: 'ENV'    },
    { id: 'taildecay',    label: 'Tail Decay',    min: 20,   max: 400,   def: 120,   skew: true,  section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',       min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',       min: 0,    max: 500,   def: 30,    skew: true,  section: 'ENV'    },
  ],
  rs: [
    { id: 'velSens',      label: 'Vel Sens',  min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'freq',         label: 'Tune',      min: 1000, max: 3000,  def: 1700,  skew: true,  section: 'OSC'    },
    { id: 'o1wave',       label: 'Wave 1',    min: 0,    max: 3,     def: 1,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
    { id: 'o2wave',       label: 'Wave 2',    min: 0,    max: 3,     def: 1,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
    { id: 'filtertype',   label: 'Type',      min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',    min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',    min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',   min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',    min: 0,    max: 10,    def: 0,     skew: false, section: 'ENV'    },
    { id: 'decaytime',    label: 'Decay',     min: 10,   max: 200,   def: 60,    skew: true,  section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',   min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',   min: 0,    max: 200,   def: 20,    skew: true,  section: 'ENV'    },
  ],
  cl: [
    { id: 'velSens',      label: 'Vel Sens',  min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'freq',         label: 'Tune',      min: 1500, max: 4000,  def: 2500,  skew: true,  section: 'OSC'    },
    { id: 'wave',         label: 'Waveform',  min: 0,    max: 3,     def: 0,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
    { id: 'filtertype',   label: 'Type',      min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',    min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',    min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',   min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',    min: 0,    max: 10,    def: 0,     skew: false, section: 'ENV'    },
    { id: 'decaytime',    label: 'Decay',     min: 10,   max: 200,   def: 50,    skew: true,  section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',   min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',   min: 0,    max: 200,   def: 20,    skew: true,  section: 'ENV'    },
  ],
  cb: [
    { id: 'velSens',      label: 'Vel Sens',  min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'o1freq',       label: 'Osc1 Freq', min: 400,  max: 700,   def: 540,   skew: false, section: 'OSC'    },
    { id: 'o1wave',       label: 'Osc1 Wave', min: 0,    max: 3,     def: 2,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
    { id: 'o2freq',       label: 'Osc2 Freq', min: 600,  max: 1000,  def: 800,   skew: false, section: 'OSC'    },
    { id: 'o2wave',       label: 'Osc2 Wave', min: 0,    max: 3,     def: 2,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
    { id: 'bpfreq',       label: 'BP Centre', min: 1000, max: 4000,  def: 2640,  skew: true,  section: 'OSC'    },
    { id: 'bpq',          label: 'BP Q',      min: 0.3,  max: 3,     def: 0.8,   skew: false, section: 'OSC'    },
    { id: 'filtertype',   label: 'Type',      min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',    min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',    min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',   min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',    min: 0,    max: 10,    def: 2,     skew: false, section: 'ENV'    },
    { id: 'decaytime',    label: 'Decay',     min: 100,  max: 1000,  def: 400,   skew: true,  section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',   min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',   min: 0,    max: 300,   def: 30,    skew: true,  section: 'ENV'    },
  ],
  cy: [
    { id: 'velSens',      label: 'Vel Sens',    min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'oscfreq',      label: 'Osc Base',    min: 100,  max: 500,   def: 240,   skew: true,  section: 'METAL'  },
    { id: 'hpf',          label: 'HPF',         min: 2000, max: 9000,  def: 5000,  skew: true,  section: 'METAL'  },
    { id: 'bpfreq',       label: 'Clang Band',  min: 1500, max: 6000,  def: 3200,  skew: true,  section: 'METAL'  },
    { id: 'balance',      label: 'Band Bal.',   min: 0,    max: 1,     def: 0.5,   skew: false, section: 'METAL'  },
    { id: 'humanize',     label: 'Humanize',    min: 0,    max: 1,     def: 0,     skew: false, section: 'METAL'  },
    { id: 'noiseAmt',     label: 'Noise Amt',   min: 0,    max: 1,     def: 0.25,  skew: false, section: 'NOISE'  },
    { id: 'noiseType',    label: 'Noise Type',  min: 0,    max: 1,     def: 0,     skew: false, labels: ['WHT','PNK'], section: 'NOISE' },
    { id: 'noisehpf',     label: 'Noise HPF',   min: 500,  max: 15000, def: 6000,  skew: true,  section: 'NOISE'  },
    { id: 'noiselpf',     label: 'Noise LP',    min: 2000, max: 20000, def: 14000, skew: true,  section: 'NOISE'  },
    { id: 'filtertype',   label: 'Type',        min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',      min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',      min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',     min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',      min: 0,    max: 30,    def: 2,     skew: false, section: 'ENV'    },
    { id: 'decaytime',    label: 'Decay',       min: 400,  max: 4000,  def: 1500,  skew: true,  section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',     min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',     min: 0,    max: 1000,  def: 100,   skew: true,  section: 'ENV'    },
  ],
  oh: [
    { id: 'velSens',      label: 'Vel Sens',  min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'oscfreq',      label: 'Osc Base',  min: 150,  max: 600,   def: 320,   skew: true,  section: 'METAL'  },
    { id: 'hpf',          label: 'HPF',       min: 2000, max: 12000, def: 3000,  skew: true,  section: 'METAL'  },
    { id: 'lpf',          label: 'Color',     min: 5000, max: 16000, def: 12000, skew: true,  section: 'METAL'  },
    { id: 'humanize',     label: 'Humanize',  min: 0,    max: 1,     def: 0,     skew: false, section: 'METAL'  },
    { id: 'chokeGroup',   label: 'Choke',     min: 0,    max: 1,     def: 1,     skew: false, labels: ['OFF','ON'], section: 'METAL' },
    { id: 'noiseAmt',     label: 'Noise Amt', min: 0,    max: 1,     def: 0.3,   skew: false, section: 'NOISE'  },
    { id: 'noiseType',    label: 'Noise Type',min: 0,    max: 1,     def: 0,     skew: false, labels: ['WHT','PNK'], section: 'NOISE' },
    { id: 'noisehpf',     label: 'Noise HPF', min: 500,  max: 15000, def: 4000,  skew: true,  section: 'NOISE'  },
    { id: 'noiselpf',     label: 'Noise LP',  min: 2000, max: 20000, def: 12000, skew: true,  section: 'NOISE'  },
    { id: 'filtertype',   label: 'Type',      min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',    min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',    min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',   min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',    min: 0,    max: 20,    def: 1,     skew: false, section: 'ENV'    },
    { id: 'decaytime',    label: 'Decay',     min: 150,  max: 1500,  def: 600,   skew: true,  section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',   min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',   min: 0,    max: 500,   def: 50,    skew: true,  section: 'ENV'    },
  ],
  ch: [
    { id: 'velSens',      label: 'Vel Sens',  min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'oscfreq',      label: 'Osc Base',  min: 150,  max: 600,   def: 320,   skew: true,  section: 'METAL'  },
    { id: 'hpf',          label: 'HPF',       min: 2000, max: 12000, def: 3000,  skew: true,  section: 'METAL'  },
    { id: 'lpf',          label: 'Color',     min: 5000, max: 16000, def: 11000, skew: true,  section: 'METAL'  },
    { id: 'humanize',     label: 'Humanize',  min: 0,    max: 1,     def: 0,     skew: false, section: 'METAL'  },
    { id: 'chokeGroup',   label: 'Choke',     min: 0,    max: 1,     def: 1,     skew: false, labels: ['OFF','ON'], section: 'METAL' },
    { id: 'noiseAmt',     label: 'Noise Amt', min: 0,    max: 1,     def: 0.4,   skew: false, section: 'NOISE'  },
    { id: 'noiseType',    label: 'Noise Type',min: 0,    max: 1,     def: 0,     skew: false, labels: ['WHT','PNK'], section: 'NOISE' },
    { id: 'noisehpf',     label: 'Noise HPF', min: 500,  max: 15000, def: 5000,  skew: true,  section: 'NOISE'  },
    { id: 'noiselpf',     label: 'Noise LP',  min: 2000, max: 20000, def: 11000, skew: true,  section: 'NOISE'  },
    { id: 'filtertype',   label: 'Type',      min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',    min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',    min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',   min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',    min: 0,    max: 10,    def: 0,     skew: false, section: 'ENV'    },
    { id: 'decaytime',    label: 'Decay',     min: 20,   max: 300,   def: 180,   skew: true,  section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',   min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',   min: 0,    max: 200,   def: 20,    skew: true,  section: 'ENV'    },
  ],
  ma: [
    { id: 'velSens',      label: 'Vel Sens',  min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
    { id: 'hpf',          label: 'HPF',       min: 3000, max: 12000, def: 6000,  skew: true,  section: 'NOISE'  },
    { id: 'noiseamt',     label: 'Noise Amt', min: 0,    max: 1,     def: 0.35,  skew: false, section: 'NOISE'  },
    { id: 'filtertype',   label: 'Type',      min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
    { id: 'cutoff',       label: 'Cutoff',    min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
    { id: 'resonance',    label: 'Reson.',    min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
    { id: 'filterenv',    label: 'Env Amt',   min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
    { id: 'attack',       label: 'Attack',    min: 0,    max: 5,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'decaytime',    label: 'Decay',     min: 10,   max: 100,   def: 30,    skew: true,  section: 'ENV'    },
    { id: 'sustainLevel', label: 'Sustain',   min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
    { id: 'release',      label: 'Release',   min: 0,    max: 100,   def: 10,    skew: true,  section: 'ENV'    },
  ],
};

const TOM_DEEP_TEMPLATE = (freq, decDef) => [
  { id: 'velSens',      label: 'Vel Sens',       min: 0,    max: 1,     def: 1,     skew: false, section: 'AMP'    },
  { id: 'freq',         label: 'Tune',            min: freq[0], max: freq[1], def: freq[2], skew: true, section: 'OSC' },
  { id: 'finetune',     label: 'Fine Tune',       min: -100, max: 100,   def: 0,     skew: false, section: 'OSC'    },
  { id: 'wave',         label: 'Waveform',        min: 0,    max: 3,     def: 0,     skew: false, labels: WAVE_LABELS, section: 'OSC' },
  { id: 'penvamt',      label: 'Pitch Env Amt',   min: 0,    max: 1,     def: 0.6,   skew: false, section: 'OSC'    },
  { id: 'penvtime',     label: 'Pitch Env Time',  min: 5,    max: 150,   def: 40,    skew: true,  section: 'OSC'    },
  { id: 'atknoise',     label: 'Attack Noise',    min: 0,    max: 1,     def: 0.3,   skew: false, section: 'NOISE'  },
  { id: 'noisebpfreq',  label: 'Noise Freq',      min: 500,  max: 8000,  def: 2000,  skew: true,  section: 'NOISE'  },
  { id: 'filtertype',   label: 'Type',            min: 0,    max: 2,     def: 0,     skew: false, labels: FILTER_LABELS, section: 'FILTER' },
  { id: 'cutoff',       label: 'Cutoff',          min: 20,   max: 20000, def: 20000, skew: true,  section: 'FILTER' },
  { id: 'resonance',    label: 'Reson.',          min: 0.1,  max: 20,    def: 0.7,   skew: true,  section: 'FILTER' },
  { id: 'filterenv',    label: 'Env Amt',         min: 0,    max: 1,     def: 0,     skew: false, section: 'FILTER' },
  { id: 'attack',       label: 'Attack',          min: 0,    max: 20,    def: 1,     skew: false, section: 'ENV'    },
  { id: 'decaytime',    label: 'Decay',           min: 50,   max: decDef[0], def: decDef[1], skew: true, section: 'ENV' },
  { id: 'sustainLevel', label: 'Sustain',         min: 0,    max: 1,     def: 0,     skew: false, section: 'ENV'    },
  { id: 'release',      label: 'Release',         min: 0,    max: 500,   def: 50,    skew: true,  section: 'ENV'    },
];

DEEP_PARAMS.lt = TOM_DEEP_TEMPLATE([40, 200, 90],    [2000, 600]);
DEEP_PARAMS.mt = TOM_DEEP_TEMPLATE([50, 260, 130],   [2000, 500]);
DEEP_PARAMS.ht = TOM_DEEP_TEMPLATE([60, 320, 180],   [1500, 450]);
DEEP_PARAMS.lc = TOM_DEEP_TEMPLATE([120, 400, 220],  [800,  250]);
DEEP_PARAMS.mc = TOM_DEEP_TEMPLATE([150, 460, 280],  [700,  220]);
DEEP_PARAMS.hc = TOM_DEEP_TEMPLATE([200, 560, 370],  [600,  200]);

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

    this._ohNodes = null;
    this._keepAliveEl = null;
    this._noiseBuffer = null;

    this.phaserRate = 0.5;
    this.phaserDepth = 0.467;
    this.phaserFeedbackAmt = 0.3;
    this.chorusRate = 0.8;
    this.chorusDepth = 0.3;

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

    this.bassParams = {
      level: 0.7, velSens: 0.5,
      wave: 0, punch: 0.5, pitchDecay: 0.3, sub: 0,
      tone: 0.5, resonance: 0, filterEnv: 0,
      attack: 0, decay: 0.5, sustain: 0, release: 0.05,
      drive: 1.0,
    };

    this.voicePan = {};
    this.voiceRevSend = {};
    this.voiceDlySend = {};
    this.voicePhaseSend = {};
    this.voiceChorusSend = {};
    for (const v of VOICES) {
      this.voicePan[v.id] = 0;
      this.voiceRevSend[v.id] = 0;
      this.voiceDlySend[v.id] = 0;
      this.voicePhaseSend[v.id] = 0;
      this.voiceChorusSend[v.id] = 0;
    }
  }

  setPan(id, val) { this.voicePan[id] = Math.max(-1, Math.min(1, val)); }
  getPan(id) { return this.voicePan[id] || 0; }
  setRevSend(id, val) { this.voiceRevSend[id] = Math.max(0, Math.min(1, val)); }
  getRevSend(id) { return this.voiceRevSend[id] || 0; }
  setDlySend(id, val) { this.voiceDlySend[id] = Math.max(0, Math.min(1, val)); }
  getDlySend(id) { return this.voiceDlySend[id] || 0; }
  setPhaseSend(id, val) { this.voicePhaseSend[id] = Math.max(0, Math.min(1, val)); }
  getPhaseSend(id) { return this.voicePhaseSend[id] || 0; }
  setChorusSend(id, val) { this.voiceChorusSend[id] = Math.max(0, Math.min(1, val)); }
  getChorusSend(id) { return this.voiceChorusSend[id] || 0; }

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
      this._setupPhaser();
      this._setupChorus();
      this._setupNoise();
    }

    if (this.ctx.state === 'suspended') await this.ctx.resume();

    // Try to bypass iOS silent switch via looping HTMLAudioElement (iOS 14+)
    await this._startKeepAlive();

    // Audible unlock pulse so iOS activates audio session
    const ctx = this.ctx;
    const osc = ctx.createOscillator();
    const g = ctx.createGain();
    osc.frequency.value = 440;
    g.gain.setValueAtTime(0.001, ctx.currentTime);
    g.gain.exponentialRampToValueAtTime(0.0001, ctx.currentTime + 0.05);
    osc.connect(g).connect(ctx.destination);
    osc.start(ctx.currentTime);
    osc.stop(ctx.currentTime + 0.05);
  }

  // Creates a looping silent audio element. On iOS, having an HTMLAudioElement
  // actively playing can set the AVAudioSession to "playback" category, which
  // ignores the physical Ring/Silent switch (not guaranteed on all iOS versions).
  async _startKeepAlive() {
    if (this._keepAliveEl) { try { await this._keepAliveEl.play(); } catch(e) {} return; }

    const sr = 22050, len = sr; // 1 s silence
    const buf = new ArrayBuffer(44 + len * 2);
    const v = new DataView(buf);
    const ws = (o, s) => { for (let i = 0; i < s.length; i++) v.setUint8(o + i, s.charCodeAt(i)); };
    ws(0, 'RIFF'); v.setUint32(4, 36 + len * 2, true);
    ws(8, 'WAVE'); ws(12, 'fmt ');
    v.setUint32(16, 16, true); v.setUint16(20, 1, true); v.setUint16(22, 1, true);
    v.setUint32(24, sr, true); v.setUint32(28, sr * 2, true);
    v.setUint16(32, 2, true);  v.setUint16(34, 16, true);
    ws(36, 'data'); v.setUint32(40, len * 2, true);
    // data bytes remain zeroed → silence

    const url = URL.createObjectURL(new Blob([buf], { type: 'audio/wav' }));
    const el = document.createElement('audio');
    el.src = url;
    el.loop = true;
    el.volume = 0.001;
    el.setAttribute('playsinline', '');
    el.setAttribute('preload', 'auto');
    document.body.appendChild(el);
    this._keepAliveEl = el;
    try { await el.play(); } catch(e) {}
  }

  getAudioState() {
    if (!this.ctx) return 'not created';
    return this.ctx.state;
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
      for (let i = 0; i < len; i++) d[i] = (Math.random() * 2 - 1) * Math.pow(1 - i / len, 2.5);
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

  _setupPhaser() {
    const ctx = this.ctx;
    // 4-stage allpass phaser. LFO sweeps all stage frequencies in unison.
    this.phaserInput = ctx.createGain();
    this.phaserAllpass = [];
    for (let i = 0; i < 4; i++) {
      const ap = ctx.createBiquadFilter();
      ap.type = 'allpass';
      ap.frequency.value = 1000;
      ap.Q.value = 8;
      this.phaserAllpass.push(ap);
    }
    this.phaserInput.connect(this.phaserAllpass[0]);
    for (let i = 0; i < 3; i++) this.phaserAllpass[i].connect(this.phaserAllpass[i + 1]);

    this.phaserFeedbackGain = ctx.createGain();
    this.phaserFeedbackGain.gain.value = this.phaserFeedbackAmt;
    this.phaserAllpass[3].connect(this.phaserFeedbackGain);
    this.phaserFeedbackGain.connect(this.phaserInput);

    this.phaserLfo = ctx.createOscillator();
    this.phaserLfo.type = 'sine';
    this.phaserLfo.frequency.value = this.phaserRate;
    this.phaserLfoDepth = ctx.createGain();
    this.phaserLfoDepth.gain.value = this.phaserDepth * 1500;
    this.phaserLfo.connect(this.phaserLfoDepth);
    this.phaserAllpass.forEach(ap => this.phaserLfoDepth.connect(ap.frequency));
    this.phaserLfo.start();

    this.phaserSend = ctx.createGain();
    this.phaserSend.gain.value = 1.0;
    this.phaserReturn = ctx.createGain();
    this.phaserReturn.gain.value = 0.5;

    this.phaserSend.connect(this.phaserInput);
    this.phaserAllpass[3].connect(this.phaserReturn);
    this.phaserReturn.connect(this.masterGain);
  }

  _setupChorus() {
    const ctx = this.ctx;
    // Stereo chorus: two delay lines with slightly different LFO rates.
    this.chorusDelayL = ctx.createDelay(0.05);
    this.chorusDelayR = ctx.createDelay(0.05);
    this.chorusDelayL.delayTime.value = 0.015;
    this.chorusDelayR.delayTime.value = 0.017;

    this.chorusLfoL = ctx.createOscillator();
    this.chorusLfoL.type = 'sine';
    this.chorusLfoL.frequency.value = this.chorusRate;
    this.chorusLfoR = ctx.createOscillator();
    this.chorusLfoR.type = 'sine';
    this.chorusLfoR.frequency.value = this.chorusRate * 1.3;

    this.chorusLfoDepthL = ctx.createGain();
    this.chorusLfoDepthL.gain.value = this.chorusDepth * 0.01;
    this.chorusLfoDepthR = ctx.createGain();
    this.chorusLfoDepthR.gain.value = this.chorusDepth * 0.01;

    this.chorusLfoL.connect(this.chorusLfoDepthL);
    this.chorusLfoDepthL.connect(this.chorusDelayL.delayTime);
    this.chorusLfoR.connect(this.chorusLfoDepthR);
    this.chorusLfoDepthR.connect(this.chorusDelayR.delayTime);
    this.chorusLfoL.start();
    this.chorusLfoR.start();

    const merger = ctx.createChannelMerger(2);
    this.chorusDelayL.connect(merger, 0, 0);
    this.chorusDelayR.connect(merger, 0, 1);

    this.chorusSend = ctx.createGain();
    this.chorusSend.gain.value = 1.0;
    this.chorusReturn = ctx.createGain();
    this.chorusReturn.gain.value = 0.5;

    this.chorusSend.connect(this.chorusDelayL);
    this.chorusSend.connect(this.chorusDelayR);
    merger.connect(this.chorusReturn);
    this.chorusReturn.connect(this.masterGain);
  }

  _setupNoise() {
    // Pre-generate a shared 4-second white-noise buffer reused by all voices.
    // Eliminates per-trigger buffer allocation and Math.random() filling
    // that causes main-thread jank when many voices fire simultaneously.
    const sr = this.ctx.sampleRate;
    this._noiseBuffer = this.ctx.createBuffer(1, sr * 4, sr);
    const d = this._noiseBuffer.getChannelData(0);
    for (let i = 0; i < sr * 4; i++) d[i] = Math.random() * 2 - 1;
  }

  _noiseSource() {
    const src = this.ctx.createBufferSource();
    src.buffer = this._noiseBuffer;
    src.loop = true;
    // Random start offset so each trigger sounds different.
    src.loopStart = 0;
    src.loopEnd = this._noiseBuffer.duration;
    return src;
  }

  get currentTime() { return this.ctx ? this.ctx.currentTime : 0; }

  trigger(voiceIndex, time, accent) {
    if (!this.ctx) return;
    const v = VOICES[voiceIndex];
    const p = this.params[v.id];
    const d = this.deep[v.id] || {};
    const t = time || this.ctx.currentTime;
    const vel = accent ? Math.min(1.0, 0.75 * this.accentLevel) : 0.75;
    switch (v.id) {
      case 'bd': this._bassDrum(t, p, d, vel, v.id); break;
      case 'rs': this._rimShot(t, p, d, vel, v.id); break;
      case 'sd': this._snare(t, p, d, vel, v.id); break;
      case 'cp': this._clap(t, p, d, vel, v.id); break;
      case 'lt': case 'mt': case 'ht': case 'lc': case 'mc': case 'hc':
        this._tomConga(t, p, d, vel, v.id); break;
      case 'ma': this._maracas(t, p, d, vel, v.id); break;
      case 'cb': this._cowbell(t, p, d, vel, v.id); break;
      case 'cy': this._cymbal(t, p, d, vel, v.id); break;
      case 'oh': this._hihat(t, p, d, vel, true, v.id); break;
      case 'ch': this._hihat(t, p, d, vel, false, v.id); break;
      case 'cl': this._claves(t, p, d, vel, v.id); break;
    }
  }

  triggerBass(midiNote, time, velocity = 0.75) {
    if (!this.ctx) return;
    const freq = 440 * Math.pow(2, (midiNote - 69) / 12);
    const p = this.bassParams;
    const t = time || this.ctx.currentTime;
    const ctx = this.ctx;

    const atkS  = Math.max(0, (p.attack  || 0)) * 0.001;
    const decS  = Math.max(0.05, 0.15 + (p.decay  || 0.5) * 0.85);
    const relS  = Math.max(0.01, (p.release || 0.05) * 0.5);
    const sus   = Math.max(0.0001, Math.min(1, p.sustain || 0));
    const totalDur = atkS + decS + relS + 0.05;

    // Main oscillator
    const osc = ctx.createOscillator();
    osc.type = this._getWave(p.wave || 0);
    const punchMult = 1.5 + (p.punch || 0.5);
    const pitchDecS = Math.max(0.005, (p.pitchDecay || 0.3) * 0.1);
    osc.frequency.setValueAtTime(freq * punchMult, t);
    osc.frequency.exponentialRampToValueAtTime(freq, t + pitchDecS);

    // Sub oscillator (one octave down)
    let subNode = null;
    if ((p.sub || 0) > 0.01) {
      const subOsc = ctx.createOscillator();
      subOsc.type = 'sine';
      subOsc.frequency.value = freq / 2;
      const subGain = ctx.createGain();
      subGain.gain.value = p.sub;
      subOsc.connect(subGain);
      subNode = subGain;
      subOsc.start(t); subOsc.stop(t + totalDur);
    }

    // Drive / waveshaper
    let output = osc;
    if ((p.drive || 1) > 1.1) {
      const ws = ctx.createWaveShaper();
      ws.curve = this._tanhCurve(p.drive);
      const pg = ctx.createGain(); pg.gain.value = p.drive;
      osc.connect(pg).connect(ws); output = ws;
    }

    // Filter with envelope
    const toneFilter = ctx.createBiquadFilter();
    toneFilter.type = 'lowpass';
    toneFilter.Q.value = 0.1 + (p.resonance || 0) * 12;
    const baseCutoff = 200 + (p.tone || 0.5) * 2000;
    const envBoost = (p.filterEnv || 0) * 4000;
    toneFilter.frequency.setValueAtTime(Math.min(20000, baseCutoff + envBoost), t);
    toneFilter.frequency.exponentialRampToValueAtTime(Math.max(20, baseCutoff), t + atkS + decS * 0.6);

    // Amp envelope
    const velScale = (1 - (p.velSens || 0.5)) + (p.velSens || 0.5) * velocity;
    const level = (p.level || 0.7) * velScale;
    const gain = ctx.createGain();
    if (atkS > 0.001) {
      gain.gain.setValueAtTime(0.0001, t);
      gain.gain.linearRampToValueAtTime(level, t + atkS);
    } else {
      gain.gain.setValueAtTime(level, t);
    }
    const susLevel = Math.max(0.0001, level * sus);
    gain.gain.exponentialRampToValueAtTime(susLevel, t + atkS + decS);
    gain.gain.exponentialRampToValueAtTime(0.0001, t + atkS + decS + relS);

    // Wire up
    if (subNode) subNode.connect(toneFilter);
    output.connect(toneFilter);
    toneFilter.connect(gain);
    gain.connect(this.masterGain);
    osc.start(t); osc.stop(t + totalDur);
  }

  // ── helpers ───────────────────────────────────────────────────────────────

  _tanhCurve(drive) {
    const n = 256, c = new Float32Array(n);
    for (let i = 0; i < n; i++) { const x = (i / (n - 1)) * 2 - 1; c[i] = Math.tanh(drive * x); }
    return c;
  }

  _getWave(val) {
    return ['sine', 'triangle', 'square', 'sawtooth'][Math.max(0, Math.min(3, Math.round(val || 0)))];
  }

  // Schedules ADSR on a gain node. Returns total duration in seconds.
  _applyEnv(gain, amp, atkMs, decayMs, t, sustainLevel = 0, releaseMs = 30) {
    const atk = Math.max(0, (atkMs || 0)) * 0.001;
    const dec = Math.max(0.001, (decayMs || 500)) * 0.001;
    const sus = Math.max(0.0001, Math.min(1, sustainLevel || 0));
    const rel = Math.max(0.01, (releaseMs || 30)) * 0.001;

    if (atk > 0.001) {
      gain.gain.setValueAtTime(0.0001, t);
      gain.gain.linearRampToValueAtTime(amp, t + atk);
    } else {
      gain.gain.setValueAtTime(amp, t);
    }
    const decTarget = Math.max(0.0001, amp * sus);
    gain.gain.exponentialRampToValueAtTime(decTarget, t + atk + dec);
    gain.gain.setValueAtTime(decTarget, t + atk + dec);
    gain.gain.exponentialRampToValueAtTime(0.0001, t + atk + dec + rel);
    return atk + dec + rel;
  }

  // Inserts an optional voice-level BiquadFilter with envelope before FX routing.
  _connectFiltered(sourceNode, vid, d, t, atkMs, decayMs) {
    const cutoff = d && d.cutoff;
    if (cutoff && cutoff < 19000) {
      const ctx = this.ctx;
      const types = ['lowpass', 'highpass', 'bandpass'];
      const f = ctx.createBiquadFilter();
      f.type = types[Math.max(0, Math.min(2, Math.round(d.filtertype || 0)))];
      f.frequency.value = cutoff;
      f.Q.value = Math.max(0.1, d.resonance || 0.7);

      const envAmt = d.filterenv || 0;
      if (envAmt > 0.01) {
        const peak = Math.min(20000, cutoff * (1 + envAmt * 6));
        const sweepTime = (atkMs || 0) * 0.001 + (decayMs || 500) * 0.001 * 0.25;
        f.frequency.setValueAtTime(peak, t);
        f.frequency.exponentialRampToValueAtTime(Math.max(1, cutoff), t + sweepTime);
      }

      sourceNode.connect(f);
      this._connectToFx(f, vid);
    } else {
      this._connectToFx(sourceNode, vid);
    }
  }

  _connectToFx(node, voiceId) {
    if (voiceId && this.voicePan[voiceId] !== 0) {
      const pan = this.ctx.createStereoPanner();
      pan.pan.value = this.voicePan[voiceId];
      node.connect(pan).connect(this.masterGain);
    } else {
      node.connect(this.masterGain);
    }
    if (this.reverbSend && voiceId && (this.voiceRevSend[voiceId] || 0) > 0.01) {
      const rg = this.ctx.createGain();
      rg.gain.value = this.voiceRevSend[voiceId];
      node.connect(rg).connect(this.reverbSend);
    }
    if (this.delaySend && voiceId && (this.voiceDlySend[voiceId] || 0) > 0.01) {
      const dg = this.ctx.createGain();
      dg.gain.value = this.voiceDlySend[voiceId];
      node.connect(dg).connect(this.delaySend);
    }
    if (this.phaserSend && voiceId && (this.voicePhaseSend[voiceId] || 0) > 0.01) {
      const pg = this.ctx.createGain();
      pg.gain.value = this.voicePhaseSend[voiceId];
      node.connect(pg).connect(this.phaserSend);
    }
    if (this.chorusSend && voiceId && (this.voiceChorusSend[voiceId] || 0) > 0.01) {
      const cg = this.ctx.createGain();
      cg.gain.value = this.voiceChorusSend[voiceId];
      node.connect(cg).connect(this.chorusSend);
    }
  }

  // ── synthesis ─────────────────────────────────────────────────────────────

  _bassDrum(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const fineTune = Math.pow(2, (d.finetune || 0) / 1200);
    const baseFreq = (d.freq || 55) * centeredPitch(p.tune, 12) * fineTune;
    const punchMult = Math.max(1, Math.min(4, d.punch || 1.4));
    const startFreq = baseFreq * punchMult;
    const bodyDecaySec = (d.bodydecay || 650) * 0.001 * centeredScale(p.decay, 4) * (1 + (d.sustain || 0) * 8);
    const driveAmt = d.drive || 1;
    const retrigAmt = d.retrig || 0.5;
    const sweepTime = 0.015 + 0.5 / startFreq;

    const osc = ctx.createOscillator();
    osc.type = this._getWave(d.wave);
    osc.frequency.setValueAtTime(startFreq, t);
    osc.frequency.exponentialRampToValueAtTime(Math.max(baseFreq, 20), t + sweepTime);
    if (retrigAmt > 0.05) {
      osc.frequency.setValueAtTime(baseFreq * (1 + retrigAmt * 0.15), t + sweepTime);
      osc.frequency.exponentialRampToValueAtTime(Math.max(baseFreq, 20), t + sweepTime + 0.03);
    }

    let output = osc;
    if (driveAmt > 1.05) {
      const ws = ctx.createWaveShaper(); ws.curve = this._tanhCurve(driveAmt);
      osc.connect(ws); output = ws;
    }

    const gain = ctx.createGain();
    const amp = p.level * eVel * 0.9;
    const dur = this._applyEnv(gain, amp, d.attack, bodyDecaySec * 1000, t, d.sustainLevel, d.release);
    output.connect(gain);
    this._connectFiltered(gain, vid, d, t, d.attack, bodyDecaySec * 1000);
    osc.start(t); osc.stop(t + dur + 0.05);

    const clickAmt = d.clickamt ?? 0.12;
    if (p.tone > 0.01 && clickAmt > 0.001) {
      const click = ctx.createOscillator();
      click.type = 'sine';
      click.frequency.value = d.clickfreq || 1200;
      const cg = ctx.createGain();
      cg.gain.setValueAtTime(p.tone * p.level * eVel * clickAmt, t);
      cg.gain.exponentialRampToValueAtTime(0.001, t + 0.003);
      click.connect(cg).connect(this.masterGain);
      click.start(t); click.stop(t + 0.008);
    }
  }

  _rimShot(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const freq = d.freq || 1700;
    const decayMs = d.decaytime || 60;

    const osc1 = ctx.createOscillator(); osc1.type = this._getWave(d.o1wave ?? 1); osc1.frequency.value = freq;
    const osc2 = ctx.createOscillator(); osc2.type = this._getWave(d.o2wave ?? 1); osc2.frequency.value = freq * 0.56;
    const bp = ctx.createBiquadFilter(); bp.type = 'bandpass'; bp.frequency.value = freq * 0.7; bp.Q.value = 4;

    const gain = ctx.createGain();
    const dur = this._applyEnv(gain, p.level * eVel * 0.55, d.attack, decayMs, t, d.sustainLevel, d.release);
    osc1.connect(bp); osc2.connect(bp); bp.connect(gain);
    this._connectFiltered(gain, vid, d, t, d.attack, decayMs);
    osc1.start(t); osc1.stop(t + dur + 0.01);
    osc2.start(t); osc2.stop(t + dur + 0.01);
  }

  _snare(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const o1freq = d.o1freq || 180;
    const o2freq = d.o2freq || 330;
    const oscMix = Math.max(0, Math.min(1, (d.oscmix || 0.5) + (p.tone - 0.5)));
    const shellDecayMs = d.shelldecay || 130;
    const noiseDecay = (d.ndecay || 200) * 0.001 * centeredScale(p.snappy, 2.5);
    const balance = d.balance || 0.5;
    const snappyMod = centeredScale(p.snappy, 2);

    const osc1 = ctx.createOscillator();
    osc1.type = this._getWave(d.o1wave ?? 1);
    osc1.frequency.setValueAtTime(o1freq * 1.7, t);
    osc1.frequency.exponentialRampToValueAtTime(o1freq, t + 0.028);
    const osc2 = ctx.createOscillator();
    osc2.type = this._getWave(d.o2wave ?? 1);
    osc2.frequency.setValueAtTime(o2freq * 1.5, t);
    osc2.frequency.exponentialRampToValueAtTime(o2freq, t + 0.028);

    const g1 = ctx.createGain(); g1.gain.value = (1 - oscMix) * 1.5;
    const g2 = ctx.createGain(); g2.gain.value = oscMix * 1.5;

    const shellGain = ctx.createGain();
    const dur = this._applyEnv(shellGain, p.level * eVel * (1 - balance) * 0.7, d.attack, shellDecayMs, t, d.sustainLevel, d.release);
    osc1.connect(g1).connect(shellGain);
    osc2.connect(g2).connect(shellGain);

    const hpf = ctx.createBiquadFilter(); hpf.type = 'highpass'; hpf.frequency.value = d.hpf || 300;
    shellGain.connect(hpf);
    this._connectFiltered(hpf, vid, d, t, d.attack, shellDecayMs);
    osc1.start(t); osc1.stop(t + dur + 0.02);
    osc2.start(t); osc2.stop(t + dur + 0.02);

    this._noiseHit(t, p.level * eVel * balance * snappyMod * 0.7, noiseDecay,
                   d.nbpfreq || 1800, d.nbpq || 1.1, d.hpf || 300, vid, d.attack);
  }

  _clap(t, p, d, vel, vid) {
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const bpFreq = d.bpfreq || 1000;
    const bpQ = d.bpq || 1.3;
    const numPulses = Math.round(d.npulses || 3);
    const spacing = (d.spacing || 10) * 0.001;
    const tailDecayMs = d.taildecay || 120;
    const amp = p.level * eVel * (d.noiseamt ?? 0.5);

    for (let i = 0; i < numPulses; i++) {
      this._filteredNoiseHit(t + i * spacing, amp * 0.6, 0.005, bpFreq, bpQ, vid, d.attack);
    }
    this._filteredNoiseHit(t + numPulses * spacing, amp, tailDecayMs * 0.001, bpFreq, bpQ, vid, d.attack);
  }

  _tomConga(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const fineTune = Math.pow(2, (d.finetune || 0) / 1200);
    const baseFreq = (d.freq || 130) * centeredPitch(p.tune, 12) * fineTune;
    const decayMs = d.decaytime || 500;
    const penvAmt = d.penvamt || 0.6;
    const penvTime = (d.penvtime || 40) * 0.001;

    const osc = ctx.createOscillator();
    osc.type = this._getWave(d.wave);
    osc.frequency.setValueAtTime(baseFreq * (1 + penvAmt), t);
    osc.frequency.exponentialRampToValueAtTime(Math.max(baseFreq, 20), t + penvTime);

    const gain = ctx.createGain();
    const dur = this._applyEnv(gain, p.level * eVel * 0.8, d.attack, decayMs, t, d.sustainLevel, d.release);
    osc.connect(gain);
    this._connectFiltered(gain, vid, d, t, d.attack, decayMs);
    osc.start(t); osc.stop(t + dur + 0.05);

    if ((d.atknoise || 0.3) > 0.01) {
      const noiseBpFreq = d.noisebpfreq || 2000;
      this._noiseHit(t, p.level * eVel * (d.atknoise || 0.3) * 0.3, 0.004, noiseBpFreq, 1.0, 0, vid);
    }
  }

  _maracas(t, p, d, vel, vid) {
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const hpf = d.hpf || 6000;
    const decayMs = d.decaytime || 30;
    const noiseAmt = d.noiseamt ?? 0.35;
    this._noiseHit(t, p.level * eVel * noiseAmt, decayMs * 0.001, hpf, 1.0, hpf, vid, d.attack);
  }

  _cowbell(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const decayMs = d.decaytime || 400;

    const osc1 = ctx.createOscillator(); osc1.type = this._getWave(d.o1wave ?? 2); osc1.frequency.value = d.o1freq || 540;
    const osc2 = ctx.createOscillator(); osc2.type = this._getWave(d.o2wave ?? 2); osc2.frequency.value = d.o2freq || 800;

    const mix = ctx.createGain(); mix.gain.value = 0.5;
    osc1.connect(mix); osc2.connect(mix);
    const bp = ctx.createBiquadFilter(); bp.type = 'bandpass'; bp.frequency.value = d.bpfreq || 2640; bp.Q.value = d.bpq || 0.8;

    const gain = ctx.createGain();
    const dur = this._applyEnv(gain, p.level * eVel * 0.3, d.attack, decayMs, t, d.sustainLevel, d.release);
    mix.connect(bp).connect(gain);
    this._connectFiltered(gain, vid, d, t, d.attack, decayMs);
    osc1.start(t); osc1.stop(t + dur + 0.01);
    osc2.start(t); osc2.stop(t + dur + 0.01);
  }

  _cymbal(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const hPitch = (d.humanize || 0) > 0 ? (1 + (Math.random() - 0.5) * (d.humanize) * 0.04) : 1;
    const hVel   = (d.humanize || 0) > 0 ? eVel * (1 + (Math.random() - 0.5) * (d.humanize) * 0.1) : eVel;
    const decayMs = (d.decaytime || 1500) * centeredScale(p.decay, 3);
    const balance = Math.max(0, Math.min(1, (d.balance || 0.5) + (p.tone - 0.5)));
    const oscBase = (d.oscfreq || 240) * hPitch;

    const ratios = [1.0, 1.483, 1.932, 2.546, 2.987, 3.412];
    const merger = ctx.createGain(); merger.gain.value = 1.0;
    for (const r of ratios) {
      const osc = ctx.createOscillator(); osc.type = 'square'; osc.frequency.value = oscBase * r;
      const g = ctx.createGain(); g.gain.value = 0.08;
      osc.connect(g).connect(merger);
      osc.start(t); osc.stop(t + decayMs * 0.001 + 0.01);
    }

    const hp = ctx.createBiquadFilter(); hp.type = 'highpass'; hp.frequency.value = d.hpf || 5000;
    const bp2 = ctx.createBiquadFilter(); bp2.type = 'bandpass'; bp2.frequency.value = d.bpfreq || 3200; bp2.Q.value = 0.8;
    const hpG = ctx.createGain(); hpG.gain.value = balance;
    const bpG = ctx.createGain(); bpG.gain.value = 1 - balance;
    const sum = ctx.createGain(); sum.gain.value = 1;
    merger.connect(hp).connect(hpG).connect(sum);
    merger.connect(bp2).connect(bpG).connect(sum);

    const gain = ctx.createGain();
    this._applyEnv(gain, p.level * hVel * 0.25, d.attack, decayMs, t, d.sustainLevel, d.release);
    sum.connect(gain);

    // Noise layer — mixed into the same ADSR gain
    const noiseAmt = d.noiseAmt ?? 0.25;
    if (noiseAmt > 0.005) {
      const nSrc = this._noiseSource();

      const nHpf = ctx.createBiquadFilter(); nHpf.type = 'highpass'; nHpf.frequency.value = Math.max(20, d.noisehpf ?? 6000);
      const nLpf = ctx.createBiquadFilter(); nLpf.type = 'lowpass';  nLpf.frequency.value = Math.min(20000, d.noiselpf ?? 14000);
      const nG = ctx.createGain(); nG.gain.value = noiseAmt;

      if ((d.noiseType ?? 0) > 0.5) {
        const nPink = ctx.createBiquadFilter(); nPink.type = 'lowpass'; nPink.frequency.value = 3000;
        nSrc.connect(nHpf); nHpf.connect(nPink); nPink.connect(nLpf);
      } else {
        nSrc.connect(nHpf); nHpf.connect(nLpf);
      }
      nLpf.connect(nG); nG.connect(gain);
      nSrc.start(t); nSrc.stop(t + decayMs * 0.001 + 0.1);
    }

    this._connectFiltered(gain, vid, d, t, d.attack, decayMs);
  }

  _hihat(t, p, d, vel, open, vid) {
    const ctx = this.ctx;
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const hPitch = (d.humanize || 0) > 0 ? (1 + (Math.random() - 0.5) * (d.humanize) * 0.04) : 1;
    const hVel   = (d.humanize || 0) > 0 ? eVel * (1 + (Math.random() - 0.5) * (d.humanize) * 0.1) : eVel;

    // Choke: CH closes OH when choke is enabled
    if (!open && (d.chokeGroup ?? 1) > 0.5 && this._ohNodes) {
      const oh = this._ohNodes;
      oh.gain.gain.cancelScheduledValues(t);
      oh.gain.gain.setValueAtTime(oh.peakAmp * 0.5, t);
      oh.gain.gain.exponentialRampToValueAtTime(0.0001, t + 0.004);
      oh.oscs.forEach(o => { try { o.stop(t + 0.006); } catch(e) {} });
      if (oh.noiseSrc) { try { oh.noiseSrc.stop(t + 0.006); } catch(e) {} }
      this._ohNodes = null;
    }

    const hpfFreq = d.hpf || 3000;
    const lpfFreq = d.lpf || (open ? 12000 : 11000);
    const decayMs = (d.decaytime || (open ? 600 : 180)) * (open ? centeredScale(p.decay, 3) : 1);
    const oscBase = (d.oscfreq || 320) * hPitch;

    const ratios = [1.0, 1.483, 1.932, 2.546, 2.987, 3.412];
    const merger = ctx.createGain(); merger.gain.value = 1.0;
    const oscs = [];
    for (const r of ratios) {
      const osc = ctx.createOscillator(); osc.type = 'square'; osc.frequency.value = oscBase * r;
      const g = ctx.createGain(); g.gain.value = 0.06;
      osc.connect(g).connect(merger);
      osc.start(t); osc.stop(t + decayMs * 0.001 + 0.05);
      oscs.push(osc);
    }

    const hp = ctx.createBiquadFilter(); hp.type = 'highpass'; hp.frequency.value = hpfFreq; hp.Q.value = open ? 0.7 : 1.3;
    const lp = ctx.createBiquadFilter(); lp.type = 'lowpass'; lp.frequency.value = lpfFreq;

    const gain = ctx.createGain();
    const amp = p.level * hVel * 0.28;
    this._applyEnv(gain, amp, d.attack, decayMs, t, d.sustainLevel, d.release);
    merger.connect(hp).connect(lp).connect(gain);

    // Noise layer — mixed into the same ADSR gain so it shares the envelope
    let noiseSrc = null;
    const noiseAmt = d.noiseAmt ?? 0.3;
    if (noiseAmt > 0.005) {
      noiseSrc = this._noiseSource();

      const nHpf = ctx.createBiquadFilter(); nHpf.type = 'highpass'; nHpf.frequency.value = Math.max(20, d.noisehpf ?? 4000);
      const nLpf = ctx.createBiquadFilter(); nLpf.type = 'lowpass';  nLpf.frequency.value = Math.min(20000, d.noiselpf ?? 12000);
      const nG = ctx.createGain(); nG.gain.value = noiseAmt;

      if ((d.noiseType ?? 0) > 0.5) {
        // Pink approximation: extra gentle rolloff before main LPF
        const nPink = ctx.createBiquadFilter(); nPink.type = 'lowpass'; nPink.frequency.value = 3000;
        noiseSrc.connect(nHpf); nHpf.connect(nPink); nPink.connect(nLpf);
      } else {
        noiseSrc.connect(nHpf); nHpf.connect(nLpf);
      }
      nLpf.connect(nG); nG.connect(gain);
      noiseSrc.start(t); noiseSrc.stop(t + decayMs * 0.001 + 0.1);
    }

    this._connectFiltered(gain, vid, d, t, d.attack, decayMs);

    if (open) this._ohNodes = { gain, oscs, peakAmp: amp, noiseSrc };
  }

  _claves(t, p, d, vel, vid) {
    const ctx = this.ctx;
    const eVel = (1 - (d.velSens ?? 1)) + (d.velSens ?? 1) * vel;
    const freq = d.freq || 2500;
    const decayMs = d.decaytime || 50;

    const osc = ctx.createOscillator(); osc.type = this._getWave(d.wave); osc.frequency.value = freq;
    const bp = ctx.createBiquadFilter(); bp.type = 'bandpass'; bp.frequency.value = freq; bp.Q.value = 15;

    const gain = ctx.createGain();
    const dur = this._applyEnv(gain, p.level * eVel * 0.4, d.attack, decayMs, t, d.sustainLevel, d.release);
    osc.connect(bp).connect(gain);
    this._connectFiltered(gain, vid, d, t, d.attack, decayMs);
    osc.start(t); osc.stop(t + dur + 0.02);
  }

  _noiseHit(t, amp, duration, freq, q, hpfFreq, vid, attackMs = 0) {
    const ctx = this.ctx;
    const src = this._noiseSource();

    const chain = [];
    if (hpfFreq > 0) { const h = ctx.createBiquadFilter(); h.type = 'highpass'; h.frequency.value = hpfFreq; chain.push(h); }
    if (freq > 0)    { const b = ctx.createBiquadFilter(); b.type = 'bandpass'; b.frequency.value = freq; b.Q.value = q || 1.2; chain.push(b); }

    const gain = ctx.createGain();
    const atk = this._applyEnv(gain, amp, attackMs, duration * 1000, t);
    chain.push(gain);

    let prev = src;
    for (const n of chain) { prev.connect(n); prev = n; }
    this._connectToFx(prev, vid);
    src.start(t); src.stop(t + atk + duration + 0.01);
  }

  _filteredNoiseHit(t, amp, duration, bpFreq, bpQ, vid, attackMs = 0) {
    const ctx = this.ctx;
    const src = this._noiseSource();

    const bp = ctx.createBiquadFilter(); bp.type = 'bandpass'; bp.frequency.value = bpFreq; bp.Q.value = bpQ;
    const gain = ctx.createGain();
    const atk = this._applyEnv(gain, amp, attackMs, duration * 1000, t);

    src.connect(bp).connect(gain);
    this._connectToFx(gain, vid);
    src.start(t); src.stop(t + atk + duration + 0.02);
  }
}

export { AudioEngine, VOICES, DEEP_PARAMS };
