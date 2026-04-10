// Flow view — SVG + dagre layout

let flowCanvas = null;
let flowHistory = [];
let currentFlowId = null;

// Node type styling
const flowNodeStyles = {
  start:    { fill: '#238636', stroke: '#2ea043', color: '#ffffff', fontWeight: 'bold' },
  step:     { fill: '#161b22', stroke: '#30363d', color: '#c9d1d9', fontWeight: 'normal' },
  key:      { fill: '#161b22', stroke: '#30363d', color: '#c9d1d9', fontWeight: 'normal' },
  decision: { fill: '#0d1117', stroke: '#a371f7', color: '#c9d1d9', fontWeight: 'normal' },
  error:    { fill: 'rgba(218, 54, 51, 0.08)', stroke: '#da3633', color: '#f85149', fontWeight: 'normal' },
  end:      { fill: '#161b22', stroke: '#3fb950', color: '#c9d1d9', fontWeight: 'normal' },
};

function openFlow(flowId) {
  const flow = flowRegistry[flowId];
  if (!flow) { console.warn('Flow not found:', flowId); return; }
  document.getElementById('drilldown').classList.add('visible');
  currentFlowId = flowId;
  updateBreadcrumbs();
  renderFlow(flowId);
  updateHash();
}

function goBack() {
  if (flowHistory.length > 0) {
    const prev = flowHistory.pop();
    openFlow(prev.flowId);
  } else {
    closeOverlay();
  }
}

function goToBreadcrumb(index) {
  const target = flowHistory[index];
  if (!target) return;
  flowHistory = flowHistory.slice(0, index);
  openFlow(target.flowId);
}

function closeOverlay() {
  document.getElementById('drilldown').classList.remove('visible');
  if (flowCanvas) { flowCanvas.destroy(); flowCanvas = null; }
  flowHistory = [];
  currentFlowId = null;
  _expansions = {};
  updateHash();
}

function updateBreadcrumbs() {
  const el = document.getElementById('breadcrumbs');
  if (!el) return;
  let html = '';
  flowHistory.forEach((h, i) => {
    html += `<span class="crumb" onclick="goToBreadcrumb(${i})">${h.title}</span>`;
    html += '<span class="crumb-sep">\u203a</span>';
  });
  const current = flowRegistry[currentFlowId];
  html += `<span class="crumb-current">${current ? current.title : currentFlowId}</span>`;
  el.innerHTML = html;
}

// ── Flow data store ──────────────────────────────────────────
let _flowNodes = {};     // id → {id, label, type, desc, drillTarget, x, y, w, h}
let _flowEdges = [];     // {source, target, label, index, points}
let _flowNodeEls = {};   // id → <g>
let _flowEdgeEls = {};   // index → {path, label}
let _expansions = {};    // nodeId → {addedNodeIds, addedEdgeIndices, removedEdges, subHeight, threshold}

function renderFlow(flowId) {
  const flow = flowRegistry[flowId];
  if (!flow) return;

  if (flowCanvas) { flowCanvas.destroy(); flowCanvas = null; }
  _flowNodes = {};
  _flowEdges = [];
  _flowNodeEls = {};
  _flowEdgeEls = {};
  _expansions = {};

  const container = document.getElementById('flow-cy');
  if (!container) return;
  document.getElementById('flow-info').style.display = 'none';

  flowCanvas = createSvgCanvas(container);

  // Add arrow markers for flow edges
  addArrowMarker(flowCanvas.defs, 'flow-arrow', '#484f58');
  addArrowMarker(flowCanvas.defs, 'flow-arrow-yes', '#3fb950');
  addArrowMarker(flowCanvas.defs, 'flow-arrow-no', '#da3633');

  // Build dagre graph
  const g = new dagre.graphlib.Graph();
  g.setGraph({ rankdir: 'TB', nodesep: 50, ranksep: 55, edgesep: 20, ranker: 'longest-path' });
  g.setDefaultEdgeLabel(function() { return {}; });

  const fontSize = 11;
  for (const n of flow.nodes) {
    const m = measureMultilineText(n[1], fontSize);
    const w = Math.min(Math.max(m.width + 24, 60), 220);
    const h = Math.max(m.lines.length * 16 + 20, 36);
    const type = n[2] || 'step';
    // Diamonds need more space for dagre
    const dagreW = type === 'decision' ? w * 1.4 : w;
    const dagreH = type === 'decision' ? h * 1.4 : h;
    g.setNode(n[0], { width: dagreW, height: dagreH });
    _flowNodes[n[0]] = { id: n[0], label: n[1], type, desc: n[3] || '', drillTarget: n[4] || '', w, h };
  }
  for (let i = 0; i < flow.edges.length; i++) {
    const e = flow.edges[i];
    g.setEdge(e[0], e[1]);
    _flowEdges.push({ source: e[0], target: e[1], label: e[2] || '', index: i });
  }

  dagre.layout(g);

  // Read positions from dagre
  for (const id of g.nodes()) {
    const nd = g.node(id);
    if (nd && _flowNodes[id]) {
      _flowNodes[id].x = nd.x - _flowNodes[id].w / 2;
      _flowNodes[id].y = nd.y - _flowNodes[id].h / 2;
    }
  }
  for (let i = 0; i < _flowEdges.length; i++) {
    const e = _flowEdges[i];
    const dagreEdge = g.edge(e.source, e.target);
    e.points = dagreEdge ? dagreEdge.points : [];
  }

  renderFlowElements();

  // Fit to content
  const allRects = Object.values(_flowNodes).map(n => ({ x: n.x, y: n.y, w: n.w, h: n.h }));
  flowCanvas.fit(computeBounds(allRects), 40);
}

