import { AudioEngine, VOICES, DEEP_PARAMS } from './audio-engine.js';
import { Sequencer, NUM_VOICES, NUM_PATTERNS, NUM_VARS } from './sequencer.js';

const engine = new AudioEngine();
const seq = new Sequencer(engine);

let selectedVoice = 0;
let audioInitialized = false;
let currentTab = 'drum';

const FACTORY_PATTERNS = [
  {
    name: 'Classic 808', bpm: 120,
    data: { 0:[1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0], 2:[0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0],
            11:[1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0], 10:[0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0] }
  },
  {
    name: 'Boom Bap', bpm: 90,
    data: { 0:[1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0], 2:[0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0],
            11:[1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0], 10:[0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0] }
  },
  {
    name: 'Trap', bpm: 140,
    data: { 0:[1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0], 2:[0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0],
            11:[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1], 10:[0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0],
            7:[0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0] }
  },
  {
    name: 'Electro', bpm: 130,
    data: { 0:[1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0], 2:[0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1],
            3:[0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0], 11:[1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0],
            8:[0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,0] }
  },
  {
    name: 'Latin', bpm: 110,
    data: { 0:[1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0], 1:[0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0],
            12:[0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0], 13:[1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0],
            15:[1,0,0,1,0,0,1,0,0,1,0,0,1,0,0,0], 11:[1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0] }
  },
];

async function initAudio() {
  await engine.init();
  if (!audioInitialized) {
    audioInitialized = true;
    document.getElementById('init-overlay').classList.add('hidden');
  }
}

function buildUI() {
  buildHeader();
  buildSeqControls();
  buildTabBar();
  buildInstrumentBar();
  buildParamsPanel();
  buildStepGrid();
  buildInitOverlay();
  seq.onStep = (step) => updatePlayhead(step);
}

function buildInitOverlay() {
  const overlay = document.createElement('div');
  overlay.id = 'init-overlay';
  overlay.innerHTML = `
    <div class="init-content">
      <div class="init-logo">TR-808</div>
      <div class="init-sub">RHYTHM COMPOSER</div>
      <div class="init-hint">Make sure silent mode is OFF</div>
      <button class="init-btn" id="start-btn">TAP TO START</button>
    </div>`;
  document.body.appendChild(overlay);
  const handler = async (e) => {
    e.preventDefault();
    e.stopPropagation();
    await initAudio();
    const state = engine.getAudioState();
    if (state !== 'running') {
      document.querySelector('.init-hint').textContent = 'Audio: ' + state + ' — tap again';
    }
  };
  document.getElementById('start-btn').addEventListener('click', handler);
  document.getElementById('start-btn').addEventListener('touchend', handler);
}

function buildHeader() {
  const header = document.getElementById('header');
  header.innerHTML = `
    <div class="header-row">
      <button class="transport-btn" id="play-btn">▶</button>
      <div class="compact-group">
        <label>BPM</label>
        <input type="range" id="tempo-slider" min="40" max="300" value="120" step="1">
        <span id="tempo-val">120</span>
      </div>
      <div class="compact-group">
        <label>SWING</label>
        <input type="range" id="swing-slider" min="0" max="75" value="0" step="1">
        <span id="swing-val">0%</span>
      </div>
      <select id="pattern-select" class="header-select">
        <option value="">PRESET</option>
        ${FACTORY_PATTERNS.map((p, i) => `<option value="${i}">${p.name}</option>`).join('')}
      </select>
      <button class="clear-btn" id="clear-btn">CLR</button>
    </div>`;

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
    seq.clearPattern(); renderStepGrid();
  });
  document.getElementById('pattern-select').addEventListener('change', async (e) => {
    if (e.target.value === '') return;
    await initAudio();
    loadFactoryPattern(parseInt(e.target.value));
    e.target.value = '';
  });
}

