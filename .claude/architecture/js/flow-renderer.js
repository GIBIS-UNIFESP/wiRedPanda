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
  // Pop history to the given index
  while (flowHistory.length > index) {
    flowHistory.pop();
  }
  if (flowHistory.length === 0 && index === 0) {
    // User clicked the first crumb — nothing in history, just re-render current root
    return;
  }
  const target = flowHistory.pop();
  if (target) {
    openFlow(target.flowId);
  }
}

function closeOverlay() {
  document.getElementById('drilldown').classList.remove('visible');
  if (flowCy) { flowCy.destroy(); flowCy = null; }
  flowHistory = [];
  currentFlowId = null;
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
      { selector: '.key', style: { 'border-color': '#d29922', 'border-width': 2.5 }},
      { selector: '.error', style: {
        'border-color': '#da3633', 'border-width': 2,
        'background-color': 'rgba(218, 54, 51, 0.08)', 'color': '#f85149', 'font-size': 10,
      }},
      { selector: '.end', style: { 'border-color': '#3fb950', 'border-width': 2.5 }},
      // Drillable nodes: double border + glow
      { selector: '.drillable', style: {
        'border-style': 'double', 'border-width': 5,
        'underlay-color': '#58a6ff', 'underlay-padding': 3, 'underlay-opacity': 0.08,
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
    wheelSensitivity: 0.3, minZoom: 0.2, maxZoom: 2.5,
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

  // Double click: drill into sub-flow
  flowCy.on('dbltap', 'node', function(e) {
    const drill = e.target.data('drillTarget');
    if (drill && flowRegistry[drill]) {
      drillInto(drill);
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
