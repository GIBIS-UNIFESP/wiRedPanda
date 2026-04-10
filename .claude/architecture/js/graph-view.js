// Main architecture graph — SVG + manual compact layout

// ── Color helpers ────────────────────────────────────────────
function hexAlpha(hex, alpha) {
  const r = parseInt(hex.slice(1, 3), 16);
  const g = parseInt(hex.slice(3, 5), 16);
  const b = parseInt(hex.slice(5, 7), 16);
  return 'rgba(' + r + ',' + g + ',' + b + ',' + (alpha / 255).toFixed(2) + ')';
}

// ── Build data lookups ───────────────────────────────────────
const nodeMap = {};       // id → node data
const edgeList = [];      // {s, t, label, type, index}
const moduleNodes = {};   // mod → [nodeId, ...]
const nodePositions = {}; // id → {x, y}
const nodeSizes = {};     // id → {w, h}
const modulePositions = {}; // mod → {x, y, w, h}

for (const n of nodes) {
  n.hasDrill = !!flowIdForNode(n.id);
  nodeMap[n.id] = n;
  if (!moduleNodes[n.module]) moduleNodes[n.module] = [];
  moduleNodes[n.module].push(n.id);
}
for (let i = 0; i < edges.length; i++) {
  edgeList.push({ ...edges[i], index: i });
}

// ── SVG element references ───────────────────────────────────
const svgNodeEls = {};    // id → <g> element
const svgEdgeEls = {};    // index → {path, label}
const svgModuleEls = {};  // mod → <g> element

// ── Create SVG canvas ────────────────────────────────────────
const canvas = createSvgCanvas(document.getElementById('cy'));

// Edge type colors
const edgeColors = {
  ownership: 'rgba(63, 185, 80, 0.53)',
  element: 'rgba(218, 54, 51, 0.53)',
  wiring: 'rgba(163, 113, 247, 0.53)',
  simulation: 'rgba(210, 153, 34, 0.53)',
  io: 'rgba(121, 192, 255, 0.53)',
  ui: 'rgba(247, 120, 186, 0.53)',
  bwd: 'rgba(126, 231, 135, 0.53)',
  codegen: 'rgba(255, 166, 87, 0.53)',
  core: 'rgba(88, 166, 255, 0.53)',
  component: 'rgba(240, 136, 62, 0.53)',
  test: 'rgba(139, 148, 158, 0.4)',
  external: 'rgba(72, 79, 88, 0.4)',
  inherit: 'rgba(218, 54, 51, 0.4)',
};
const dashedTypes = new Set(['component', 'inherit']);
const dottedTypes = new Set(['test', 'external']);

// Key nodes get thicker borders
const keyNodes = new Set(['graphic_elem', 'scene', 'simulation', 'mainwindow', 'main']);

// ── Arrow markers ────────────────────────────────────────────
const markerIds = {};
for (const [type, color] of Object.entries(edgeColors)) {
  const mid = 'arrow-' + type;
  // Extract solid color for arrow from rgba
  const solidColor = color.replace(/[\d.]+\)$/, '1)');
  addArrowMarker(canvas.defs, mid, solidColor);
  markerIds[type] = mid;
}
addArrowMarker(canvas.defs, 'arrow-default', '#30363d');

// ── Filter state ─────────────────────────────────────────────
const filterState = {};
for (const mod of Object.keys(modules)) filterState[mod] = true;

// ── Render functions ─────────────────────────────────────────

