# Cytoscape Replacement Plan

Replace Cytoscape.js with a custom SVG renderer. 1:1 functional replacement — same behavior, same appearance, less overhead.

## What We Drop

| Library | Size | Why unused |
|---------|------|------------|
| cytoscape@3.28.1 | ~500KB | Full graph engine — we use <10% of features |
| layout-base@2.0.1 | ~20KB | fcose dependency — not used (layout is manual) |
| cose-base@2.2.0 | ~15KB | fcose dependency — not used |
| cytoscape-fcose@2.2.0 | ~25KB | Never called — layout is manual compact grid |
| cytoscape-dagre@2.5.0 | ~5KB | Thin wrapper — we'll call dagre directly |
| **Total removed** | **~565KB** | |

## What We Keep

| Library | Size | Why |
|---------|------|-----|
| dagre@0.8.5 | ~30KB | Layout algorithm for flow views. No replacement needed. |

## Coverage Analysis (2026-04-09)

Instrumented the unminified `cytoscape.umd.js` with a method-level probe. After exercising all features (click, double-click, search, filter, zoom, pan, drill down, inline expand, collapse, breadcrumbs, keyboard shortcuts), ran `coverageReport()`.

### Summary

- **174 methods called** out of hundreds available in Cytoscape
- Top 10 methods by call count are all internal machinery (style cascade, compound bounds, dirty checks) — millions of calls each
- Only **~35 methods are called from our code** — the other 139 are Cytoscape calling itself internally

### Methods Called From Our Code

**graph-setup.js (main graph):**

| Method | Purpose |
|--------|---------|
| `cytoscape()` | Constructor |
| `cy.getElementById()` | Find node by ID |
| `cy.nodes()`, `cy.edges()`, `cy.elements()`, `cy.$()` | Query elements |
| `cy.on()` | Bind click/double-click events |
| `cy.fit()`, `cy.animate()` | Viewport control |
| `cy.ready()` | Init callback |
| `collection.position()` | Get/set node positions |
| `collection.data()` | Read node data (label, desc, module) |
| `collection.addClass()`, `collection.removeClass()` | Visual state (faded, highlighted, search-match) |
| `collection.style()` | Show/hide modules (display: none/element) |
| `collection.filter()`, `collection.visible()` | Filter nodes |
| `collection.children()`, `collection.connectedEdges()` | Module children, connected edges |
| `collection.neighborhood()`, `collection.incomers()`, `collection.outgoers()` | Dependency traversal |
| `collection.id()`, `collection.parent()` | Identity |
| `collection.boundingBox()` | Measurement for viewport stretch |
| `collection.sort()` | Sort by edge count |

**flow-renderer.js (flow view):**

| Method | Purpose |
|--------|---------|
| `cytoscape()` | Constructor with dagre layout |
| `cy.add()` | Add inline expansion elements |
| `collection.remove()` | Remove elements on collapse |
| `collection.layout()` | Run dagre on sub-flow subset |
| `collection.animate()` | Shift nodes during expand/collapse |
| `collection.merge()` | Build collection from IDs |
| `collection.isNode()` | Type check during expansion |
| `collection.not()` | Set difference for fade effect |
| `collection.map()` | Iteration |
| `cy.destroy()` | Cleanup on flow change |

### Internal Methods (called by Cytoscape, not by us)

The top 20 by call count — all internal overhead:

| Method | Calls | Purpose |
|--------|-------|---------|
| `collection.cy` | 7,072,223 | Internal: get parent graph reference |
| `cy.styleEnabled` | 6,571,366 | Internal: check if styles active |
| `collection.cleanStyle` | 3,022,910 | Internal: style cascade |
| `collection.pstyle` | 2,997,030 | Internal: parsed style lookup |
| `collection.group` | 1,543,535 | Internal: node vs edge check |
| `cy.style` | 1,510,609 | Internal: style object access |
| `collection.isNode` | 950,035 | Internal: type check (also used by us, 51 times) |
| `collection.isParent` | 632,179 | Internal: compound node check |
| `collection.isEdge` | 565,925 | Internal: type check |
| `cy.hasCompoundNodes` | 515,648 | Internal: compound mode check |
| `collection.updateCompoundBounds` | 450,748 | Internal: recalc module boundaries |
| `collection.position` | 402,131 | Internal: position reads (also used by us) |
| `collection.id` | 370,588 | Internal: ID reads (also used by us) |
| `cy.batching` | 365,051 | Internal: batch mode check |
| `collection.target` | 326,610 | Internal: edge target resolution |
| `collection.source` | 282,481 | Internal: edge source resolution |
| `collection.padding` | 259,620 | Internal: compound padding calc |
| `collection.spawn` | 258,069 | Internal: create sub-collections |
| `collection.hasClass` | 253,481 | Internal: class checks for styling |
| `collection.filter` | 241,674 | Internal: style selector matching |

