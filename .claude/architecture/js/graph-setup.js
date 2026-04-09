// Main Cytoscape graph: setup, layout, interactions, search

// Convert hex color + alpha (0-255) to rgba() string for Cytoscape
function hexAlpha(hex, alpha) {
  const r = parseInt(hex.slice(1,3), 16);
  const g = parseInt(hex.slice(3,5), 16);
  const b = parseInt(hex.slice(5,7), 16);
  return 'rgba(' + r + ',' + g + ',' + b + ',' + (alpha/255).toFixed(2) + ')';
}
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
  minZoom: 0.08, maxZoom: 3,
  style: [
    { selector: '.module', style: {
      'shape': 'round-rectangle', 'background-color': '#161b22', 'border-width': 2,
      'border-color': '#30363d', 'padding': '3px', 'text-valign': 'top', 'text-halign': 'center',
      'label': 'data(label)', 'font-size': 11, 'font-weight': 'bold', 'color': '#484f58',
      'text-margin-y': 2, 'compound-sizing-wrt-labels': 'include',
    }},
    ...Object.entries(modules).map(([mod, info]) => ({
      selector: `#mod_${mod}`, style: { 'border-color': hexAlpha(info.color, 0x88), 'color': info.color }
    })),
    { selector: '.classNode', style: {
      'shape': 'round-rectangle', 'width': 120, 'height': 30, 'padding': '6px',
      'background-color': '#21262d', 'border-width': 1.5, 'border-color': '#30363d',
      'label': 'data(label)', 'font-size': 10, 'color': '#e6edf3',
      'text-valign': 'center', 'text-halign': 'center', 'text-wrap': 'wrap', 'text-max-width': 100,
      'transition-property': 'background-color, border-color, border-width, opacity', 'transition-duration': '0.2s',
    }},
    ...Object.entries(modules).map(([mod, info]) => ({
      selector: `.classNode[module="${mod}"]`, style: { 'border-color': hexAlpha(info.color, 0x99) }
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
    { selector: '.ownership', style: { 'line-color': 'rgba(63, 185, 80, 0.53)', 'target-arrow-color': 'rgba(63, 185, 80, 0.53)' }},
    { selector: '.element', style: { 'line-color': 'rgba(218, 54, 51, 0.53)', 'target-arrow-color': 'rgba(218, 54, 51, 0.53)' }},
    { selector: '.wiring', style: { 'line-color': 'rgba(163, 113, 247, 0.53)', 'target-arrow-color': 'rgba(163, 113, 247, 0.53)' }},
    { selector: '.simulation', style: { 'line-color': 'rgba(210, 153, 34, 0.53)', 'target-arrow-color': 'rgba(210, 153, 34, 0.53)' }},
    { selector: '.io', style: { 'line-color': 'rgba(121, 192, 255, 0.53)', 'target-arrow-color': 'rgba(121, 192, 255, 0.53)' }},
    { selector: '.ui', style: { 'line-color': 'rgba(247, 120, 186, 0.53)', 'target-arrow-color': 'rgba(247, 120, 186, 0.53)' }},
    { selector: '.bwd', style: { 'line-color': 'rgba(126, 231, 135, 0.53)', 'target-arrow-color': 'rgba(126, 231, 135, 0.53)' }},
    { selector: '.codegen', style: { 'line-color': 'rgba(255, 166, 87, 0.53)', 'target-arrow-color': 'rgba(255, 166, 87, 0.53)' }},
    { selector: '.core', style: { 'line-color': 'rgba(88, 166, 255, 0.53)', 'target-arrow-color': 'rgba(88, 166, 255, 0.53)' }},
    { selector: '.component', style: { 'line-color': 'rgba(240, 136, 62, 0.53)', 'target-arrow-color': 'rgba(240, 136, 62, 0.53)', 'line-style': 'dashed' }},
    { selector: '.test', style: { 'line-color': 'rgba(139, 148, 158, 0.4)', 'target-arrow-color': 'rgba(139, 148, 158, 0.4)', 'line-style': 'dotted' }},
    { selector: '.external', style: { 'line-color': 'rgba(72, 79, 88, 0.4)', 'target-arrow-color': 'rgba(72, 79, 88, 0.4)', 'line-style': 'dotted' }},
    { selector: '.inherit', style: { 'line-color': 'rgba(218, 54, 51, 0.4)', 'target-arrow-color': 'rgba(218, 54, 51, 0.4)', 'target-arrow-shape': 'triangle', 'line-style': 'dashed' }},
    { selector: '.highlighted', style: { 'opacity': 1, 'border-width': 3, 'z-index': 999 }},
    { selector: '.highlighted-edge', style: { 'opacity': 1, 'width': 2.5, 'z-index': 998 }},
    { selector: '.faded', style: { 'opacity': 0.08 }},
    { selector: '.search-match', style: { 'border-color': '#ffa657', 'border-width': 3, 'background-color': 'rgba(255, 166, 87, 0.2)', 'z-index': 999 }},
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