function renderGraph() {
  canvas.root.innerHTML = '';

  // Layer 0: edges (behind everything)
  const edgeLayer = svgEl('g', { class: 'edge-layer' });
  canvas.root.appendChild(edgeLayer);

  // Layer 1: modules
  const moduleLayer = svgEl('g', { class: 'module-layer' });
  canvas.root.appendChild(moduleLayer);

  // Layer 2: nodes (on top)
  const nodeLayer = svgEl('g', { class: 'node-layer' });
  canvas.root.appendChild(nodeLayer);

  // Render modules
  for (const [mod, info] of Object.entries(modules)) {
    const g = svgEl('g', { class: 'module', 'data-mod': mod });
    svgModuleEls[mod] = g;

    const rect = svgEl('rect', {
      rx: '8', ry: '8',
      fill: '#161b22', stroke: hexAlpha(info.color, 0x88), 'stroke-width': '2'
    });
    g.appendChild(rect);

    const label = svgEl('text', {
      fill: info.color, 'font-size': '11', 'font-weight': 'bold',
      'text-anchor': 'middle', 'dominant-baseline': 'hanging',
      'pointer-events': 'none'
    });
    label.textContent = info.label;
    g.appendChild(label);

    g._rect = rect;
    g._label = label;
    moduleLayer.appendChild(g);
  }

  // Render class nodes
  for (const n of nodes) {
    const fontSize = keyNodes.has(n.id) ? 11 : 10;
    const w = measureText(n.label, fontSize) + 14;
    const h = 24;
    nodeSizes[n.id] = { w, h };

    const g = svgEl('g', { class: 'class-node', 'data-id': n.id, cursor: 'pointer' });
    svgNodeEls[n.id] = g;

    const borderColor = hexAlpha(modules[n.module].color, 0x99);
    const borderWidth = keyNodes.has(n.id) ? 2.5 : 1.5;

    const rect = svgEl('rect', {
      width: w, height: h, rx: '4', ry: '4',
      fill: '#21262d', stroke: borderColor, 'stroke-width': borderWidth
    });
    g.appendChild(rect);

    const text = svgEl('text', {
      x: w / 2, y: h / 2,
      fill: '#e6edf3', 'font-size': fontSize,
      'font-weight': keyNodes.has(n.id) ? 'bold' : 'normal',
      'text-anchor': 'middle', 'dominant-baseline': 'central',
      'pointer-events': 'none'
    });
    text.textContent = n.label;
    g.appendChild(text);

    // Drillable indicator: small gold dot
    if (n.hasDrill) {
      const dot = svgEl('circle', {
        cx: w - 4, cy: 4, r: '3', fill: '#d29922', opacity: '0.7'
      });
      g.appendChild(dot);
    }

    g._rect = rect;
    nodeLayer.appendChild(g);

    // Click handler
    g.addEventListener('click', (e) => { e.stopPropagation(); highlightNode(n.id); });
    g.addEventListener('dblclick', (e) => {
      e.stopPropagation();
      const fid = flowIdForNode(n.id);
      if (fid) { flowHistory = []; openFlow(fid); }
    });
  }

  // Render edges
  for (const e of edgeList) {
    const color = edgeColors[e.type] || '#30363d';
    const mid = markerIds[e.type] || 'arrow-default';

    let dashArray = '';
    if (dashedTypes.has(e.type)) dashArray = '6,3';
    if (dottedTypes.has(e.type)) dashArray = '2,3';

    const path = svgEl('path', {
      fill: 'none', stroke: color, 'stroke-width': '1.2',
      'marker-end': `url(#${mid})`, opacity: '0.55'
    });
    if (dashArray) path.setAttribute('stroke-dasharray', dashArray);

    const labelEl = e.label ? createEdgeLabel(e.label) : null;

    svgEdgeEls[e.index] = { path, label: labelEl };
    edgeLayer.appendChild(path);
    if (labelEl) edgeLayer.appendChild(labelEl);
  }
}

function createEdgeLabel(text) {
  const g = svgEl('g', { 'pointer-events': 'none' });
  const tw = measureText(text, 8) + 4;
  const bg = svgEl('rect', {
    width: tw, height: 12, rx: '2', ry: '2',
    fill: '#0d1117', opacity: '0.9'
  });
  g.appendChild(bg);
  const t = svgEl('text', {
    x: tw / 2, y: 9, fill: '#484f58', 'font-size': '8',
    'text-anchor': 'middle', 'pointer-events': 'none'
  });
  t.textContent = text;
  g.appendChild(t);
  g._bg = bg;
  return g;
}

// ── Compact layout ───────────────────────────────────────────

