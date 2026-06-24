import { AudioEngine, VOICES } from './audio-engine.js';
import { Sequencer, NUM_VOICES } from './sequencer.js';

const engine = new AudioEngine();
const seq = new Sequencer(engine);

let selectedVoice = 0;
let audioInitialized = false;

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
];

async function initAudio() {
  if (audioInitialized) return;
  await engine.init();
  audioInitialized = true;
  document.getElementById('init-overlay').classList.add('hidden');
}

function buildUI() {
  buildHeader();
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
  document.getElementById('start-btn').addEventListener('click', initAudio);
  document.getElementById('start-btn').addEventListener('touchend', (e) => {
    e.preventDefault();
    initAudio();
  });
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

  const tempoSlider = document.getElementById('tempo-slider');
  tempoSlider.addEventListener('input', () => {
    seq.bpm = parseInt(tempoSlider.value);
    document.getElementById('tempo-val').textContent = tempoSlider.value;
  });

  const swingSlider = document.getElementById('swing-slider');
  swingSlider.addEventListener('input', () => {
    seq.swing = parseInt(swingSlider.value) / 100;
    document.getElementById('swing-val').textContent = swingSlider.value + '%';
  });

  document.getElementById('clear-btn').addEventListener('click', () => {
    seq.clearPattern();
    renderStepGrid();
  });

  document.getElementById('pattern-select').addEventListener('change', async (e) => {
    await initAudio();
    const val = e.target.value;
    if (val.startsWith('factory-') && val !== 'factory-none') {
      const idx = parseInt(val.split('-')[1]);
      loadFactoryPattern(idx);
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
    for (let s = 0; s < steps.length; s++) {
      seq.setStep(voice, s, !!steps[s]);
    }
  }
  renderStepGrid();
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
  const v = VOICES[selectedVoice];
  const p = engine.params[v.id];

  let html = `<div class="params-title">${v.fullName}</div><div class="knobs-row">`;

  html += makeKnob('level', 'LEVEL', p.level);
  if (v.tone)   html += makeKnob('tone',   'TONE',   p.tone);
  if (v.decay)  html += makeKnob('decay',  'DECAY',  p.decay);
  if (v.snappy) html += makeKnob('snappy', 'SNAPPY', p.snappy);
  if (v.tune)   html += makeKnob('tune',   'TUNE',   p.tune);

  html += '</div>';
  panel.innerHTML = html;

  panel.querySelectorAll('.knob-input').forEach(input => {
    input.addEventListener('input', () => {
      const param = input.dataset.param;
      const val = parseInt(input.value) / 100;
      engine.params[v.id][param] = val;
      input.parentElement.querySelector('.knob-value').textContent = Math.round(val * 100);
    });
  });
}

function makeKnob(param, label, value) {
  return `
    <div class="knob-group">
      <label class="knob-label">${label}</label>
      <input type="range" class="knob-input" data-param="${param}"
             min="0" max="100" value="${Math.round(value * 100)}" step="1">
      <span class="knob-value">${Math.round(value * 100)}</span>
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
      const on = seq.toggleAccent(s);
      btn.classList.toggle('on', on);
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

function renderStepGrid() {
  buildStepGrid();
}

function updatePlayhead(step) {
  document.querySelectorAll('.step-btn').forEach(btn => {
    btn.classList.remove('playing');
  });

  if (step >= 0) {
    document.querySelectorAll(`.step-btn[data-step="${step}"]`).forEach(btn => {
      btn.classList.add('playing');
    });
  }
}

document.addEventListener('DOMContentLoaded', buildUI);

export { engine, seq };
