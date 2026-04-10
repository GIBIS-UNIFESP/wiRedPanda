// Entry points: connect main architecture graph to flow registry

// Map from architecture graph node ID to flow registry entry
const nodeFlowMap = {
  'mainwindow': 'mw_ops',
  'workspace': 'ws_ops',
  'scene': 'scene_ops',
  'commands': 'cmd_ops',
  'conn_mgr': 'conn_ops',
  'graphic_elem': 'ge_ops',
  'elem_factory': 'ef_build',
  'elem_editor': 'editor_apply',
  'simulation': 'sim_ops',
  'serialization': 'ser_ops',
  'ic': 'ic_ops',
  'ic_registry': 'icreg_embed',
  'bwd': 'bwd_ops',
  'arduino_gen': 'cg_arduino',
  'sv_gen': 'cg_verilog',
  'gfx_view': 'ui_zoom',
  'elem_palette': 'ui_palette',
  'elem_ctx_menu': 'ui_menu_dispatch',
  'theme_mgr': 'ui_theme',
  'clipboard': 'scene_clipboard',
  'vis_mgr': 'scene_ops',
  'prop_shortcut': 'scene_ops',
  'sim_blocker': 'sim_ops',
  'qne_port': 'nodes_ports',
  'qne_conn': 'conn_ops',
  'graphic_input': 'ge_ops',
  'ic_def': 'ic_ops',
  'elem_info': 'ef_build',
  'elem_metadata': 'element_hierarchy',
  'elem_label': 'ge_ops',
  'prop_desc': 'editor_apply',
  'serial_ctx': 'ser_deserialize',
  'file_utils': 'ser_ops',
  'recent_files': 'mw_ops',
  'version_info': 'ser_header',
  'mainwindow_ui': 'mw_ops',
  'elem_editor_ui': 'editor_selection',
  'tab_nav': 'ui_palette',
  'sel_caps': 'scene_ops',
  'clock_dlg': 'mw_ops',
  'length_dlg': 'mw_ops',
  'circuit_exp': 'mw_ops',
  'ic_dropzone': 'mw_embed_dz',
  'lang_mgr': 'ui_ops',
  'trash_btn': 'mw_remove_embedded',
  'labeled_slider': 'ui_ops',
  'bwd_ui': 'bwd_ops',
  'bwd_serial': 'bwd_save_load',
  'bwd_model': 'bwd_run',
  'bwd_delegate': 'bwd_ops',
  'bwd_waveform': 'bwd_ops',
  'grp_gate': 'components_registry',
  'grp_input': 'components_registry',
  'grp_output': 'components_registry',
  'grp_memory': 'components_registry',
  'grp_mux': 'components_registry',
  'grp_static': 'components_registry',
  'grp_other': 'components_registry',
  'grp_ic': 'ic_ops',
  'application': 'core_services',
  'settings': 'core_services',
  'common': 'core_services',
  'enums': 'core_services',
  'theme_mgr': 'ui_theme',
  'sys_theme': 'ui_theme',
  'item_with_id': 'core_services',
  'priorities': 'sim_topo_sort',
  'sentry': 'core_services',
  'update_check': 'core_services',
  'status_ops': 'core_services',
  'main': 'overview',
  'test_unit': 'test_unit_ops',
  'test_integ': 'test_integ_ops',
  'test_system': 'test_system_ops',
  'test_compat': 'test_compat_ops',
};

function flowIdForNode(nodeId, moduleName) {
  if (nodeFlowMap[nodeId]) return nodeFlowMap[nodeId];
  const modKey = moduleName.toLowerCase() + '_mod';
  if (flowRegistry[modKey]) return modKey;
  return null;
}

// Called from graph-view.js after SVG canvas is created
// Events are wired directly in graph-view.js on SVG node elements,
// so this is a no-op now. Kept for compatibility.
function setupFlowEntryPoints() {
  // Node double-click → openFlow is handled directly in graph-view.js
}

function drillButtonHtml(nodeId, moduleName) {
  const fid = flowIdForNode(nodeId, moduleName);
  if (!fid) return '';
  const flow = flowRegistry[fid];
  const label = flow ? flow.title : fid;
  return `<button class="drill-btn" onclick="flowHistory=[];openFlow('${fid}')">${label} \u2192</button>`;
}