function runCompactLayout() {
  const container = document.getElementById('cy');
  const cellW = 105;
  const cellH = 34;
  const modGap = 18;

  // Measure each module: grid its children
  const modSizes = {};
  const modChildList = {};
  for (const mod of Object.keys(modules)) {
    const children = (moduleNodes[mod] || []).filter(id => filterState[mod]);
    modChildList[mod] = children;
    const n = children.length;
    if (n === 0) { modSizes[mod] = { w: 0, h: 0, cols: 0, rows: 0 }; continue; }
    const cols = Math.ceil(Math.sqrt(n * 1.6));
    const rows = Math.ceil(n / cols);
    modSizes[mod] = { w: cols * cellW, h: rows * cellH + 16, cols, rows };
  }

  // Sort nodes by edge count (descending)
  const edgeCount = {};
  for (const e of edgeList) {
    edgeCount[e.s] = (edgeCount[e.s] || 0) + 1;
    edgeCount[e.t] = (edgeCount[e.t] || 0) + 1;
  }
  for (const mod of Object.keys(modules)) {
    modChildList[mod].sort((a, b) => (edgeCount[b] || 0) - (edgeCount[a] || 0));
  }

  // Arrange modules in rows (bin-packing)
  const modOrder = Object.keys(modules).filter(m => modSizes[m].w > 0);
  modOrder.sort((a, b) => modSizes[b].w * modSizes[b].h - modSizes[a].w * modSizes[a].h);

  const maxRowW = 1100;
  let curX = 0, curY = 0, rowH = 0;

  for (const mod of modOrder) {
    const sz = modSizes[mod];
    if (curX + sz.w > maxRowW && curX > 0) {
      curX = 0;
      curY += rowH + modGap;
      rowH = 0;
    }
    modulePositions[mod] = { x: curX, y: curY, w: sz.w, h: sz.h };
    curX += sz.w + modGap;
    rowH = Math.max(rowH, sz.h);
  }

  // Position children in grid inside each module
  for (const mod of Object.keys(modules)) {
    const children = modChildList[mod];
    if (children.length === 0) continue;
    const sz = modSizes[mod];
    const mp = modulePositions[mod];

    children.forEach((id, i) => {
      const col = i % sz.cols;
      const row = Math.floor(i / sz.cols);
      nodePositions[id] = {
        x: mp.x + col * cellW + cellW / 2 - (nodeSizes[id]?.w || 80) / 2,
        y: mp.y + row * cellH + 16 + cellH / 2 - 12,
      };
    });
  }

  // Stretch Y spacing to fill viewport
  const allRects = [];
  for (const id of Object.keys(nodePositions)) {
    const p = nodePositions[id], s = nodeSizes[id] || { w: 80, h: 24 };
    allRects.push({ x: p.x, y: p.y, w: s.w, h: s.h });
  }
  const bb = computeBounds(allRects);
  const viewAspect = (container.clientHeight || 800) / (container.clientWidth || 1100);
  const layoutAspect = bb.h / bb.w;
  if (layoutAspect < viewAspect && bb.h > 0) {
    const stretch = viewAspect / layoutAspect;
    const centerY = bb.y + bb.h / 2;
    for (const id of Object.keys(nodePositions)) {
      const p = nodePositions[id];
      p.y = centerY + (p.y - centerY) * stretch;
    }
    // Also stretch module positions
    for (const mod of Object.keys(modulePositions)) {
      const mp = modulePositions[mod];
      const newY = centerY + (mp.y - centerY) * stretch;
      const newH = mp.h * stretch;
      mp.y = newY;
      mp.h = newH;
    }
  }

  applyPositions();

  // Fit to view (recompute bounds after stretch)
  const fitRects = Object.keys(nodePositions).map(id => {
    const p = nodePositions[id], s = nodeSizes[id] || { w: 80, h: 24 };
    return { x: p.x, y: p.y, w: s.w, h: s.h };
  });
  canvas.fit(computeBounds(fitRects), 60);
}