### Conclusion

We call ~35 methods. Cytoscape runs 174 methods and makes ~30 million internal calls to service those 35. The style cascade alone (`cleanStyle` + `pstyle` + `styleEnabled`) accounts for 12.5 million calls — all to maintain a CSS-like style system we could replace with direct SVG attribute setting.

## New Files

### `js/svg-engine.js` (~120 lines)

Shared SVG utilities used by both views.

- `svgEl(tag, attrs)` — create namespaced SVG element
- `measureText(text, fontSize)` — canvas-based text measurement (port existing `measureLabel`)
- `createSvgCanvas(container)` — returns `{svg, defs, root, fit(), destroy()}`
  - Creates `<svg>` with `viewBox` for coordinate-space zoom
  - Wheel zoom (toward cursor, respects viewBox)
  - Mouse drag pan (on background only)
  - `fit(bounds, padding)` — adjust viewBox to show given bounds, preserve aspect ratio
- `addArrowMarker(defs, id, color)` — add `<marker>` for edge arrowheads
- `drawEdgePath(points)` — convert dagre point array to SVG `d` attribute

### `js/graph-view.js` (~350 lines)

Replaces `graph-setup.js`. Same structure, SVG instead of Cytoscape.

**Data build:** Same loop over `modules`, `nodes`, `edges` — build lookup maps instead of Cytoscape elements.

**Layout (`runCompactLayout`):** Port existing logic. Instead of `node.position()`, store positions in a plain object `nodePositions[id] = {x, y}`. Same compact grid + viewport stretch.

**Rendering:** After layout, create SVG elements:
- Module groups: `<g>` with `<rect>` background (colored border, dark fill) + `<text>` label
- Class nodes: `<rect rx="4">` + `<text>` (centered). Width from `measureText`. Colored border per module.
- Edges: `<path>` with bezier curves between node centers. Colored by type. Arrow markers. Dashed/dotted via `stroke-dasharray`.
- Key nodes (graphic_elem, scene, etc.): thicker border, bold text.

**Interactions:**
- Click node → add `.faded` class to all, remove from neighborhood. Show info panel. (Use SVG `opacity` attribute or CSS classes on `<g>` elements.)
- Double-click node → `openFlow(flowId)` (same as current `flow-entry.js` wiring)
- Double-click background → `fit()`
- Click background → reset highlight
- Filter pills → toggle `display:none` on module `<g>` groups + their edges
- Search → match against node data, add `.search-match` class
- `focusNode(id)` → animate viewBox to center on node

**Info panel, stats, keyboard shortcuts:** Identical to current — these are DOM operations, not Cytoscape-specific.

### `js/flow-view.js` (~350 lines)

Replaces `flow-renderer.js`. Same navigation logic, SVG + dagre instead of Cytoscape.

**`renderFlow(flowId)`:**
1. Build dagre graph: `g.setNode(id, {width, height})` with dimensions from `measureText`
2. `dagre.layout(g)` → get node positions + edge point arrays
3. Create SVG elements:
   - Nodes: `<rect rx="4">` for most types, `<polygon>` rotated square for `decision` diamonds
   - Styling by type: start=green fill, decision=purple border, error=red border, end=green border, drillable=gold border, expanded=blue dashed
   - Edge paths from dagre points with arrow markers. Yes=green, No=red, default=gray.
4. Event handlers on SVG elements (native DOM events — no cytoscape abstraction)

**Click/double-click:** Same behavior — click highlights neighborhood, double-click expands inline.

**Inline expansion (`expandInline`/`collapseInline`):**
1. Run dagre on sub-flow nodes (build separate dagre graph, layout, read positions)
2. Create SVG elements at computed positions (offset below expanded node)
3. Shift downstream SVG elements by adjusting `transform` attributes
4. Collapse: remove added SVG elements, shift back, restore edges
- Much simpler than current approach — no "add to cytoscape instance + re-layout subset" hack