function renderFlowElements() {
  flowCanvas.root.innerHTML = '';

  const edgeLayer = svgEl('g', { class: 'flow-edge-layer' });
  flowCanvas.root.appendChild(edgeLayer);
  const nodeLayer = svgEl('g', { class: 'flow-node-layer' });
  flowCanvas.root.appendChild(nodeLayer);

  // Render edges
  for (const e of _flowEdges) {
    const ref = renderFlowEdge(e, edgeLayer);
    _flowEdgeEls[e.index] = ref;
  }

  // Render nodes
  for (const [id, n] of Object.entries(_flowNodes)) {
    const g = renderFlowNode(n, nodeLayer);
    _flowNodeEls[id] = g;
  }

  // Background click → clear highlight
  flowCanvas.svg.addEventListener('click', (e) => {
    if (e.target === flowCanvas.svg || e.target === flowCanvas.root ||
        e.target.closest('.flow-edge-layer')) {
      clearFlowHighlight();
    }
  });
}

function renderFlowNode(n, parent) {
  const style = flowNodeStyles[n.type] || flowNodeStyles.step;
  const hasDrill = !!n.drillTarget;
  const isExpanded = !!_expansions[n.id];

  const g = svgEl('g', {
    class: 'flow-node',
    'data-id': n.id,
    transform: `translate(${n.x}, ${n.y})`,
    cursor: 'pointer'
  });

  let strokeColor = style.stroke;
  let strokeWidth = '2';
  let strokeDash = '';

  if (isExpanded) {
    strokeColor = '#58a6ff';
    strokeWidth = '3';
    strokeDash = '6,3';
  } else if (hasDrill) {
    strokeColor = '#d29922';
    strokeWidth = '2.5';
  }

  if (n.type === 'decision') {
    // Diamond shape
    const cx = n.w / 2, cy = n.h / 2;
    const dx = n.w / 2 + 8, dy = n.h / 2 + 8;
    const diamond = svgEl('polygon', {
      points: `${cx},${-8} ${cx + dx},${cy} ${cx},${n.h + 8} ${cx - dx},${cy}`,
      fill: style.fill, stroke: strokeColor, 'stroke-width': strokeWidth
    });
    if (strokeDash) diamond.setAttribute('stroke-dasharray', strokeDash);
    g.appendChild(diamond);
  } else {
    const rect = svgEl('rect', {
      width: n.w, height: n.h, rx: '4', ry: '4',
      fill: isExpanded ? 'rgba(88, 166, 255, 0.08)' : style.fill,
      stroke: strokeColor, 'stroke-width': strokeWidth
    });
    if (strokeDash) rect.setAttribute('stroke-dasharray', strokeDash);
    g.appendChild(rect);
  }

  // Text (multiline)
  const lines = n.label.split('\n');
  const lineH = 16;
  const textStartY = n.h / 2 - (lines.length - 1) * lineH / 2;
  for (let i = 0; i < lines.length; i++) {
    const text = svgEl('text', {
      x: n.w / 2, y: textStartY + i * lineH,
      fill: style.color, 'font-size': n.type === 'decision' ? '10' : '11',
      'font-weight': style.fontWeight,
      'text-anchor': 'middle', 'dominant-baseline': 'central',
      'pointer-events': 'none'
    });
    text.textContent = lines[i];
    g.appendChild(text);
  }

  // Event handlers
  g.addEventListener('click', (e) => {
    e.stopPropagation();
    highlightFlowNode(n.id);
  });
  g.addEventListener('dblclick', (e) => {
    e.stopPropagation();
    if (n.drillTarget && flowRegistry[n.drillTarget]) {
      expandInline(n.id);
    }
  });

  parent.appendChild(g);
  return g;
}