function applyPositions() {
  // Position class nodes
  for (const [id, pos] of Object.entries(nodePositions)) {
    const g = svgNodeEls[id];
    if (g) g.setAttribute('transform', `translate(${pos.x}, ${pos.y})`);
  }

  // Update module backgrounds to wrap their children
  for (const [mod, info] of Object.entries(modules)) {
    const g = svgModuleEls[mod];
    if (!g) continue;
    const children = (moduleNodes[mod] || []).filter(id => filterState[mod] && nodePositions[id]);
    if (children.length === 0) { g.style.display = 'none'; continue; }
    g.style.display = '';

    let x1 = Infinity, y1 = Infinity, x2 = -Infinity, y2 = -Infinity;
    for (const id of children) {
      const p = nodePositions[id], s = nodeSizes[id] || { w: 80, h: 24 };
      x1 = Math.min(x1, p.x);
      y1 = Math.min(y1, p.y);
      x2 = Math.max(x2, p.x + s.w);
      y2 = Math.max(y2, p.y + s.h);
    }
    const pad = 8;
    g._rect.setAttribute('x', x1 - pad);
    g._rect.setAttribute('y', y1 - pad - 14);
    g._rect.setAttribute('width', x2 - x1 + pad * 2);
    g._rect.setAttribute('height', y2 - y1 + pad * 2 + 14);
    g._label.setAttribute('x', (x1 + x2) / 2);
    g._label.setAttribute('y', y1 - pad - 10);
  }

  // Update edges
  for (const e of edgeList) {
    const sp = nodePositions[e.s], tp = nodePositions[e.t];
    const ss = nodeSizes[e.s] || { w: 80, h: 24 }, ts = nodeSizes[e.t] || { w: 80, h: 24 };
    const ref = svgEdgeEls[e.index];
    if (!ref || !sp || !tp) { if (ref) ref.path.setAttribute('d', ''); continue; }

    // Edge hidden if either module filtered out
    const sMod = nodeMap[e.s]?.module, tMod = nodeMap[e.t]?.module;
    const visible = filterState[sMod] && filterState[tMod];
    ref.path.style.display = visible ? '' : 'none';
    if (ref.label) ref.label.style.display = visible ? '' : 'none';
    if (!visible) continue;

    const sx = sp.x + ss.w / 2, sy = sp.y + ss.h;
    const tx = tp.x + ts.w / 2, ty = tp.y;
    ref.path.setAttribute('d', drawEdgePath([{ x: sx, y: sy }, { x: tx, y: ty }]));

    if (ref.label) {
      const mx = (sx + tx) / 2, my = (sy + ty) / 2;
      const tw = parseFloat(ref.label._bg.getAttribute('width'));
      ref.label.setAttribute('transform', `translate(${mx - tw / 2}, ${my - 6})`);
    }
  }
}

// ── Initial render ───────────────────────────────────────────
renderGraph();
runCompactLayout();

// ── Highlight / selection ────────────────────────────────────
function highlightNode(id) {

  // Find neighborhood
  const neighbors = new Set([id]);
  for (const e of edgeList) {
    if (e.s === id) neighbors.add(e.t);
    if (e.t === id) neighbors.add(e.s);
  }

  // Fade all, un-fade neighborhood
  for (const [nid, g] of Object.entries(svgNodeEls)) {
    g.style.opacity = neighbors.has(nid) ? '1' : '0.08';
    if (nid === id || neighbors.has(nid)) {
      g._rect.setAttribute('stroke-width', '3');
    } else {
      g._rect.setAttribute('stroke-width', keyNodes.has(nid) ? '2.5' : '1.5');
    }
  }

  // Fade modules that have no visible nodes
  for (const [mod, g] of Object.entries(svgModuleEls)) {
    const hasVisible = (moduleNodes[mod] || []).some(nid => neighbors.has(nid));
    g.style.opacity = hasVisible ? '1' : '0.08';
  }

  // Highlight edges
  for (const e of edgeList) {
    const ref = svgEdgeEls[e.index];
    if (!ref) continue;
    const connected = (e.s === id || e.t === id);
    ref.path.style.opacity = connected ? '1' : '0.08';
    if (connected) ref.path.setAttribute('stroke-width', '2.5');
    else ref.path.setAttribute('stroke-width', '1.2');
    if (ref.label) ref.label.style.opacity = connected ? '1' : '0.08';
  }

  showInfo(id);
}

