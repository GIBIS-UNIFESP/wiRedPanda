// Shared SVG utilities for graph and flow views

const SVG_NS = 'http://www.w3.org/2000/svg';

function svgEl(tag, attrs) {
  const el = document.createElementNS(SVG_NS, tag);
  if (attrs) for (const [k, v] of Object.entries(attrs)) el.setAttribute(k, v);
  return el;
}

// Canvas-based text measurement (shared with old graph-setup.js)
const _measureCtx = document.createElement('canvas').getContext('2d');
function measureText(text, fontSize) {
  _measureCtx.font = fontSize + 'px -apple-system, "Segoe UI", Helvetica, Arial, sans-serif';
  return _measureCtx.measureText(text).width;
}

// Multi-line text measurement: returns { width, height, lines }
function measureMultilineText(text, fontSize) {
  const lines = text.split('\n');
  let maxW = 0;
  for (const line of lines) maxW = Math.max(maxW, measureText(line, fontSize));
  return { width: maxW, height: lines.length * (fontSize + 4), lines };
}

// Create an SVG canvas with viewBox zoom/pan
function createSvgCanvas(container) {
  container.innerHTML = '';

  const svg = svgEl('svg', { width: '100%', height: '100%', style: 'display:block' });
  const defs = svgEl('defs');
  svg.appendChild(defs);
  const root = svgEl('g');
  svg.appendChild(root);
  container.appendChild(svg);

  // ViewBox state
  let vb = { x: 0, y: 0, w: 1000, h: 800 };
  const minZoom = 0.08, maxZoom = 3;

  function applyViewBox() {
    svg.setAttribute('viewBox', `${vb.x} ${vb.y} ${vb.w} ${vb.h}`);
  }
  applyViewBox();

  // Wheel zoom toward cursor
  svg.addEventListener('wheel', (e) => {
    e.preventDefault();
    const rect = svg.getBoundingClientRect();
    const mx = (e.clientX - rect.left) / rect.width;
    const my = (e.clientY - rect.top) / rect.height;
    const cursorX = vb.x + mx * vb.w;
    const cursorY = vb.y + my * vb.h;

    const factor = e.deltaY > 0 ? 1.12 : 1 / 1.12;
    const newW = vb.w * factor;
    const newH = vb.h * factor;

    // Clamp zoom
    const baseW = 1000;
    if (newW < baseW * minZoom || newW > baseW * maxZoom) return;

    vb.x = cursorX - mx * newW;
    vb.y = cursorY - my * newH;
    vb.w = newW;
    vb.h = newH;
    applyViewBox();
  }, { passive: false });

  // Mouse drag pan (on background only)
  let dragging = false, dragStart = { x: 0, y: 0 }, vbStart = { x: 0, y: 0 };

  svg.addEventListener('mousedown', (e) => {
    if (e.target === svg || e.target === root) {
      dragging = true;
      dragStart = { x: e.clientX, y: e.clientY };
      vbStart = { x: vb.x, y: vb.y };
      svg.style.cursor = 'grabbing';
    }
  });
  window.addEventListener('mousemove', (e) => {
    if (!dragging) return;
    const rect = svg.getBoundingClientRect();
    const dx = (e.clientX - dragStart.x) / rect.width * vb.w;
    const dy = (e.clientY - dragStart.y) / rect.height * vb.h;
    vb.x = vbStart.x - dx;
    vb.y = vbStart.y - dy;
    applyViewBox();
  });
  window.addEventListener('mouseup', () => {
    if (dragging) { dragging = false; svg.style.cursor = ''; }
  });

  function fit(bounds, padding) {
    padding = padding || 40;
    const rect = svg.getBoundingClientRect();
    const aspect = rect.width / rect.height;
    let w = bounds.w + padding * 2;
    let h = bounds.h + padding * 2;
    if (w / h > aspect) { h = w / aspect; } else { w = h * aspect; }
    vb = {
      x: bounds.x - (w - bounds.w) / 2,
      y: bounds.y - (h - bounds.h) / 2,
      w, h
    };
    applyViewBox();
  }

  function animateFit(bounds, padding, duration) {
    padding = padding || 40;
    duration = duration || 400;
    const rect = svg.getBoundingClientRect();
    const aspect = rect.width / rect.height;
    let w = bounds.w + padding * 2;
    let h = bounds.h + padding * 2;
    if (w / h > aspect) { h = w / aspect; } else { w = h * aspect; }
    const target = {
      x: bounds.x - (w - bounds.w) / 2,
      y: bounds.y - (h - bounds.h) / 2,
      w, h
    };
    const start = { ...vb };
    const t0 = performance.now();
    function step(now) {
      const t = Math.min((now - t0) / duration, 1);
      const ease = t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t;
      vb.x = start.x + (target.x - start.x) * ease;
      vb.y = start.y + (target.y - start.y) * ease;
      vb.w = start.w + (target.w - start.w) * ease;
      vb.h = start.h + (target.h - start.h) * ease;
      applyViewBox();
      if (t < 1) requestAnimationFrame(step);
    }
    requestAnimationFrame(step);
  }

  function getViewBox() { return { ...vb }; }
  function setViewBox(newVb) { vb = { ...newVb }; applyViewBox(); }

  function destroy() { container.innerHTML = ''; }

  return { svg, defs, root, fit, animateFit, getViewBox, setViewBox, destroy, applyViewBox };
}

// Add an arrow marker to defs
function addArrowMarker(defs, id, color) {
  const marker = svgEl('marker', {
    id, markerWidth: '10', markerHeight: '7', refX: '10', refY: '3.5', orient: 'auto', markerUnits: 'strokeWidth'
  });
  const poly = svgEl('polygon', { points: '0 0, 10 3.5, 0 7', fill: color });
  marker.appendChild(poly);
  defs.appendChild(marker);
  return marker;
}

// Convert a series of points to an SVG path d attribute (polyline with straight segments)
function drawEdgePath(points) {
  if (!points || points.length === 0) return '';
  let d = `M ${points[0].x} ${points[0].y}`;
  for (let i = 1; i < points.length; i++) {
    d += ` L ${points[i].x} ${points[i].y}`;
  }
  return d;
}

// Bounding box helper for a set of positioned objects {x, y, w, h}
function computeBounds(rects) {
  if (rects.length === 0) return { x: 0, y: 0, w: 0, h: 0 };
  let x1 = Infinity, y1 = Infinity, x2 = -Infinity, y2 = -Infinity;
  for (const r of rects) {
    x1 = Math.min(x1, r.x);
    y1 = Math.min(y1, r.y);
    x2 = Math.max(x2, r.x + (r.w || 0));
    y2 = Math.max(y2, r.y + (r.h || 0));
  }
  return { x: x1, y: y1, w: x2 - x1, h: y2 - y1 };
}