function renderFlowEdge(e, parent) {
  if (!e.points || e.points.length === 0) return { path: svgEl('path'), label: null };

  // Determine edge color
  let color = '#484f58', arrowId = 'flow-arrow';
  if (e.label && e.label.includes('Yes')) { color = 'rgba(63, 185, 80, 0.47)'; arrowId = 'flow-arrow-yes'; }
  if (e.label && e.label.includes('No')) { color = 'rgba(218, 54, 51, 0.47)'; arrowId = 'flow-arrow-no'; }

  // Shorten path to stop before node border
  const targetNode = _flowNodes[e.target];
  const pts = [...e.points];

  if (targetNode && pts.length >= 2) {
    const last = pts[pts.length - 1];
    const prev = pts[pts.length - 2];
    const shrink = targetNode.type === 'decision' ? 12 : 8;
    const angle = Math.atan2(last.y - prev.y, last.x - prev.x);
    pts[pts.length - 1] = { x: last.x - Math.cos(angle) * shrink, y: last.y - Math.sin(angle) * shrink };
  }

  const path = svgEl('path', {
    d: drawEdgePath(pts),
    fill: 'none', stroke: color, 'stroke-width': '1.5',
    'marker-end': `url(#${arrowId})`
  });
  parent.appendChild(path);

  // Edge label
  let labelEl = null;
  if (e.label) {
    const mid = pts[Math.floor(pts.length / 2)];
    const tw = measureText(e.label, 9) + 6;
    const lg = svgEl('g', {
      transform: `translate(${mid.x - tw / 2}, ${mid.y - 7})`,
      'pointer-events': 'none'
    });
    const bg = svgEl('rect', {
      width: tw, height: 14, rx: '2', ry: '2',
      fill: '#0d1117', opacity: '0.9'
    });
    lg.appendChild(bg);
    const lt = svgEl('text', {
      x: tw / 2, y: 10,
      fill: color === '#484f58' ? '#7d8590' : color.replace(/[\d.]+\)$/, '1)'),
      'font-size': '9', 'text-anchor': 'middle'
    });
    lt.textContent = e.label;
    lg.appendChild(lt);
    parent.appendChild(lg);
    labelEl = lg;
  }

  return { path, label: labelEl };
}

// ── Flow highlight ───────────────────────────────────────────
function highlightFlowNode(id) {
  const n = _flowNodes[id];
  if (!n) return;

  // Find neighborhood
  const neighbors = new Set([id]);
  for (const e of _flowEdges) {
    if (e.source === id) neighbors.add(e.target);
    if (e.target === id) neighbors.add(e.source);
  }

  // Fade non-neighbors
  for (const [nid, g] of Object.entries(_flowNodeEls)) {
    g.style.opacity = neighbors.has(nid) ? '1' : '0.25';
  }
  for (const e of _flowEdges) {
    const ref = _flowEdgeEls[e.index];
    if (!ref) continue;
    const connected = e.source === id || e.target === id;
    ref.path.style.opacity = connected ? '1' : '0.25';
    if (ref.label) ref.label.style.opacity = connected ? '1' : '0.25';
  }

  // Show info
  const info = document.getElementById('flow-info');
  let html = '<strong>' + n.label.replace(/\n/g, ' ') + '</strong>';
  if (n.desc) html += '<br>' + n.desc;
  if (n.drillTarget) html += '<br><em style="color:#58a6ff">Double-click to drill into \u2192 ' + (flowRegistry[n.drillTarget]?.title || n.drillTarget) + '</em>';
  info.innerHTML = html;
  info.style.display = 'block';
}

function clearFlowHighlight() {
  for (const [id, g] of Object.entries(_flowNodeEls)) g.style.opacity = '';
  for (const e of _flowEdges) {
    const ref = _flowEdgeEls[e.index];
    if (ref) {
      ref.path.style.opacity = '';
      if (ref.label) ref.label.style.opacity = '';
    }
  }
  document.getElementById('flow-info').style.display = 'none';
}

