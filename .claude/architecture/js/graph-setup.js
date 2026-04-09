// Main Cytoscape graph: setup, layout, interactions, search
// ── Build Cytoscape ──────────────────────────────────────────
const cyNodes = [];
const cyEdges = [];

for (const [mod, info] of Object.entries(modules)) {
  cyNodes.push({ data: { id: `mod_${mod}`, label: info.label, nodeType: 'module', module: mod }, classes: 'module' });
}
for (const n of nodes) {
  const hasDrill = !!flowIdForNode(n.id, n.module);
  cyNodes.push({ data: { id: n.id, label: n.label, desc: n.desc, module: n.module, parent: `mod_${n.module}`, nodeType: 'class', hasDrill }, classes: 'classNode' });
}
for (let i = 0; i < edges.length; i++) {
  const e = edges[i];
  cyEdges.push({ data: { id: `e${i}`, source: e.s, target: e.t, label: e.label || '', edgeType: e.type }, classes: e.type });
}

const cy = cytoscape({
  container: document.getElementById('cy'),
  elements: { nodes: cyNodes, edges: cyEdges },
  wheelSensitivity: 0.3, minZoom: 0.08, maxZoom: 3,
  style: [
    { selector: '.module', style: {
      'shape': 'round-rectangle', 'background-color': '#161b22', 'border-width': 2,
      'border-color': '#30363d', 'padding': '3px', 'text-valign': 'top', 'text-halign': 'center',
      'label': 'data(label)', 'font-size': 11, 'font-weight': 'bold', 'color': '#484f58',
      'text-margin-y': 2, 'compound-sizing-wrt-labels': 'include',
    }},
    ...Object.entries(modules).map(([mod, info]) => ({
      selector: `#mod_${mod}`, style: { 'border-color': info.color + '88', 'color': info.color }
    })),
    { selector: '.classNode', style: {
      'shape': 'round-rectangle', 'width': 'label', 'height': 'label', 'padding': '6px',
      'background-color': '#21262d', 'border-width': 1.5, 'border-color': '#30363d',
      'label': 'data(label)', 'font-size': 10, 'color': '#e6edf3',
      'text-valign': 'center', 'text-halign': 'center', 'text-wrap': 'wrap', 'text-max-width': 100,
      'transition-property': 'background-color, border-color, border-width, opacity', 'transition-duration': '0.2s',
    }},
    ...Object.entries(modules).map(([mod, info]) => ({
      selector: `.classNode[module="${mod}"]`, style: { 'border-color': info.color + '99' }
    })),
    ...['graphic_elem', 'scene', 'simulation', 'mainwindow', 'main'].map(id => ({
      selector: `#${id}`, style: { 'border-width': 2.5, 'font-weight': 'bold', 'font-size': 11 }
    })),
    { selector: 'edge', style: {
      'width': 1.2, 'line-color': '#30363d', 'target-arrow-color': '#30363d',
      'target-arrow-shape': 'triangle', 'arrow-scale': 0.8, 'curve-style': 'bezier', 'opacity': 0.55,
      'label': 'data(label)', 'font-size': 8, 'color': '#484f58',
      'text-background-color': '#0d1117', 'text-background-opacity': 0.9, 'text-background-padding': '2px',
      'transition-property': 'opacity, line-color, width', 'transition-duration': '0.2s',
    }},
    { selector: '.ownership', style: { 'line-color': '#3fb95088', 'target-arrow-color': '#3fb95088' }},
    { selector: '.element', style: { 'line-color': '#da363388', 'target-arrow-color': '#da363388' }},
    { selector: '.wiring', style: { 'line-color': '#a371f788', 'target-arrow-color': '#a371f788' }},
    { selector: '.simulation', style: { 'line-color': '#d2992288', 'target-arrow-color': '#d2992288' }},
    { selector: '.io', style: { 'line-color': '#79c0ff88', 'target-arrow-color': '#79c0ff88' }},
    { selector: '.ui', style: { 'line-color': '#f778ba88', 'target-arrow-color': '#f778ba88' }},
    { selector: '.bwd', style: { 'line-color': '#7ee78788', 'target-arrow-color': '#7ee78788' }},
    { selector: '.codegen', style: { 'line-color': '#ffa65788', 'target-arrow-color': '#ffa65788' }},
    { selector: '.core', style: { 'line-color': '#58a6ff88', 'target-arrow-color': '#58a6ff88' }},
    { selector: '.component', style: { 'line-color': '#f0883e88', 'target-arrow-color': '#f0883e88', 'line-style': 'dashed' }},
    { selector: '.test', style: { 'line-color': '#8b949e66', 'target-arrow-color': '#8b949e66', 'line-style': 'dotted' }},
    { selector: '.external', style: { 'line-color': '#484f5866', 'target-arrow-color': '#484f5866', 'line-style': 'dotted' }},
    { selector: '.inherit', style: { 'line-color': '#da363366', 'target-arrow-color': '#da363366', 'target-arrow-shape': 'triangle', 'line-style': 'dashed' }},
    { selector: '.highlighted', style: { 'opacity': 1, 'border-width': 3, 'z-index': 999 }},
    { selector: '.highlighted-edge', style: { 'opacity': 1, 'width': 2.5, 'z-index': 998 }},
    { selector: '.faded', style: { 'opacity': 0.08 }},
    { selector: '.search-match', style: { 'border-color': '#ffa657', 'border-width': 3, 'background-color': '#ffa65733', 'z-index': 999 }},
  ],
  layout: { name: 'preset' }, // positions set below
});