function buildSeqControls() {
  const bar = document.getElementById('seq-controls');
  bar.innerHTML = `
    <div class="seq-row">
      <div class="seq-group">
        <label class="seq-label">PAT</label>
        ${Array.from({length: 8}, (_, i) =>
          `<button class="seq-btn pat-btn${i === seq.currentPattern ? ' selected' : ''}" data-pat="${i}">${i+1}</button>`
        ).join('')}
      </div>
      <div class="seq-group">
        <label class="seq-label">VAR</label>
        ${['A','B','C','D'].map((l, i) =>
          `<button class="seq-btn var-btn${i === seq.currentVar ? ' selected' : ''}" data-var="${i}">${l}</button>`
        ).join('')}
        <select class="seq-select" id="play-mode">
          <option value="a">A</option><option value="b">B</option>
          <option value="c">C</option><option value="d">D</option>
          <option value="cycle">CYCLE</option>
        </select>
        <select class="seq-select" id="copy-var"><option value="">COPY</option>
          <option value="0">→A</option><option value="1">→B</option>
          <option value="2">→C</option><option value="3">→D</option>
        </select>
      </div>
      <div class="seq-group">
        <select class="seq-select" id="step-len">
          <option value="16">16</option><option value="32">32</option><option value="8">8</option>
        </select>
        <select class="seq-select" id="time-sig">
          <option value="4">4/4</option><option value="3">3/4</option>
        </select>
        <button class="seq-btn" id="trip-btn">1/16</button>
      </div>
      <div class="seq-group">
        <button class="seq-btn" id="song-btn">SONG</button>
        <input class="chain-input" id="chain-input" placeholder="1 2 1 3" type="text">
      </div>
    </div>`;

  bar.querySelectorAll('.pat-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      seq.currentPattern = parseInt(btn.dataset.pat);
      syncSeqUI(); renderStepGrid();
    });
  });
  bar.querySelectorAll('.var-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      seq.currentVar = parseInt(btn.dataset.var);
      syncSeqUI(); renderStepGrid();
    });
  });
  document.getElementById('play-mode').addEventListener('change', (e) => {
    seq.playMode = e.target.value;
  });
  document.getElementById('copy-var').addEventListener('change', (e) => {
    if (e.target.value === '') return;
    seq.copyVariation(seq.currentVar, parseInt(e.target.value));
    e.target.value = ''; renderStepGrid();
  });
  document.getElementById('step-len').addEventListener('change', (e) => {
    seq.steps = parseInt(e.target.value); renderStepGrid();
  });
  document.getElementById('time-sig').addEventListener('change', (e) => {
    const g = parseInt(e.target.value);
    seq.steps = g === 3 ? 12 : 16; renderStepGrid();
  });
  document.getElementById('trip-btn').addEventListener('click', () => {
    seq.triplet = !seq.triplet;
    document.getElementById('trip-btn').textContent = seq.triplet ? '1/16T' : '1/16';
  });
  document.getElementById('song-btn').addEventListener('click', () => {
    seq.chainEnabled = !seq.chainEnabled;
    document.getElementById('song-btn').classList.toggle('active', seq.chainEnabled);
  });
  const chainInput = document.getElementById('chain-input');
  const applyChain = () => {
    seq.chain = chainInput.value.split(/[\s,]+/).filter(s => s).map(s => parseInt(s) - 1).filter(n => n >= 0 && n < NUM_PATTERNS);
  };
  chainInput.addEventListener('change', applyChain);
  chainInput.addEventListener('blur', applyChain);
}

function syncSeqUI() {
  document.querySelectorAll('.pat-btn').forEach(b => b.classList.toggle('selected', parseInt(b.dataset.pat) === seq.currentPattern));
  document.querySelectorAll('.var-btn').forEach(b => b.classList.toggle('selected', parseInt(b.dataset.var) === seq.currentVar));
  const len = seq.steps;
  document.getElementById('step-len').value = [8,16,32].includes(len) ? len : 16;
  document.getElementById('trip-btn').textContent = seq.triplet ? '1/16T' : '1/16';
}

function loadFactoryPattern(idx) {
  const p = FACTORY_PATTERNS[idx];
  seq.clearPattern(); seq.bpm = p.bpm;
  document.getElementById('tempo-slider').value = p.bpm;
  document.getElementById('tempo-val').textContent = p.bpm;
  for (const [v, steps] of Object.entries(p.data)) {
    for (let s = 0; s < steps.length; s++) seq.setStep(parseInt(v), s, !!steps[s]);
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
    if (seq.getMute(i)) btn.classList.add('muted');
    if (seq.getSolo(i)) btn.classList.add('soloed');
    btn.textContent = v.name;
    btn.dataset.index = i;
    btn.addEventListener('click', async () => {
      await initAudio(); selectVoice(i); engine.trigger(i);
    });
    bar.appendChild(btn);
  });
}

