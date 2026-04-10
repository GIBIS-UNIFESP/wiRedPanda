// Coverage probe: wraps cytoscape to log which methods are actually called.
// Load this AFTER cytoscape.umd.js and BEFORE any other scripts.
// After exercising all features, call coverageReport() in the console.

(function() {
  const hits = {};
  const ignoredKeys = new Set(['length', 'name', 'prototype', 'constructor', 'caller', 'arguments', '__proto__']);

  function wrapObj(obj, path) {
    if (!obj || typeof obj !== 'object') return;
    for (const key of Object.getOwnPropertyNames(obj)) {
      if (ignoredKeys.has(key)) continue;
      try {
        const desc = Object.getOwnPropertyDescriptor(obj, key);
        if (!desc || !desc.writable || !desc.configurable) continue;
        const val = obj[key];
        if (typeof val === 'function') {
          const fullPath = path + '.' + key;
          obj[key] = function() {
            hits[fullPath] = (hits[fullPath] || 0) + 1;
            return val.apply(this, arguments);
          };
          // Preserve function properties
          Object.defineProperty(obj[key], 'name', { value: val.name });
          if (val.prototype) obj[key].prototype = val.prototype;
        }
      } catch (e) { /* skip non-wrappable */ }
    }
  }

  // Wrap cytoscape constructor
  const origCytoscape = window.cytoscape;
  let cyProtoWrapped = false;
  let collProtoWrapped = false;

  window.cytoscape = function() {
    hits['cytoscape()'] = (hits['cytoscape()'] || 0) + 1;
    const result = origCytoscape.apply(this, arguments);

    // Plugin registration calls return undefined — skip wrapping
    if (!result) return result;

    // Wrap instance prototype methods (once)
    if (!cyProtoWrapped) {
      const proto = Object.getPrototypeOf(result);
      if (proto) { wrapObj(proto, 'cy'); cyProtoWrapped = true; }
    }

    // Wrap collection prototype methods (once)
    if (!collProtoWrapped && result.nodes) {
      try {
        const coll = result.nodes();
        const collProto = Object.getPrototypeOf(coll);
        if (collProto) { wrapObj(collProto, 'collection'); collProtoWrapped = true; }
      } catch (e) { /* not ready yet */ }
    }

    return result;
  };
  // Copy static properties so plugins can register
  for (const key of Object.getOwnPropertyNames(origCytoscape)) {
    if (key !== 'length' && key !== 'name' && key !== 'prototype') {
      try { window.cytoscape[key] = origCytoscape[key]; } catch (e) {}
    }
  }
  window.cytoscape.prototype = origCytoscape.prototype;

  // Report function
  window.coverageReport = function() {
    const sorted = Object.entries(hits).sort((a, b) => b[1] - a[1]);
    console.log('%c=== Cytoscape Coverage Report ===', 'font-weight:bold;font-size:14px');
    console.log(`${sorted.length} methods called:\n`);
    console.table(sorted.map(([method, count]) => ({ method, calls: count })));

    // Group by prefix
    const groups = {};
    for (const [method, count] of sorted) {
      const prefix = method.split('.')[0];
      if (!groups[prefix]) groups[prefix] = [];
      groups[prefix].push({ method, calls: count });
    }
    console.log('\n%cGrouped by object:', 'font-weight:bold');
    for (const [prefix, methods] of Object.entries(groups)) {
      console.groupCollapsed(`${prefix} (${methods.length} methods)`);
      console.table(methods);
      console.groupEnd();
    }

    return sorted;
  };

  console.log('%c[coverage-probe] Active. Exercise all features, then run coverageReport() in console.',
    'color: #3fb950; font-weight: bold');
})();
