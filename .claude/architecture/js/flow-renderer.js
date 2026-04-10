// Flow rendering engine with hierarchical navigation
let flowCy = null;
let flowHistory = [];   // [{flowId, title}]
let currentFlowId = null;

function openFlow(flowId) {
  const flow = flowRegistry[flowId];
  if (!flow) { console.warn('Flow not found:', flowId); return; }

  // Show the overlay if not already visible
  document.getElementById('drilldown').classList.add('visible');

  currentFlowId = flowId;
  updateBreadcrumbs();
  renderFlow(flowId);
  updateHash();
}

function drillInto(flowId) {
  if (!currentFlowId) return;
  const current = flowRegistry[currentFlowId];
  flowHistory.push({ flowId: currentFlowId, title: current ? current.title : currentFlowId });
  openFlow(flowId);
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
  // Breadcrumb[index] corresponds to flowHistory[index].
  // Navigate to that flow and keep history up to (but not including) it.
  const target = flowHistory[index];
  if (!target) return;
  flowHistory = flowHistory.slice(0, index);
  openFlow(target.flowId);
}

function closeOverlay() {
  document.getElementById('drilldown').classList.remove('visible');
  if (flowCy) { flowCy.destroy(); flowCy = null; }
  flowHistory = [];
  currentFlowId = null;
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

function renderFlow(flowId) {
  const flow = flowRegistry[flowId];
  if (!flow) return;

  if (flowCy) { flowCy.destroy(); flowCy = null; }

  const container = document.getElementById('flow-cy');
  if (!container) return;

  document.getElementById('flow-info').style.display = 'none';

  const elements = [];
  for (const n of flow.nodes) {
    const hasDrill = !!(n[4]);
    elements.push({
      data: { id: n[0], label: n[1], nodeType: n[2] || 'step', desc: n[3] || '', drillTarget: n[4] || '' },
      classes: (n[2] || 'step') + (hasDrill ? ' drillable' : '')
    });
  }
  for (let i = 0; i < flow.edges.length; i++) {
    const e = flow.edges[i];
    elements.push({
      data: { id: `fe_${i}`, source: e[0], target: e[1], label: e[2] || '' }
    });
  }

  flowCy = cytoscape({
    container,
    elements,
    style: [
      { selector: 'node', style: {
        'shape': 'round-rectangle', 'padding': '10px',
        'width': function(ele) {
          var lines = ele.data('label').split('\n');
          var maxW = 0;
          for (var i = 0; i < lines.length; i++) { maxW = Math.max(maxW, measureLabel(lines[i], 11)); }
          return Math.min(Math.max(maxW + 24, 60), 220);
        },
        'height': function(ele) {
          var lines = ele.data('label').split('\n');
          return Math.max(lines.length * 16 + 20, 36);
        },
        'background-color': '#161b22', 'border-width': 2, 'border-color': '#30363d',
        'label': 'data(label)', 'font-size': 11, 'color': '#c9d1d9',
        'text-valign': 'center', 'text-halign': 'center',
        'text-wrap': 'wrap', 'text-max-width': 200,
      }},
      { selector: '.start', style: {
        'background-color': '#238636', 'border-color': '#2ea043',
        'color': '#ffffff', 'font-weight': 'bold',
      }},
      { selector: '.decision', style: {
        'shape': 'diamond', 'background-color': '#0d1117',
        'border-color': '#a371f7', 'border-width': 2.5,
        'padding': '16px', 'text-max-width': 140, 'font-size': 10,
      }},
      { selector: '.key', style: {}},
      { selector: '.error', style: {
        'border-color': '#da3633', 'border-width': 2,
        'background-color': '#da3633', 'background-opacity': 0.08, 'color': '#f85149', 'font-size': 10,
      }},
      { selector: '.end', style: { 'border-color': '#3fb950', 'border-width': 2.5 }},
      // Drillable nodes: gold border (clickable)
      { selector: '.drillable', style: {
        'border-color': '#d29922', 'border-width': 2.5,
      }},
      { selector: 'edge', style: {
        'width': 1.5, 'line-color': '#484f58', 'target-arrow-color': '#484f58',
        'target-arrow-shape': 'triangle', 'arrow-scale': 1.0,
        'curve-style': 'taxi', 'taxi-direction': 'downward', 'taxi-turn': '20px',
        'label': 'data(label)', 'font-size': 9, 'color': '#7d8590',
        'text-background-color': '#0d1117', 'text-background-opacity': 0.9,
        'text-background-padding': '3px', 'text-rotation': 'autorotate',
      }},
      { selector: 'edge[label *= "Yes"]', style: {
        'line-color': 'rgba(63, 185, 80, 0.47)', 'target-arrow-color': '#3fb950', 'color': '#3fb950',
      }},
      { selector: 'edge[label *= "No"]', style: {
        'line-color': 'rgba(218, 54, 51, 0.47)', 'target-arrow-color': '#da3633', 'color': '#f85149',
      }},
      { selector: '.seg-left', style: {
        'curve-style': 'segments',
        'edge-distances': 'endpoints',
        'source-endpoint': '-50% 0%',
        'segment-weights': [0],
        'segment-distances': [-30],
      }},
      { selector: '.seg-right', style: {
        'curve-style': 'segments',
        'edge-distances': 'endpoints',
        'source-endpoint': '50% 0%',
        'segment-weights': [0],
        'segment-distances': [30],
      }},
      { selector: '.expanded', style: {
        'border-color': '#58a6ff', 'border-width': 3, 'border-style': 'dashed',
        'background-color': 'rgba(88, 166, 255, 0.08)',
      }},
      { selector: '.fc-highlight', style: { 'border-width': 3.5, 'z-index': 999 }},
      { selector: '.fc-fade', style: { 'opacity': 0.25 }},
    ],
    layout: {
      name: 'dagre',
      rankDir: 'TB',
      nodeSep: 50,
      rankSep: 55,
      edgeSep: 20,
      ranker: 'longest-path',
    },
    minZoom: 0.2, maxZoom: 2.5, wheelSensitivity: 0.3,
  });

  // Single click: show description + highlight neighbourhood
  flowCy.on('tap', 'node', function(e) {
    const node = e.target;
    flowCy.elements().removeClass('fc-highlight fc-fade');
    const hood = node.neighborhood().add(node);
    flowCy.elements().not(hood).addClass('fc-fade');
    hood.addClass('fc-highlight');
    const desc = node.data('desc');
    const drill = node.data('drillTarget');
    const info = document.getElementById('flow-info');
    let html = '<strong>' + node.data('label').replace(/\n/g, ' ') + '</strong>';
    if (desc) html += '<br>' + desc;
    if (drill) html += '<br><em style="color:#58a6ff">Double-click to drill into \u2192 ' + (flowRegistry[drill]?.title || drill) + '</em>';
    info.innerHTML = html;
    info.style.display = 'block';
  });

  // Double click: expand sub-flow inline
  flowCy.on('dbltap', 'node', function(e) {
    const drill = e.target.data('drillTarget');
    if (drill && flowRegistry[drill]) {
      expandInline(e.target.id());
    }
  });

  // Click background: clear highlight
  flowCy.on('tap', function(e) {
    if (e.target === flowCy) {
      flowCy.elements().removeClass('fc-highlight fc-fade');
      document.getElementById('flow-info').style.display = 'none';
    }
  });

  flowCy.ready(() => flowCy.fit(undefined, 40));
}

// ── Inline expansion ────────────────────────────────────────
const _expansions = {};  // nodeId → { addedIds, removedEdges }

function expandInline(nodeId) {
  if (!flowCy) return;

  // Toggle: collapse if already expanded
  if (_expansions[nodeId]) { collapseInline(nodeId); return; }

  const node = flowCy.getElementById(nodeId);
  if (!node.length) return;

  const drillTarget = node.data('drillTarget');
  const flow = flowRegistry[drillTarget];
  if (!flow) return;

  const pfx = nodeId + '__';

  // Save and remove outgoing edges (incoming stay on the original node)
  const removedEdges = [];
  node.outgoers('edge').forEach(e => {
    removedEdges.push({ id: e.id(), src: e.source().id(), tgt: e.target().id(), label: e.data('label') || '' });
  });
  flowCy.remove(node.outgoers('edge'));

  // Find start and terminal nodes in sub-flow
  const intSources = new Set(flow.edges.map(e => e[0]));
  const starts = flow.nodes.filter(n => n[2] === 'start').map(n => pfx + n[0]);
  const ends = flow.nodes.filter(n => !intSources.has(n[0])).map(n => pfx + n[0]);

  const addedIds = [];
  const toAdd = [];

  // Add sub-flow nodes
  for (const n of flow.nodes) {
    const id = pfx + n[0];
    const hasDrill = !!(n[4]);
    toAdd.push({
      group: 'nodes',
      data: { id, label: n[1], nodeType: n[2] || 'step', desc: n[3] || '', drillTarget: n[4] || '' },
      classes: (n[2] || 'step') + (hasDrill ? ' drillable' : '')
    });
    addedIds.push(id);
  }

  // Add sub-flow internal edges
  for (let i = 0; i < flow.edges.length; i++) {
    const e = flow.edges[i];
    const id = pfx + 'e' + i;
    toAdd.push({ group: 'edges', data: { id, source: pfx + e[0], target: pfx + e[1], label: e[2] || '' } });
    addedIds.push(id);
  }

  // Bridge: original node → sub-flow start nodes
  let ei = flow.edges.length;
  for (const s of starts) {
    const id = pfx + 'e' + (ei++);
    toAdd.push({ group: 'edges', data: { id, source: nodeId, target: s, label: '' } });
    addedIds.push(id);
  }

  // Bridge: sub-flow end nodes → original node's former outgoing targets
  for (const re of removedEdges) {
    for (const en of ends) {
      const id = pfx + 'e' + (ei++);
      toAdd.push({ group: 'edges', data: { id, source: en, target: re.tgt, label: re.label } });
      addedIds.push(id);
    }
  }

  // Add sub-flow elements to the real graph (initially at origin)
  flowCy.add(toAdd);

  // Layout only the sub-flow nodes using the real rendered sizes
  const subNodes = addedIds.filter(id => flowCy.getElementById(id).isNode());
  const subEls = flowCy.collection();
  for (const id of addedIds) {
    subEls.merge(flowCy.getElementById(id));
  }
  subEls.layout({ name: 'dagre', rankDir: 'TB', nodeSep: 50, rankSep: 55, edgeSep: 20, ranker: 'longest-path', fit: false }).run();

  // Measure sub-flow bounding box and reposition below expanded node
  const subBB = subEls.nodes().boundingBox();
  const nodePos = node.position();
  const offsetX = nodePos.x - (subBB.x1 + subBB.w / 2);
  const offsetY = nodePos.y + 60 - subBB.y1;
  const subHeight = subBB.h + 60;
  subEls.nodes().forEach(n => {
    const p = n.position();
    n.position({ x: p.x + offsetX, y: p.y + offsetY });
  });

  // Shift all existing nodes below the expanded node down
  const threshold = nodePos.y + 20;
  flowCy.nodes().forEach(n => {
    if (n.id() !== nodeId && !addedIds.includes(n.id()) && n.position().y > threshold) {
      n.animate({ position: { x: n.position().x, y: n.position().y + subHeight } }, { duration: 300 });
    }
  });

  node.addClass('expanded');
  _expansions[nodeId] = { addedIds, removedEdges, subHeight, threshold };
}

function collapseInline(nodeId) {
  if (!flowCy || !_expansions[nodeId]) return;
  const { addedIds, removedEdges, subHeight, threshold } = _expansions[nodeId];

  // Remove all added elements
  for (const id of addedIds) {
    const el = flowCy.getElementById(id);
    if (el.length) el.remove();
  }

  // Restore original outgoing edges
  for (const re of removedEdges) {
    flowCy.add({ group: 'edges', data: { id: re.id, source: re.src, target: re.tgt, label: re.label } });
  }

  // Shift nodes back up
  flowCy.nodes().forEach(n => {
    if (n.id() !== nodeId && n.position().y > threshold + subHeight / 2) {
      n.animate({ position: { x: n.position().x, y: n.position().y - subHeight } }, { duration: 300 });
    }
  });

  flowCy.getElementById(nodeId).removeClass('expanded');
  delete _expansions[nodeId];
}

// ── URL hash persistence + browser history ──────────────────
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
  if (flowCy) { flowCy.destroy(); flowCy = null; }
  flowHistory = [];
  currentFlowId = null;
}

window.addEventListener('popstate', () => { restoreFromHash(); });