function resetHighlight() {
  for (const [nid, g] of Object.entries(svgNodeEls)) {
    g.style.opacity = '';
    g._rect.setAttribute('stroke-width', keyNodes.has(nid) ? '2.5' : '1.5');
  }
  for (const [mod, g] of Object.entries(svgModuleEls)) {
    g.style.opacity = '';
  }
  for (const e of edgeList) {
    const ref = svgEdgeEls[e.index];
    if (!ref) continue;
    ref.path.style.opacity = '0.55';
    ref.path.setAttribute('stroke-width', '1.2');
    if (ref.label) ref.label.style.opacity = '1';
  }
  document.getElementById('info').style.display = 'none';
}

// Click background → reset
canvas.svg.addEventListener('click', (e) => {
  if (e.target === canvas.svg || e.target === canvas.root) resetHighlight();
});
canvas.svg.addEventListener('dblclick', (e) => {
  if (e.target === canvas.svg || e.target === canvas.root) {
    resetHighlight();
    const allRects = [];
    for (const id of Object.keys(nodePositions)) {
      const p = nodePositions[id], s = nodeSizes[id] || { w: 80, h: 24 };
      allRects.push({ x: p.x, y: p.y, w: s.w, h: s.h });
    }
    canvas.animateFit(computeBounds(allRects), 60, 400);
  }
});

// ── Info panel ───────────────────────────────────────────────
function showInfo(id) {
  const n = nodeMap[id];
  if (!n) return;
  const info = document.getElementById('info');
  const color = modules[n.module]?.color || '#8b949e';

  const incoming = edgeList.filter(e => e.t === id).map(e => nodeMap[e.s]).filter(Boolean);
  const outgoing = edgeList.filter(e => e.s === id).map(e => nodeMap[e.t]).filter(Boolean);

  const depLinks = (arr, dir) => {
    if (!arr.length) return '';
    const unique = [...new Map(arr.map(n => [n.id, n])).values()];
    return `<div class="deps">${dir}: ${unique.map(n => `<span onclick="focusNode('${n.id}')">${n.label}</span>`).join(', ')}</div>`;
  };

  const drillBtn = drillButtonHtml(id);

  info.innerHTML = `
    <h3>${n.label}</h3>
    <div class="module-tag" style="background:${color}22;color:${color};border:1px solid ${color}44">${n.module}</div>
    <div class="desc">${n.desc}</div>
    ${depLinks(incoming, '← Used by')}
    ${depLinks(outgoing, '→ Depends on')}
    ${drillBtn}
  `;
  info.style.display = 'block';
}

window.focusNode = function(id) {
  const pos = nodePositions[id];
  const sz = nodeSizes[id];
  if (!pos || !sz) return;
  highlightNode(id);
  canvas.animateFit({ x: pos.x - 100, y: pos.y - 80, w: sz.w + 200, h: sz.h + 160 }, 40, 400);
};

// ── Filter pills ─────────────────────────────────────────────
const filtersEl = document.getElementById('filters');
for (const [mod, info] of Object.entries(modules)) {
  const pill = document.createElement('div');
  pill.className = 'pill active';
  pill.textContent = info.label;
  pill.style.borderColor = info.color;
  pill.style.color = info.color;
  pill.style.backgroundColor = info.color + '18';
  pill.dataset.mod = mod;
  pill.addEventListener('click', () => {
    filterState[mod] = !filterState[mod];
    pill.className = `pill ${filterState[mod] ? 'active' : 'inactive'}`;

    // Toggle visibility
    const g = svgModuleEls[mod];
    if (g) g.style.display = filterState[mod] ? '' : 'none';
    for (const id of (moduleNodes[mod] || [])) {
      const ng = svgNodeEls[id];
      if (ng) ng.style.display = filterState[mod] ? '' : 'none';
    }
    // Update edge visibility
    for (const e of edgeList) {
      const sMod = nodeMap[e.s]?.module, tMod = nodeMap[e.t]?.module;
      const vis = filterState[sMod] && filterState[tMod];
      const ref = svgEdgeEls[e.index];
      if (ref) {
        ref.path.style.display = vis ? '' : 'none';
        if (ref.label) ref.label.style.display = vis ? '' : 'none';
      }
    }
  });
  filtersEl.appendChild(pill);
}