// ── Inline expansion ─────────────────────────────────────────
function expandInline(nodeId) {
  if (!flowCanvas) return;

  // Toggle: collapse if already expanded
  if (_expansions[nodeId]) { collapseInline(nodeId); return; }

  const n = _flowNodes[nodeId];
  if (!n || !n.drillTarget) return;
  const flow = flowRegistry[n.drillTarget];
  if (!flow) return;

  const pfx = nodeId + '__';

  // Save and hide outgoing edges
  const removedEdges = [];
  for (let i = _flowEdges.length - 1; i >= 0; i--) {
    const e = _flowEdges[i];
    if (e.source === nodeId) {
      removedEdges.push({ source: e.source, target: e.target, label: e.label, index: e.index });
      // Hide the edge visually
      const ref = _flowEdgeEls[e.index];
      if (ref) {
        ref.path.style.display = 'none';
        if (ref.label) ref.label.style.display = 'none';
      }
    }
  }

  // Find start and terminal nodes in sub-flow
  const intSources = new Set(flow.edges.map(e => e[0]));
  const starts = flow.nodes.filter(fn => fn[2] === 'start').map(fn => pfx + fn[0]);
  const ends = flow.nodes.filter(fn => !intSources.has(fn[0])).map(fn => pfx + fn[0]);

  // Build sub-flow dagre layout
  const sg = new dagre.graphlib.Graph();
  sg.setGraph({ rankdir: 'TB', nodesep: 50, ranksep: 55, edgesep: 20, ranker: 'longest-path' });
  sg.setDefaultEdgeLabel(function() { return {}; });

  const subNodeIds = [];
  for (const fn of flow.nodes) {
    const id = pfx + fn[0];
    const m = measureMultilineText(fn[1], 11);
    const w = Math.min(Math.max(m.width + 24, 60), 220);
    const h = Math.max(m.lines.length * 16 + 20, 36);
    const type = fn[2] || 'step';
    const dagreW = type === 'decision' ? w * 1.4 : w;
    const dagreH = type === 'decision' ? h * 1.4 : h;
    sg.setNode(id, { width: dagreW, height: dagreH });
    _flowNodes[id] = { id, label: fn[1], type, desc: fn[3] || '', drillTarget: fn[4] || '', w, h };
    subNodeIds.push(id);
  }
  const subEdges = [];
  for (let i = 0; i < flow.edges.length; i++) {
    const e = flow.edges[i];
    const sId = pfx + e[0], tId = pfx + e[1];
    sg.setEdge(sId, tId);
    const idx = _flowEdges.length;
    const fe = { source: sId, target: tId, label: e[2] || '', index: idx, points: [] };
    _flowEdges.push(fe);
    subEdges.push(fe);
  }

  dagre.layout(sg);

  // Read positions
  for (const id of sg.nodes()) {
    const nd = sg.node(id);
    if (nd && _flowNodes[id]) {
      _flowNodes[id].x = nd.x - _flowNodes[id].w / 2;
      _flowNodes[id].y = nd.y - _flowNodes[id].h / 2;
    }
  }
  for (const fe of subEdges) {
    const dagreEdge = sg.edge(fe.source, fe.target);
    fe.points = dagreEdge ? dagreEdge.points : [];
  }

  // Reposition sub-flow below expanded node
  const subRects = subNodeIds.map(id => ({ x: _flowNodes[id].x, y: _flowNodes[id].y, w: _flowNodes[id].w, h: _flowNodes[id].h }));
  const subBB = computeBounds(subRects);
  const offsetX = (n.x + n.w / 2) - (subBB.x + subBB.w / 2);
  const offsetY = (n.y + n.h + 60) - subBB.y;
  const subHeight = subBB.h + 60;

  for (const id of subNodeIds) {
    _flowNodes[id].x += offsetX;
    _flowNodes[id].y += offsetY;
  }
  for (const fe of subEdges) {
    if (fe.points) {
      fe.points = fe.points.map(p => ({ x: p.x + offsetX, y: p.y + offsetY }));
    }
  }

  // Shift all existing nodes below the expanded node down
  const threshold = n.y + 20;
  for (const [id, fn] of Object.entries(_flowNodes)) {
    if (id !== nodeId && !subNodeIds.includes(id) && fn.y > threshold) {
      fn.y += subHeight;
    }
  }

  // Add bridge edges: expanded node → sub-flow start nodes
  const bridgeEdges = [];
  for (const s of starts) {
    const idx = _flowEdges.length;
    const sn = _flowNodes[nodeId], tn = _flowNodes[s];
    const pts = [
      { x: sn.x + sn.w / 2, y: sn.y + sn.h },
      { x: tn.x + tn.w / 2, y: tn.y }
    ];
    const fe = { source: nodeId, target: s, label: '', index: idx, points: pts };
    _flowEdges.push(fe);
    bridgeEdges.push(fe);
  }
  // Bridge: sub-flow end nodes → original targets
  for (const re of removedEdges) {
    for (const en of ends) {
      const idx = _flowEdges.length;
      const sn = _flowNodes[en], tn = _flowNodes[re.target];
      if (!sn || !tn) continue;
      const pts = [
        { x: sn.x + sn.w / 2, y: sn.y + sn.h },
        { x: tn.x + tn.w / 2, y: tn.y }
      ];
      const fe = { source: en, target: re.target, label: re.label, index: idx, points: pts };
      _flowEdges.push(fe);
      bridgeEdges.push(fe);
    }
  }

  // Update edge points for shifted nodes
  for (const e of _flowEdges) {
    if (subEdges.includes(e) || bridgeEdges.includes(e)) continue;
    if (removedEdges.some(re => re.index === e.index)) continue;
    const sn = _flowNodes[e.source], tn = _flowNodes[e.target];
    if (sn && tn) {
      e.points = [
        { x: sn.x + sn.w / 2, y: sn.y + sn.h },
        { x: tn.x + tn.w / 2, y: tn.y }
      ];
    }
  }

  // Save expansion state
  _expansions[nodeId] = {
    subNodeIds,
    subEdges,
    bridgeEdges,
    removedEdges,
    subHeight,
    threshold
  };

  // Re-render everything
  renderFlowElements();
}