// ── Manual compact layout — no fcose, no empty space ──────────

function runCompactLayout() {
  const cellW = 105;
  const cellH = 34;
  const modGap = 18;

  // Measure each module: grid its children, return width/height
  const modSizes = {};
  const modChildren = {};
  for (const mod of Object.keys(modules)) {
    const children = cy.getElementById(`mod_${mod}`).children().filter(n => n.visible());
    modChildren[mod] = children;
    const n = children.length;
    if (n === 0) { modSizes[mod] = { w: 0, h: 0, cols: 0, rows: 0 }; continue; }
    const cols = Math.ceil(Math.sqrt(n * 1.6)); // wider than tall
    const rows = Math.ceil(n / cols);
    modSizes[mod] = { w: cols * cellW, h: rows * cellH + 16, cols, rows };
  }

  // Arrange modules in rows using bin-packing (first-fit decreasing height)
  const modOrder = Object.keys(modules).filter(m => modSizes[m].w > 0);
  modOrder.sort((a, b) => modSizes[b].w * modSizes[b].h - modSizes[a].w * modSizes[a].h);

  const maxRowW = 1100;
  const modPositions = {};
  let curX = 0, curY = 0, rowH = 0;

  for (const mod of modOrder) {
    const sz = modSizes[mod];
    if (curX + sz.w > maxRowW && curX > 0) {
      curX = 0;
      curY += rowH + modGap;
      rowH = 0;
    }
    modPositions[mod] = { x: curX, y: curY };
    curX += sz.w + modGap;
    rowH = Math.max(rowH, sz.h);
  }

  // Position children in grid inside each module
  for (const mod of Object.keys(modules)) {
    const children = modChildren[mod];
    if (children.length === 0) continue;
    const sz = modSizes[mod];
    const mp = modPositions[mod];
    const sorted = children.sort((a, b) => b.connectedEdges().length - a.connectedEdges().length);

    sorted.forEach((node, i) => {
      const col = i % sz.cols;
      const row = Math.floor(i / sz.cols);
      node.position({
        x: mp.x + col * cellW + cellW / 2,
        y: mp.y + row * cellH + cellH / 2 + 16,
      });
    });
  }

  cy.fit(undefined, 30);
}

runCompactLayout();


// ── Filter pills ─────────────────────────────────────────────
const filtersEl = document.getElementById('filters');
const filterState = {};
for (const [mod, info] of Object.entries(modules)) {
  filterState[mod] = true;
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
    const modNode = cy.getElementById(`mod_${mod}`);
    const children = modNode.children();
    const connEdges = children.connectedEdges();
    if (filterState[mod]) {
      modNode.style('display', 'element');
      children.style('display', 'element');
      connEdges.forEach(e => {
        const sm = e.source().data('module'), tm = e.target().data('module');
        if (filterState[sm] && filterState[tm]) e.style('display', 'element');
      });
    } else {
      modNode.style('display', 'none');
      children.style('display', 'none');
      connEdges.style('display', 'none');
    }
  });
  filtersEl.appendChild(pill);
}


