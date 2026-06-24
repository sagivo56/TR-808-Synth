import { AudioEngine, VOICES, DEEP_PARAMS } from './audio-engine.js';
import { Sequencer, NUM_VOICES } from './sequencer.js';

const engine = new AudioEngine();
const seq = new Sequencer(engine);

let selectedVoice = 0;
let audioInitialized = false;
let currentTab = 'drum';

const FACTORY_PATTERNS = [
  {
    name: 'Classic 808',
    bpm: 120,
    data: {
      0:  [1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0],
      2:  [0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0],
      11: [1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0],
      10: [0,0,0,0, 0,0,0,0, 0,0,1,0, 0,0,0,0],
    }
  },
  {
    name: 'Boom Bap',
    bpm: 90,
    data: {
      0:  [1,0,0,0, 0,0,0,0, 1,0,1,0, 0,0,0,0],
      2:  [0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0],
      11: [1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0],
      10: [0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0],
    }
  },
  {
    name: 'Trap',
    bpm: 140,
    data: {
      0:  [1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0],
      2:  [0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0],
      11: [1,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1],
      10: [0,0,0,0, 0,0,1,0, 0,0,0,0, 0,0,1,0],
      7:  [0,0,0,0, 0,0,0,0, 0,0,0,1, 0,0,0,0],
    }
  },
  {
    name: 'Electro',
    bpm: 130,
    data: {
      0:  [1,0,0,0, 0,0,1,0, 0,0,1,0, 0,0,0,0],
      2:  [0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,1],
      3:  [0,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,0,0],
      11: [1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0],
      8:  [0,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,1,0],
    }
  },
  {
    name: 'Latin',
    bpm: 110,
    data: {
      0:  [1,0,0,0, 0,0,1,0, 0,0,0,0, 1,0,0,0],
      1:  [0,0,0,1, 0,0,0,0, 0,1,0,0, 0,0,0,0],
      12: [0,0,1,0, 0,0,0,0, 1,0,0,0, 0,0,1,0],
      13: [1,0,0,0, 0,1,0,0, 0,0,0,0, 1,0,0,0],
      15: [1,0,0,1, 0,0,1,0, 0,1,0,0, 1,0,0,0],
      11: [1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,1,0],
      7:  [0,0,0,0, 0,1,0,0, 0,0,0,0, 0,1,0,0],
    }
  },
];

const BASS_NOTES = ['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'];

async function initAudio() {
  if (audioInitialized) return;
  await engine.init();
  audioInitialized = true;
  document.getElementById('init-overlay').classList.add('hidden');
}

function buildUI() {
  buildHeader();
  buildTabBar();
  buildInstrumentBar();
  buildParamsPanel();
  buildStepGrid();
  buildInitOverlay();

  seq.onStep = (step) => updatePlayhead(step);

  document.addEventListener('visibilitychange', () => {
    if (document.hidden && seq.running) {
      seq.stop();
      document.getElementById('play-btn').textContent = '▶';
      document.getElementById('play-btn').classList.remove('active');
    }
  });
}

function buildInitOverlay() {
  const overlay = document.createElement('div');
  overlay.id = 'init-overlay';
  overlay.innerHTML = `
    <div class="init-content">
      <div class="init-logo">TR-808</div>
      <div class="init-sub">RHYTHM COMPOSER</div>
      <button class="init-btn" id="start-btn">TAP TO START</button>
    </div>
  `;
  document.body.appendChild(overlay);
  const handler = (e) => { e.preventDefault(); initAudio(); };
  document.getElementById('start-btn').addEventListener('click', handler);
  document.getElementById('start-btn').addEventListener('touchend', handler);
}