function collapseInline(nodeId) {
  if (!flowCanvas || !_expansions[nodeId]) return;
  const exp = _expansions[nodeId];

  // Remove sub-flow nodes
  for (const id of exp.subNodeIds) {
    delete _flowNodes[id];
    delete _flowNodeEls[id];
  }

  // Remove sub-flow and bridge edges from _flowEdges
  const removeSet = new Set();
  for (const e of exp.subEdges) removeSet.add(e.index);
  for (const e of exp.bridgeEdges) removeSet.add(e.index);
  _flowEdges = _flowEdges.filter(e => !removeSet.has(e.index));

  // Shift nodes back up
  for (const [id, fn] of Object.entries(_flowNodes)) {
    if (id !== nodeId && fn.y > exp.threshold + exp.subHeight / 2) {
      fn.y -= exp.subHeight;
    }
  }

  // Update all edge points for shifted positions
  for (const e of _flowEdges) {
    const sn = _flowNodes[e.source], tn = _flowNodes[e.target];
    if (sn && tn) {
      e.points = [
        { x: sn.x + sn.w / 2, y: sn.y + sn.h },
        { x: tn.x + tn.w / 2, y: tn.y }
      ];
    }
  }

  delete _expansions[nodeId];

  // Re-render
  renderFlowElements();
}

// ── URL hash persistence + browser history ───────────────────
let _suppressPush = false;

function updateHash() {
  const hash = currentFlowId
    ? '#' + flowHistory.map(h => h.flowId).concat(currentFlowId).join('/')
    : '';
  const url = hash || (window.location.pathname + window.location.search);
  if (_suppressPush) {
    history.replaceState(null, '', url);
  } else {
    history.pushState(null, '', url);
  }
}

function restoreFromHash() {
  const hash = window.location.hash.slice(1);
  if (!hash) {
    if (currentFlowId) closeOverlaySilent();
    return;
  }
  const parts = hash.split('/').filter(Boolean);
  if (parts.length === 0) { if (currentFlowId) closeOverlaySilent(); return; }
  for (const id of parts) {
    if (!flowRegistry[id]) return;
  }
  flowHistory = [];
  for (let i = 0; i < parts.length - 1; i++) {
    const flow = flowRegistry[parts[i]];
    flowHistory.push({ flowId: parts[i], title: flow ? flow.title : parts[i] });
  }
  _suppressPush = true;
  openFlow(parts[parts.length - 1]);
  _suppressPush = false;
}

function closeOverlaySilent() {
  document.getElementById('drilldown').classList.remove('visible');
  if (flowCanvas) { flowCanvas.destroy(); flowCanvas = null; }
  flowHistory = [];
  currentFlowId = null;
  _expansions = {};
}

window.addEventListener('popstate', () => { restoreFromHash(); });