// ── Node interactions ────────────────────────────────────────
let highlightActive = false;

cy.on('tap', '.classNode', function(e) {
  const node = e.target;
  highlightActive = true;
  cy.elements().addClass('faded').removeClass('highlighted highlighted-edge');
  const neighborhood = node.neighborhood().add(node);
  neighborhood.nodes().removeClass('faded').addClass('highlighted');
  neighborhood.edges().removeClass('faded').addClass('highlighted-edge');
  neighborhood.nodes().forEach(n => { const p = n.parent(); if (p.length) p.removeClass('faded'); });
  showInfo(node);
});

// Double-click handlers are in flow-entry.js

// Double-click on class node → drill down to its module


cy.on('tap', function(e) { if (e.target === cy) resetHighlight(); });
cy.on('dbltap', function(e) {
  if (e.target === cy) { resetHighlight(); cy.animate({ fit: { padding: 60 }, duration: 400 }); }
});

function resetHighlight() {
  highlightActive = false;
  cy.elements().removeClass('faded highlighted highlighted-edge search-match');
  document.getElementById('info').style.display = 'none';
}


// ── Info panel ───────────────────────────────────────────────
function showInfo(node) {
  const info = document.getElementById('info');
  const mod = node.data('module');
  const nodeId = node.id();
  const color = modules[mod]?.color || '#8b949e';
  const incoming = node.incomers('edge').map(e => e.source());
  const outgoing = node.outgoers('edge').map(e => e.target());

  const depLinks = (arr, dir) => {
    if (!arr.length) return '';
    const unique = [...new Map(arr.map(n => [n.id(), n])).values()];
    return `<div class="deps">${dir}: ${unique.map(n => `<span onclick="focusNode('${n.id()}')">${n.data('label')}</span>`).join(', ')}</div>`;
  };

  const drillBtn = drillButtonHtml(nodeId, mod);

  info.innerHTML = `
    <h3>${node.data('label')}</h3>
    <div class="module-tag" style="background:${color}22;color:${color};border:1px solid ${color}44">${mod}</div>
    <div class="desc">${node.data('desc')}</div>
    ${depLinks(incoming, '← Used by')}
    ${depLinks(outgoing, '→ Depends on')}
    ${drillBtn}
  `;
  info.style.display = 'block';
}

window.focusNode = function(id) {
  const node = cy.getElementById(id);
  if (node.length) { cy.animate({ center: { eles: node }, zoom: 1.5, duration: 400 }); node.emit('tap'); }
};


// ── Search ───────────────────────────────────────────────────
const searchEl = document.getElementById('search');
searchEl.addEventListener('input', () => {
  const q = searchEl.value.trim().toLowerCase();
  cy.elements().removeClass('search-match faded highlighted highlighted-edge');
  if (!q) { document.getElementById('info').style.display = 'none'; return; }
  const matches = cy.nodes('.classNode').filter(n =>
    n.data('label').toLowerCase().includes(q) || n.data('desc').toLowerCase().includes(q)
  );
  if (matches.length > 0 && matches.length < 20) {
    cy.elements().addClass('faded');
    matches.forEach(m => { m.removeClass('faded').addClass('search-match'); const p = m.parent(); if (p.length) p.removeClass('faded'); });
    if (matches.length === 1) { cy.animate({ center: { eles: matches[0] }, duration: 300 }); showInfo(matches[0]); }
  } else { matches.addClass('search-match'); }
});


// ── Keyboard ─────────────────────────────────────────────────
document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') {
    if (document.getElementById('drilldown').classList.contains('visible')) {
      closeOverlay();
    } else {
      searchEl.value = ''; searchEl.blur(); resetHighlight();
    }
  }
  if (e.key === '/' && document.activeElement !== searchEl) { e.preventDefault(); searchEl.focus(); }
});


// ── Stats ────────────────────────────────────────────────────
document.getElementById('stats').textContent =
  `${cy.nodes('.classNode').length} classes · ${cy.edges().length} connections · ${Object.keys(flowRegistry).length} drill-downs`;

cy.ready(() => cy.fit(undefined, 60));


setupFlowEntryPoints(cy);

document.getElementById('drill-back').addEventListener('click', goBack);