function buildHeader() {
  const header = document.getElementById('header');
  header.innerHTML = `
    <div class="header-row">
      <div class="header-left">
        <button class="transport-btn" id="play-btn">▶</button>
        <div class="tempo-group">
          <label>BPM</label>
          <input type="range" id="tempo-slider" min="40" max="300" value="120" step="1">
          <span id="tempo-val">120</span>
        </div>
      </div>
      <div class="header-center">
        <span class="logo">TR-808</span>
      </div>
      <div class="header-right">
        <div class="swing-group">
          <label>SWING</label>
          <input type="range" id="swing-slider" min="0" max="75" value="0" step="1">
          <span id="swing-val">0%</span>
        </div>
        <select id="pattern-select" class="header-select">
          <option value="factory-none">PATTERN</option>
          ${FACTORY_PATTERNS.map((p, i) => `<option value="factory-${i}">${p.name}</option>`).join('')}
        </select>
        <button class="clear-btn" id="clear-btn">CLR</button>
      </div>
    </div>
  `;

  document.getElementById('play-btn').addEventListener('click', async () => {
    await initAudio();
    const playing = seq.toggle();
    const btn = document.getElementById('play-btn');
    btn.textContent = playing ? '■' : '▶';
    btn.classList.toggle('active', playing);
  });

  document.getElementById('tempo-slider').addEventListener('input', (e) => {
    seq.bpm = parseInt(e.target.value);
    document.getElementById('tempo-val').textContent = e.target.value;
  });

  document.getElementById('swing-slider').addEventListener('input', (e) => {
    seq.swing = parseInt(e.target.value) / 100;
    document.getElementById('swing-val').textContent = e.target.value + '%';
  });

  document.getElementById('clear-btn').addEventListener('click', () => {
    seq.clearPattern();
    renderStepGrid();
  });

  document.getElementById('pattern-select').addEventListener('change', async (e) => {
    await initAudio();
    const val = e.target.value;
    if (val.startsWith('factory-') && val !== 'factory-none') {
      loadFactoryPattern(parseInt(val.split('-')[1]));
    }
  });
}

function loadFactoryPattern(idx) {
  const p = FACTORY_PATTERNS[idx];
  seq.clearPattern();
  seq.bpm = p.bpm;
  document.getElementById('tempo-slider').value = p.bpm;
  document.getElementById('tempo-val').textContent = p.bpm;
  for (const [voiceStr, steps] of Object.entries(p.data)) {
    const voice = parseInt(voiceStr);
    for (let s = 0; s < steps.length; s++) seq.setStep(voice, s, !!steps[s]);
  }
  renderStepGrid();
}

function buildTabBar() {
  const bar = document.getElementById('tab-bar');
  bar.innerHTML = '';
  const tabs = [
    { id: 'drum', label: 'DRUMS' },
    { id: 'deep', label: 'DEEP EDIT' },
    { id: 'bass', label: 'BD BASS' },
    { id: 'fx',   label: 'FX' },
  ];
  for (const tab of tabs) {
    const btn = document.createElement('button');
    btn.className = 'tab-btn' + (tab.id === currentTab ? ' selected' : '');
    btn.textContent = tab.label;
    btn.addEventListener('click', () => setTab(tab.id));
    bar.appendChild(btn);
  }
}

function setTab(tab) {
  currentTab = tab;
  document.querySelectorAll('.tab-btn').forEach((b, i) => {
    b.classList.toggle('selected', ['drum','deep','bass','fx'][i] === tab);
  });
  document.getElementById('instrument-bar').style.display = (tab === 'fx' || tab === 'bass') ? 'none' : 'flex';
  buildParamsPanel();
  if (tab === 'bass') buildBassGrid(); else renderStepGrid();
}

function buildInstrumentBar() {
  const bar = document.getElementById('instrument-bar');
  bar.innerHTML = '';
  VOICES.forEach((v, i) => {
    const btn = document.createElement('button');
    btn.className = 'inst-btn' + (i === selectedVoice ? ' selected' : '');
    btn.textContent = v.name;
    btn.dataset.index = i;
    btn.addEventListener('click', async () => {
      await initAudio();
      selectVoice(i);
      engine.trigger(i);
    });
    bar.appendChild(btn);
  });
}

