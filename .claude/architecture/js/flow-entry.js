// Entry points: connect main architecture graph to flow registry

// Map from architecture graph node ID to flow registry entry
function flowIdForNode(nodeId, moduleName) {
  if (flowRegistry[nodeId + '_ops']) return nodeId + '_ops';
  if (flowRegistry[moduleName.toLowerCase() + '_mod']) return moduleName.toLowerCase() + '_mod';
  return null;
}

// Called from graph-setup.js after cy is created
function setupFlowEntryPoints(cy) {
  // Double-click class node -> open its flow
  cy.on('dbltap', '.classNode', function(e) {
    const nodeId = e.target.id();
    const mod = e.target.data('module');
    const fid = flowIdForNode(nodeId, mod);
    if (fid) {
      flowHistory = [];
      openFlow(fid);
    }
  });

  // Double-click module compound -> open module flow
  cy.on('dbltap', '.module', function(e) {
    const mod = e.target.data('module');
    const fid = flowIdForNode('', mod);
    if (fid) {
      flowHistory = [];
      openFlow(fid);
    }
  });
}

// Build drill button HTML for the info panel
function drillButtonHtml(nodeId, moduleName) {
  const fid = flowIdForNode(nodeId, moduleName);
  if (!fid) return '';
  const flow = flowRegistry[fid];
  const label = flow ? flow.title : fid;
  return `<button class="drill-btn" onclick="flowHistory=[];openFlow('${fid}')">${label} \u2192</button>`;
}