function selectVoice(index) {
  selectedVoice = index;
  document.querySelectorAll('.inst-btn').forEach((b, i) => b.classList.toggle('selected', i === index));
  buildParamsPanel(); renderStepGrid();
}

function buildParamsPanel() {
  const panel = document.getElementById('params-panel');
  switch (currentTab) {
    case 'fx':   buildFxPanel(panel); return;
    case 'bass': buildBassPanel(panel); return;
    case 'deep': buildDeepPanel(panel); return;
  }

  const v = VOICES[selectedVoice];
  const p = engine.params[v.id];
  let html = `<div class="params-header">
    <span class="params-title">${v.fullName}</span>
    <div class="ms-btns">
      <button class="ms-btn mute-btn${seq.getMute(selectedVoice) ? ' active' : ''}" id="mute-btn">M</button>
      <button class="ms-btn solo-btn${seq.getSolo(selectedVoice) ? ' active' : ''}" id="solo-btn">S</button>
    </div>
  </div><div class="knobs-row">`;
  html += makeKnob('level', 'LEVEL', p.level, 'macro');
  if (v.tone)   html += makeKnob('tone',   'TONE',   p.tone,   'macro');
  if (v.decay)  html += makeKnob('decay',  'DECAY',  p.decay,  'macro');
  if (v.snappy) html += makeKnob('snappy', 'SNAPPY', p.snappy, 'macro');
  if (v.tune)   html += makeKnob('tune',   'TUNE',   p.tune,   'macro');
  html += makeKnob('pan', 'PAN', (engine.getPan(v.id) + 1) / 2, 'pan');
  html += makeKnob('revsend', 'RVB', engine.getRevSend(v.id), 'send');
  html += makeKnob('dlysend', 'DLY', engine.getDlySend(v.id), 'send');
  html += '</div>';
  panel.innerHTML = html;
  initKnobs(panel);

  panel.querySelectorAll('.knob-canvas[data-type="macro"]').forEach(k => {
    k.addEventListener('knobchange', () => {
      engine.params[v.id][k.dataset.param] = parseInt(k.dataset.value) / 100;
      k.parentElement.querySelector('.knob-value').textContent = Math.round(parseInt(k.dataset.value));
    });
  });
  panel.querySelectorAll('.knob-canvas[data-type="pan"]').forEach(k => {
    k.addEventListener('knobchange', () => {
      const val = (parseInt(k.dataset.value) / 100) * 2 - 1;
      engine.setPan(v.id, val);
      k.parentElement.querySelector('.knob-value').textContent = val === 0 ? 'C' : (val < 0 ? `L${Math.round(-val*100)}` : `R${Math.round(val*100)}`);
    });
  });
  panel.querySelectorAll('.knob-canvas[data-type="send"]').forEach(k => {
    k.addEventListener('knobchange', () => {
      const val = parseInt(k.dataset.value) / 100;
      if (k.dataset.param === 'revsend') engine.setRevSend(v.id, val);
      else engine.setDlySend(v.id, val);
      k.parentElement.querySelector('.knob-value').textContent = Math.round(val * 100);
    });
  });

  document.getElementById('mute-btn').addEventListener('click', () => {
    const m = seq.toggleMute(selectedVoice);
    document.getElementById('mute-btn').classList.toggle('active', m);
    buildInstrumentBar();
  });
  document.getElementById('solo-btn').addEventListener('click', () => {
    const s = seq.toggleSolo(selectedVoice);
    document.getElementById('solo-btn').classList.toggle('active', s);
    buildInstrumentBar();
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
    html += makeKnob(dp.id, dp.label, norm, 'deep', dp.min, dp.max);
  }
  html += '</div>';
  panel.innerHTML = html;
  initKnobs(panel);
  panel.querySelectorAll('.knob-canvas[data-type="deep"]').forEach(k => {
    k.addEventListener('knobchange', () => {
      const min = parseFloat(k.dataset.min), max = parseFloat(k.dataset.max);
      const val = min + (parseInt(k.dataset.value) / 100) * (max - min);
      engine.deep[v.id][k.dataset.param] = val;
      k.parentElement.querySelector('.knob-value').textContent = val >= 100 ? Math.round(val) : val.toFixed(1);
    });
  });
}

function buildBassPanel(panel) {
  const p = engine.bassParams;
  let html = `<div class="params-title">BD BASS</div><div class="knobs-row">`;
  html += makeKnob('level', 'LEVEL', p.level, 'bass');
  html += makeKnob('tone',  'TONE',  p.tone,  'bass');
  html += makeKnob('decay', 'DECAY', p.decay, 'bass');
  html += makeKnob('punch', 'PUNCH', p.punch, 'bass');
  html += makeKnob('drive', 'DRIVE', (p.drive - 1) / 9, 'bass');
  html += '</div>';
  panel.innerHTML = html;
  initKnobs(panel);
  panel.querySelectorAll('.knob-canvas[data-type="bass"]').forEach(k => {
    k.addEventListener('knobchange', () => {
      let val = parseInt(k.dataset.value) / 100;
      if (k.dataset.param === 'drive') val = 1 + val * 9;
      engine.bassParams[k.dataset.param] = val;
      k.parentElement.querySelector('.knob-value').textContent =
        k.dataset.param === 'drive' ? val.toFixed(1) : Math.round(val * 100);
    });
  });
}

function buildFxPanel(panel) {
  let html = `<div class="params-title">FX</div>`;
  html += `<div class="fx-section"><div class="fx-label">REVERB</div><div class="knobs-row">`;
  html += makeKnob('reverbMix', 'RETURN', engine.reverbReturn ? engine.reverbReturn.gain.value : 0.35, 'fx');
  html += `</div></div>`;
  html += `<div class="fx-section"><div class="fx-label">DELAY</div><div class="knobs-row">`;
  html += makeKnob('delayMix', 'RETURN', engine.delayReturn ? engine.delayReturn.gain.value : 0.5, 'fx');
  html += makeKnob('delayTime', 'TIME', engine.delayNode ? engine.delayNode.delayTime.value / 2 : 0.19, 'fx');
  html += makeKnob('delayFeedback', 'FDBK', engine.delayFeedback ? engine.delayFeedback.gain.value : 0.35, 'fx');
  html += `</div></div>`;
  html += `<div class="fx-section"><div class="fx-label">MASTER</div><div class="knobs-row">`;
  html += makeKnob('masterLevel', 'OUTPUT', engine.masterLevel, 'fx');
  html += makeKnob('masterDrive', 'DRIVE', (engine.masterDrive - 1) / 9, 'fx');
  html += makeKnob('accentLevel', 'ACCENT', (engine.accentLevel - 1), 'fx');
  html += `</div></div>`;
  panel.innerHTML = html;
  initKnobs(panel);
  panel.querySelectorAll('.knob-canvas[data-type="fx"]').forEach(k => {
    k.addEventListener('knobchange', () => {
      const val = parseInt(k.dataset.value) / 100;
      switch (k.dataset.param) {
        case 'reverbMix': if (engine.reverbReturn) engine.reverbReturn.gain.value = val; break;
        case 'delayMix': if (engine.delayReturn) engine.delayReturn.gain.value = val; break;
        case 'delayTime': if (engine.delayNode) engine.delayNode.delayTime.value = val * 2; break;
        case 'delayFeedback': if (engine.delayFeedback) engine.delayFeedback.gain.value = val; break;
        case 'masterLevel': engine.masterLevel = val; if (engine.masterGain) engine.masterGain.gain.value = val; break;
        case 'masterDrive': engine.masterDrive = 1 + val * 9; break;
        case 'accentLevel': engine.accentLevel = 1 + val; break;
      }
      k.parentElement.querySelector('.knob-value').textContent = Math.round(val * 100);
    });
  });
}

function makeKnob(param, label, value, type, min, max) {
  const v100 = Math.round(Math.max(0, Math.min(1, value)) * 100);
  let displayVal;
  if (type === 'deep' && min !== undefined) {
    const raw = min + value * (max - min);
    displayVal = raw >= 100 ? Math.round(raw) : raw.toFixed(1);
  } else if (type === 'pan') {
    const pan = value * 2 - 1;
    displayVal = pan === 0 ? 'C' : (pan < 0 ? `L${Math.round(-pan*100)}` : `R${Math.round(pan*100)}`);
  } else {
    displayVal = v100;
  }
  return `<div class="knob-group">
    <label class="knob-label">${label}</label>
    <canvas class="knob-canvas" data-param="${param}" data-type="${type}" data-value="${v100}"
            ${min !== undefined ? `data-min="${min}" data-max="${max}"` : ''}
            width="88" height="88"></canvas>
    <span class="knob-value">${displayVal}</span>
  </div>`;
}

const KNOB_START = Math.PI * 0.75;
const KNOB_END = Math.PI * 2.25;
const KNOB_RANGE = KNOB_END - KNOB_START;

function drawKnob(canvas, norm) {
  const ctx = canvas.getContext('2d');
  const dpr = window.devicePixelRatio || 1;
  const w = canvas.width / dpr;
  const h = canvas.height / dpr;
  const cx = w / 2, cy = h / 2;
  const r = Math.min(cx, cy) - 4;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.save();
  ctx.scale(dpr, dpr);

  ctx.beginPath();
  ctx.arc(cx, cy, r, KNOB_START, KNOB_END);
  ctx.strokeStyle = '#444';
  ctx.lineWidth = 4;
  ctx.lineCap = 'round';
  ctx.stroke();

  if (norm > 0.005) {
    const angle = KNOB_START + norm * KNOB_RANGE;
    ctx.beginPath();
    ctx.arc(cx, cy, r, KNOB_START, angle);
    ctx.strokeStyle = '#e87830';
    ctx.lineWidth = 4;
    ctx.lineCap = 'round';
    ctx.stroke();
  }

  const angle = KNOB_START + norm * KNOB_RANGE;
  const ix = cx + Math.cos(angle) * (r - 8);
  const iy = cy + Math.sin(angle) * (r - 8);
  ctx.beginPath();
  ctx.arc(cx, cy, r * 0.35, 0, Math.PI * 2);
  ctx.fillStyle = '#3a3a3a';
  ctx.fill();
  ctx.beginPath();
  ctx.moveTo(cx, cy);
  ctx.lineTo(ix, iy);
  ctx.strokeStyle = '#f0e0c8';
  ctx.lineWidth = 2.5;
  ctx.lineCap = 'round';
  ctx.stroke();

  ctx.restore();
}

function initKnobs(container) {
  container.querySelectorAll('.knob-canvas').forEach(canvas => {
    const dpr = window.devicePixelRatio || 1;
    const size = 34;
    canvas.style.width = size + 'px';
    canvas.style.height = size + 'px';
    canvas.width = size * dpr;
    canvas.height = size * dpr;
    const norm = parseInt(canvas.dataset.value) / 100;
    drawKnob(canvas, norm);

    let startY = 0, startVal = 0, active = false;

    const onStart = (e) => {
      e.preventDefault();
      active = true;
      const pt = e.touches ? e.touches[0] : e;
      startY = pt.clientY;
      startVal = parseInt(canvas.dataset.value);
    };
    const onMove = (e) => {
      if (!active) return;
      e.preventDefault();
      const pt = e.touches ? e.touches[0] : e;
      const dy = startY - pt.clientY;
      const newVal = Math.max(0, Math.min(100, startVal + dy * 0.7));
      canvas.dataset.value = Math.round(newVal);
      drawKnob(canvas, newVal / 100);
      canvas.dispatchEvent(new Event('knobchange'));
    };
    const onEnd = () => { active = false; };

    canvas.addEventListener('mousedown', onStart);
    canvas.addEventListener('touchstart', onStart, { passive: false });
    window.addEventListener('mousemove', onMove);
    window.addEventListener('touchmove', onMove, { passive: false });
    window.addEventListener('mouseup', onEnd);
    window.addEventListener('touchend', onEnd);
  });
}

function buildStepGrid() {
  const grid = document.getElementById('step-grid');
  grid.innerHTML = '';
  const len = seq.steps;

  const accentRow = document.createElement('div');
  accentRow.className = 'step-row accent-row';
  accentRow.innerHTML = '<span class="row-label">ACC</span>';
  for (let s = 0; s < len; s++) {
    const btn = document.createElement('button');
    btn.className = 'step-btn accent-btn' + (seq.getAccent(s) ? ' on' : '');
    btn.dataset.step = s;
    if (s % 4 === 0) btn.classList.add('beat-start');
    btn.addEventListener('click', async () => {
      await initAudio(); btn.classList.toggle('on', seq.toggleAccent(s));
    });
    accentRow.appendChild(btn);
  }
  grid.appendChild(accentRow);

  const stepRow = document.createElement('div');
  stepRow.className = 'step-row main-row';
  stepRow.innerHTML = `<span class="row-label">${VOICES[selectedVoice].name}</span>`;
  for (let s = 0; s < len; s++) {
    const btn = document.createElement('button');
    btn.className = 'step-btn' + (seq.getStep(selectedVoice, s) ? ' on' : '');
    btn.dataset.step = s; btn.dataset.voice = selectedVoice;
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
  for (let s = 0; s < len; s++) {
    const span = document.createElement('span');
    span.className = 'step-num';
    span.textContent = s + 1;
    numRow.appendChild(span);
  }
  grid.appendChild(numRow);

  const fillRow = document.createElement('div');
  fillRow.className = 'fill-row';
  fillRow.innerHTML = '<span class="fill-label">FILL</span>';
  for (const n of [1, 2, 3, 4]) {
    const btn = document.createElement('button');
    btn.className = 'fill-btn';
    btn.textContent = '1/' + n;
    btn.addEventListener('click', async () => {
      await initAudio();
      for (let s = 0; s < len; s++) seq.setStep(selectedVoice, s, s % n === 0);
      buildStepGrid();
    });
    fillRow.appendChild(btn);
  }
  const clrBtn = document.createElement('button');
  clrBtn.className = 'fill-btn fill-clr';
  clrBtn.textContent = 'CLR';
  clrBtn.addEventListener('click', () => {
    for (let s = 0; s < len; s++) seq.setStep(selectedVoice, s, false);
    buildStepGrid();
  });
  fillRow.appendChild(clrBtn);
  grid.appendChild(fillRow);
}

function buildBassGrid() {
  const grid = document.getElementById('step-grid');
  grid.innerHTML = '';
  const len = seq.steps;
  const notes = [];
  for (let oct = 1; oct <= 3; oct++)
    for (const n of ['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'])
      notes.push({ name: n + oct, midi: 24 + (oct - 1) * 12 + ['C','C#','D','D#','E','F','F#','G','G#','A','A#','B'].indexOf(n) });

  const visible = notes.slice(0, 24);
  const header = document.createElement('div');
  header.className = 'step-row bass-header';
  header.innerHTML = '<span class="row-label bass-note-label">BASS</span>';
  for (let s = 0; s < len; s++) {
    const span = document.createElement('span');
    span.className = 'step-num';
    span.textContent = s + 1;
    header.appendChild(span);
  }
  grid.appendChild(header);

  for (let n = visible.length - 1; n >= 0; n--) {
    const row = document.createElement('div');
    row.className = 'step-row bass-row';
    const label = document.createElement('span');
    label.className = 'row-label bass-note-label';
    label.textContent = visible[n].name;
    if (visible[n].name.includes('#')) label.classList.add('sharp');
    row.appendChild(label);

    const midi = visible[n].midi;
    for (let s = 0; s < len; s++) {
      const btn = document.createElement('button');
      btn.className = 'step-btn bass-step';
      if (seq.getBassNote(s) === midi) btn.classList.add('on');
      if (s % 4 === 0) btn.classList.add('beat-start');
      btn.addEventListener('click', async () => {
        await initAudio();
        if (seq.getBassNote(s) === midi) { seq.setBassNote(s, -1); }
        else { seq.setBassNote(s, midi); engine.triggerBass(midi); }
        buildBassGrid();
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
    document.querySelectorAll(`.step-btn[data-step="${step}"]`).forEach(btn => btn.classList.add('playing'));
  }
}

document.addEventListener('DOMContentLoaded', buildUI);
export { engine, seq };