function selectVoice(index) {
  selectedVoice = index;
  document.querySelectorAll('.inst-btn').forEach((b, i) => {
    b.classList.toggle('selected', i === index);
  });
  buildParamsPanel();
  renderStepGrid();
}

function buildParamsPanel() {
  const panel = document.getElementById('params-panel');

  if (currentTab === 'fx') {
    buildFxPanel(panel);
    return;
  }
  if (currentTab === 'bass') {
    buildBassPanel(panel);
    return;
  }
  if (currentTab === 'deep') {
    buildDeepPanel(panel);
    return;
  }

  const v = VOICES[selectedVoice];
  const p = engine.params[v.id];

  let html = `<div class="params-title">${v.fullName}</div><div class="knobs-row">`;
  html += makeSlider('level', 'LEVEL', p.level, 'macro');
  if (v.tone)   html += makeSlider('tone',   'TONE',   p.tone,   'macro');
  if (v.decay)  html += makeSlider('decay',  'DECAY',  p.decay,  'macro');
  if (v.snappy) html += makeSlider('snappy', 'SNAPPY', p.snappy, 'macro');
  if (v.tune)   html += makeSlider('tune',   'TUNE',   p.tune,   'macro');
  html += '</div>';
  panel.innerHTML = html;

  panel.querySelectorAll('.knob-input[data-type="macro"]').forEach(input => {
    input.addEventListener('input', () => {
      engine.params[v.id][input.dataset.param] = parseInt(input.value) / 100;
      input.parentElement.querySelector('.knob-value').textContent = Math.round(parseInt(input.value));
    });
  });
}

function buildDeepPanel(panel) {
  const v = VOICES[selectedVoice];
  const deepDefs = DEEP_PARAMS[v.id];

  if (!deepDefs || deepDefs.length === 0) {
    panel.innerHTML = `<div class="params-title">${v.fullName} - DEEP EDIT</div>
      <div class="params-empty">No deep parameters for this voice</div>`;
    return;
  }

  const d = engine.deep[v.id];
  let html = `<div class="params-title">${v.fullName} - DEEP EDIT</div><div class="knobs-row deep-row">`;
  for (const dp of deepDefs) {
    const norm = (d[dp.id] - dp.min) / (dp.max - dp.min);
    html += makeSlider(dp.id, dp.label, norm, 'deep', dp.min, dp.max);
  }
  html += '</div>';
  panel.innerHTML = html;

  panel.querySelectorAll('.knob-input[data-type="deep"]').forEach(input => {
    input.addEventListener('input', () => {
      const min = parseFloat(input.dataset.min);
      const max = parseFloat(input.dataset.max);
      const val = min + (parseInt(input.value) / 100) * (max - min);
      engine.deep[v.id][input.dataset.param] = val;
      input.parentElement.querySelector('.knob-value').textContent =
        val >= 100 ? Math.round(val) : val.toFixed(1);
    });
  });
}

function buildBassPanel(panel) {
  const p = engine.bassParams;
  let html = `<div class="params-title">BD BASS</div><div class="knobs-row">`;
  html += makeSlider('level', 'LEVEL', p.level, 'bass');
  html += makeSlider('tone',  'TONE',  p.tone,  'bass');
  html += makeSlider('decay', 'DECAY', p.decay, 'bass');
  html += makeSlider('punch', 'PUNCH', p.punch, 'bass');
  html += makeSlider('drive', 'DRIVE', (p.drive - 1) / 9, 'bass');
  html += '</div>';
  panel.innerHTML = html;

  panel.querySelectorAll('.knob-input[data-type="bass"]').forEach(input => {
    input.addEventListener('input', () => {
      const param = input.dataset.param;
      let val = parseInt(input.value) / 100;
      if (param === 'drive') val = 1 + val * 9;
      engine.bassParams[param] = val;
      input.parentElement.querySelector('.knob-value').textContent =
        param === 'drive' ? val.toFixed(1) : Math.round(val * 100);
    });
  });
}