**Navigation, breadcrumbs, URL hash:** Identical — these are pure JS/DOM, not Cytoscape-specific. Port as-is.

## Files Modified

### `index.html`

Remove 6 script tags (cytoscape + plugins). Keep dagre. Add `svg-engine.js`. Replace `graph-setup.js` → `graph-view.js`, `flow-renderer.js` → `flow-view.js`.

```html
<!-- Before -->
<script src="https://unpkg.com/cytoscape@3.28.1/dist/cytoscape.min.js"></script>
<script src="https://unpkg.com/layout-base@2.0.1/layout-base.js"></script>
<script src="https://unpkg.com/cose-base@2.2.0/cose-base.js"></script>
<script src="https://unpkg.com/cytoscape-fcose@2.2.0/cytoscape-fcose.js"></script>
<script src="https://unpkg.com/dagre@0.8.5/dist/dagre.min.js"></script>
<script src="https://unpkg.com/cytoscape-dagre@2.5.0/cytoscape-dagre.js"></script>

<!-- After -->
<script src="https://unpkg.com/dagre@0.8.5/dist/dagre.min.js"></script>
```

### CSS changes

Minimal. The `#cy` and `#flow-cy` containers now hold `<svg>` instead of Cytoscape's `<canvas>`. Most CSS (topbar, info panel, drilldown overlay, breadcrumbs) is unchanged. Add:

```css
#cy svg, #flow-cy svg { width: 100%; height: 100%; }
```

Node/edge styling moves from Cytoscape style objects into SVG attributes set during rendering (fill, stroke, stroke-width, opacity, etc.). No external CSS needed for graph elements.

## Files Unchanged

- `js/graph-data.js` — node/edge data arrays
- `js/flow-registry.js` — flow registry object
- `js/flow-entry.js` — node-to-flow mapping + `setupFlowEntryPoints()` (minor: receives SVG click events instead of cytoscape tap events)
- `flows/*.js` — all 99 flow definitions
- `css/drilldown.css` — overlay/breadcrumb styling (unchanged)

## Migration Approach

1. Write `svg-engine.js` (shared utilities)
2. Write `graph-view.js` (main graph — replaces `graph-setup.js`)
3. Write `flow-view.js` (flow charts — replaces `flow-renderer.js`)
4. Update `index.html` (swap scripts)
5. Adapt `flow-entry.js` (wire SVG click events instead of cytoscape events)
6. Test all views, verify 1:1 behavior
7. Delete old `graph-setup.js` and `flow-renderer.js`

## Edge Rendering Details

**Main graph edges:** Straight lines between node centers (current bezier adds complexity with little visual benefit at this scale). Each edge type gets its own arrow marker color. Dashed/dotted via `stroke-dasharray`.

**Flow edges:** Dagre provides control points. Render as polyline through those points (matches the current taxi/stepped look). Arrow at the last segment.

**Edge labels:** SVG `<text>` positioned at edge midpoint with a dark `<rect>` background (same as current text-background-color).

## Interaction Parity Checklist

- [ ] Main graph: zoom (wheel toward cursor)
- [ ] Main graph: pan (drag background)
- [ ] Main graph: click node → highlight neighborhood, show info
- [ ] Main graph: double-click node → open flow drilldown
- [ ] Main graph: double-click background → fit to viewport
- [ ] Main graph: click background → reset highlights
- [ ] Main graph: filter pills → show/hide modules
- [ ] Main graph: search → highlight matches, fade others
- [ ] Main graph: focusNode() → animate to node
- [ ] Main graph: Escape → clear search/highlights
- [ ] Main graph: / → focus search
- [ ] Main graph: responsive relayout on resize
- [ ] Main graph: viewport stretch to fill space
- [ ] Flow view: dagre layout
- [ ] Flow view: node type styling (start/step/key/decision/error/end/drillable)
- [ ] Flow view: edge labels with Yes/No coloring
- [ ] Flow view: click node → highlight + show info
- [ ] Flow view: double-click drillable → inline expand
- [ ] Flow view: double-click expanded → collapse
- [ ] Flow view: click background → clear highlights
- [ ] Flow view: breadcrumb navigation
- [ ] Flow view: URL hash persistence
- [ ] Flow view: browser history (back/forward)
- [ ] Flow view: Escape → close overlay
- [ ] Stats bar: class count, connection count, drill-down count
- [ ] Help hints bar