// ── Search ───────────────────────────────────────────────────
const searchEl = document.getElementById('search');
searchEl.addEventListener('input', () => {
  const q = searchEl.value.trim().toLowerCase();
  // Clear previous search state
  for (const [id, g] of Object.entries(svgNodeEls)) {
    g.style.opacity = '';
    g._rect.setAttribute('stroke-width', keyNodes.has(id) ? '2.5' : '1.5');
    g._rect.setAttribute('stroke', hexAlpha(modules[nodeMap[id].module].color, 0x99));
    g._rect.setAttribute('fill', '#21262d');
  }
  for (const [mod, g] of Object.entries(svgModuleEls)) g.style.opacity = '';
  for (const e of edgeList) {
    const ref = svgEdgeEls[e.index];
    if (ref) { ref.path.style.opacity = '0.55'; if (ref.label) ref.label.style.opacity = '1'; }
  }
  document.getElementById('info').style.display = 'none';

  if (!q) return;

  const matches = nodes.filter(n =>
    n.label.toLowerCase().includes(q) || n.desc.toLowerCase().includes(q)
  );

  if (matches.length > 0 && matches.length < 20) {
    const matchSet = new Set(matches.map(m => m.id));
    // Fade non-matches
    for (const [id, g] of Object.entries(svgNodeEls)) {
      if (matchSet.has(id)) {
        g.style.opacity = '1';
        g._rect.setAttribute('stroke', '#ffa657');
        g._rect.setAttribute('stroke-width', '3');
        g._rect.setAttribute('fill', 'rgba(255, 166, 87, 0.2)');
      } else {
        g.style.opacity = '0.08';
      }
    }
    for (const [mod, g] of Object.entries(svgModuleEls)) {
      const hasMatch = (moduleNodes[mod] || []).some(id => matchSet.has(id));
      g.style.opacity = hasMatch ? '1' : '0.08';
    }
    if (matches.length === 1) {
      const m = matches[0];
      const pos = nodePositions[m.id], sz = nodeSizes[m.id];
      if (pos && sz) canvas.animateFit({ x: pos.x - 100, y: pos.y - 80, w: sz.w + 200, h: sz.h + 160 }, 40, 300);
      showInfo(m.id);
    }
  } else if (matches.length > 0) {
    const matchSet = new Set(matches.map(m => m.id));
    for (const [id, g] of Object.entries(svgNodeEls)) {
      if (matchSet.has(id)) {
        g._rect.setAttribute('stroke', '#ffa657');
        g._rect.setAttribute('stroke-width', '3');
      }
    }
  }
});

// ── Keyboard ─────────────────────────────────────────────────
document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') {
    if (document.getElementById('drilldown').classList.contains('visible')) {
      closeOverlay();
    } else {
      searchEl.value = '';
      searchEl.blur();
      resetHighlight();
      // Also clear search highlight
      searchEl.dispatchEvent(new Event('input'));
    }
  }
  if (e.key === '/' && document.activeElement !== searchEl) { e.preventDefault(); searchEl.focus(); }
});

// ── Stats ────────────────────────────────────────────────────
document.getElementById('stats').textContent =
  `${nodes.length} classes · ${edges.length} connections · ${Object.keys(flowRegistry).length} drill-downs`;

// ── Wire entry points and restore state ──────────────────────
document.getElementById('drill-back').addEventListener('click', goBack);
restoreFromHash();

// ── Responsive relayout ──────────────────────────────────────
let _resizeTimer;
window.addEventListener('resize', () => {
  clearTimeout(_resizeTimer);
  _resizeTimer = setTimeout(() => { runCompactLayout(); }, 200);
});