function buildFxPanel(panel) {
  let html = `<div class="params-title">FX</div>`;
  html += `<div class="fx-section"><div class="fx-label">REVERB</div><div class="knobs-row">`;
  html += makeSlider('reverbMix', 'SEND', engine.reverbMix, 'fx');
  html += `</div></div>`;
  html += `<div class="fx-section"><div class="fx-label">DELAY</div><div class="knobs-row">`;
  html += makeSlider('delayMix', 'SEND', engine.delayMix, 'fx');
  html += makeSlider('delayTime', 'TIME', (engine.delayNode ? engine.delayNode.delayTime.value / 2 : 0.19), 'fx');
  html += makeSlider('delayFeedback', 'FDBK', (engine.delayFeedback ? engine.delayFeedback.gain.value : 0.35), 'fx');
  html += `</div></div>`;
  html += `<div class="fx-section"><div class="fx-label">MASTER</div><div class="knobs-row">`;
  html += makeSlider('masterLevel', 'OUTPUT', engine.masterLevel, 'fx');
  html += makeSlider('masterDrive', 'DRIVE', (engine.masterDrive - 1) / 9, 'fx');
  html += makeSlider('accentLevel', 'ACCENT', (engine.accentLevel - 1) / 1, 'fx');
  html += `</div></div>`;

  panel.innerHTML = html;

  panel.querySelectorAll('.knob-input[data-type="fx"]').forEach(input => {
    input.addEventListener('input', () => {
      const param = input.dataset.param;
      const val = parseInt(input.value) / 100;
      switch (param) {
        case 'reverbMix':
          engine.reverbMix = val;
          if (engine.reverbSend) engine.reverbSend.gain.value = val;
          break;
        case 'delayMix':
          engine.delayMix = val;
          if (engine.delaySend) engine.delaySend.gain.value = val;
          break;
        case 'delayTime':
          if (engine.delayNode) engine.delayNode.delayTime.value = val * 2;
          break;
        case 'delayFeedback':
          if (engine.delayFeedback) engine.delayFeedback.gain.value = val;
          break;
        case 'masterLevel':
          engine.masterLevel = val;
          if (engine.masterGain) engine.masterGain.gain.value = val;
          break;
        case 'masterDrive':
          engine.masterDrive = 1 + val * 9;
          break;
        case 'accentLevel':
          engine.accentLevel = 1 + val;
          break;
      }
      input.parentElement.querySelector('.knob-value').textContent = Math.round(val * 100);
    });
  });
}

function makeSlider(param, label, value, type, min, max) {
  const displayVal = (type === 'deep' && min !== undefined)
    ? ((min + value * (max - min)) >= 100 ? Math.round(min + value * (max - min)) : (min + value * (max - min)).toFixed(1))
    : Math.round(value * 100);
  return `
    <div class="knob-group">
      <label class="knob-label">${label}</label>
      <input type="range" class="knob-input" data-param="${param}" data-type="${type}"
             ${min !== undefined ? `data-min="${min}" data-max="${max}"` : ''}
             min="0" max="100" value="${Math.round(value * 100)}" step="1">
      <span class="knob-value">${displayVal}</span>
    </div>
  `;
}

