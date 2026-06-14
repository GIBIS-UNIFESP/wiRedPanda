// Conservative, render-preserving editor-cruft scrub for wiRedPanda's SVG resources.
//
// Allow-list ONLY: this does NOT use preset-default, so geometry/path/viewBox/style
// plugins never run — path `d` data, coordinates, colors, styles and viewBox/size are
// left byte-for-byte intact. Only Inkscape/editor cruft is removed, so QSvgRenderer
// output is identical. cleanupIds runs before removeUselessDefs so unreferenced (but
// id'd) dead <defs> lose their id and are then dropped; minify:false keeps referenced
// id names unchanged.
//
// CAUTION: SVGO 4 can corrupt base64 data URIs that contain whitespace (Inkscape line-wraps
// its <image> data) — it mangled one character of dolphin_icon.svg's embedded PNG. Keep any
// embedded raster as clean single-line base64, and render-compare embedded-image SVGs after
// running (QSvgRenderer pristine-vs-cleaned must be pixel-identical).
export default {
  multipass: false,
  js2svg: { pretty: true, indent: 2, eol: 'lf' },
  plugins: [
    'removeComments',
    'removeMetadata',
    'removeEditorsNSData',
    'removeDesc',
    { name: 'cleanupIds', params: { minify: false } },
    'removeUselessDefs',
    'removeEmptyContainers',
    'removeEmptyText',
    'removeUnusedNS',
  ],
};