function buildStepGrid() {
  const grid = document.getElementById('step-grid');
  grid.innerHTML = '';

  const accentRow = document.createElement('div');
  accentRow.className = 'step-row accent-row';
  accentRow.innerHTML = '<span class="row-label">ACC</span>';
  for (let s = 0; s < 16; s++) {
    const btn = document.createElement('button');
    btn.className = 'step-btn accent-btn' + (seq.getAccent(s) ? ' on' : '');
    btn.dataset.step = s;
    if (s % 4 === 0) btn.classList.add('beat-start');
    btn.addEventListener('click', async () => {
      await initAudio();
      btn.classList.toggle('on', seq.toggleAccent(s));
    });
    accentRow.appendChild(btn);
  }
  grid.appendChild(accentRow);

  const stepRow = document.createElement('div');
  stepRow.className = 'step-row main-row';
  stepRow.innerHTML = `<span class="row-label">${VOICES[selectedVoice].name}</span>`;
  for (let s = 0; s < 16; s++) {
    const btn = document.createElement('button');
    btn.className = 'step-btn' + (seq.getStep(selectedVoice, s) ? ' on' : '');
    btn.dataset.step = s;
    btn.dataset.voice = selectedVoice;
    if (s % 4 === 0) btn.classList.add('beat-start');
    btn.addEventListener('click', async () => {
      await initAudio();
      const on = seq.toggleStep(selectedVoice, s);
      btn.classList.toggle('on', on);
      if (on) engine.trigger(selectedVoice);
    });
    stepRow.appendChild(btn);
  }
  grid.appendChild(stepRow);

  const numRow = document.createElement('div');
  numRow.className = 'step-numbers';
  numRow.innerHTML = '<span class="row-label"></span>';
  for (let s = 0; s < 16; s++) {
    const span = document.createElement('span');
    span.className = 'step-num';
    span.textContent = s + 1;
    numRow.appendChild(span);
  }
  grid.appendChild(numRow);
}

function buildBassGrid() {
  const grid = document.getElementById('step-grid');
  grid.innerHTML = '';

  const noteNames = [];
  for (let oct = 1; oct <= 3; oct++) {
    for (const n of BASS_NOTES) noteNames.push(n + oct);
  }

  const pianoRow = document.createElement('div');
  pianoRow.className = 'bass-grid';

  const header = document.createElement('div');
  header.className = 'step-row bass-header';
  header.innerHTML = '<span class="row-label">BASS</span>';
  for (let s = 0; s < 16; s++) {
    const span = document.createElement('span');
    span.className = 'step-num';
    span.textContent = s + 1;
    header.appendChild(span);
  }
  grid.appendChild(header);

  const visibleNotes = noteNames.slice(0, 24);
  for (let n = visibleNotes.length - 1; n >= 0; n--) {
    const row = document.createElement('div');
    row.className = 'step-row bass-row';
    const label = document.createElement('span');
    label.className = 'row-label bass-note-label';
    label.textContent = visibleNotes[n];
    if (visibleNotes[n].includes('#')) label.classList.add('sharp');
    row.appendChild(label);

    const midiNote = 36 + n;
    for (let s = 0; s < 16; s++) {
      const btn = document.createElement('button');
      btn.className = 'step-btn bass-step';
      const currentNote = seq.getBassNote(s);
      if (currentNote === midiNote) btn.classList.add('on');
      if (s % 4 === 0) btn.classList.add('beat-start');
      btn.addEventListener('click', async () => {
        await initAudio();
        const cur = seq.getBassNote(s);
        if (cur === midiNote) {
          seq.setBassNote(s, -1);
          btn.classList.remove('on');
        } else {
          seq.setBassNote(s, midiNote);
          engine.triggerBass(midiNote);
          buildBassGrid();
        }
      });
      row.appendChild(btn);
    }
    grid.appendChild(row);
  }
}

function renderStepGrid() {
  if (currentTab === 'bass') { buildBassGrid(); return; }
  buildStepGrid();
}

function updatePlayhead(step) {
  document.querySelectorAll('.step-btn').forEach(btn => btn.classList.remove('playing'));
  if (step >= 0) {
    document.querySelectorAll(`.step-btn[data-step="${step}"]`).forEach(btn => {
      btn.classList.add('playing');
    });
    if (currentTab === 'bass') {
      document.querySelectorAll('.bass-step').forEach(btn => btn.classList.remove('playing'));
      const col = step;
      document.querySelectorAll(`.bass-step:nth-child(${col + 2})`).forEach(btn => {
        btn.classList.add('playing');
      });
    }
  }
}

document.addEventListener('DOMContentLoaded', buildUI);

export { engine, seq };
