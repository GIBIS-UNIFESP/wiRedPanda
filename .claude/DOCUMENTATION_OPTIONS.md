# Documentation Generation Options

Comparison of documentation toolchains for the wiRedPanda codebase (fully Doxygen-commented C++/Qt).

## Quick Comparison

| Tool | Setup Time | Visual Quality | Maintenance | Narrative Docs | C++/Qt Support |
|------|-----------|----------------|-------------|----------------|----------------|
| Doxygen + awesome-css | ~10 min | Good | Near zero | Basic | Excellent |
| Sphinx + Breathe | Hours | Great | Medium | Excellent | Good |
| Doxygen + m.css | ~1 hr | Great | Low-medium | Basic | Excellent |
| MkDocs Material | Hours | Best | Higher risk | Excellent | Maturing |
| Plain Doxygen | ~5 min | Poor | Zero | Basic | Excellent |

---

## Doxygen + doxygen-awesome-css

A drop-in CSS theme for standard Doxygen HTML output. Transforms the default Doxygen look into a modern, responsive design with dark mode support.

**Repository**: [jothepro/doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css)

### Setup

- Add a `Doxyfile` to the project root.
- Include doxygen-awesome-css as a submodule or vendored copy.
- Set `HTML_EXTRA_STYLESHEET` in the Doxyfile to point to the theme CSS.
- Run `doxygen` — done.

### Strengths

- Minimal configuration — works directly with existing Doxygen comments.
- Modern, responsive layout with sidebar navigation.
- Dark mode support out of the box.
- Active maintenance with a large user base.
- No additional toolchain — just Doxygen.
- Fast build times (single pass).
- Full support for Qt-specific constructs (signals, slots, properties, Q_OBJECT macros).

### Weaknesses

- Still Doxygen under the hood — page structure is rigid.
- Narrative/tutorial content limited to Doxygen's `@page`, `@mainpage`, and Markdown files.
- Client-side JavaScript search — functional but not as powerful as server-side alternatives.
- Limited theme customization beyond CSS overrides.
- No cross-project linking or external content aggregation.

### Best For

Projects that need clean API reference documentation from existing Doxygen comments with minimal effort. Ideal when the primary audience is developers reading class/function docs.

---

## Sphinx + Breathe + Exhale

A two-stage pipeline: Doxygen generates XML from C++ comments, Breathe bridges that XML into Sphinx, and Exhale auto-generates the API reference page hierarchy.

**Repositories**:
- [breathe-doc/breathe](https://github.com/breathe-doc/breathe)
- [svenevs/exhale](https://github.com/svenevs/exhale)

### Setup

- Add a `Doxyfile` configured to output XML (not HTML).
- Set up a Python virtual environment with Sphinx, Breathe, and Exhale.
- Create a `docs/conf.py` with Breathe/Exhale configuration.
- Write `index.rst` or `index.md` as the documentation entry point.
- Choose a Sphinx theme (Furo, Read the Docs, Book, etc.).
- Build with `make html` or `sphinx-build`.

### Strengths

- Rich narrative documentation support — tutorials, guides, architecture docs all live alongside API reference.
- Wide selection of high-quality themes (Furo, Read the Docs, PyData, Book).
- reStructuredText and MyST Markdown support for content authoring.
- Cross-referencing between narrative docs and API reference.
- Extensive plugin ecosystem (search, versioning, internationalization).
- Read the Docs free hosting with automatic builds from Git.
- Widely recognized in open-source — users know how to navigate Sphinx sites.

### Weaknesses

- Two toolchains to maintain: Doxygen (XML pass) + Sphinx (HTML pass).
- Breathe can struggle with complex C++ templates and overloaded functions.
- Qt signals/slots sometimes need manual Breathe directives to render correctly.
- Slower builds: Doxygen XML generation + Sphinx processing.
- Exhale auto-generation can produce noisy page hierarchies for large codebases.
- Python environment dependency adds CI complexity.
- More configuration surface area — more things can break on upgrades.

### Best For

Projects that need both API reference and substantial narrative documentation (tutorials, architecture guides, contributor docs). Good for user-facing projects where non-developers also read the docs.

---

## Doxygen + m.css

A Doxygen XML postprocessor written in Python that generates clean, minimalist HTML from Doxygen's XML output. Created for the Magnum game engine.

**Repository**: [mosra/m.css](https://github.com/mosra/m.css)

### Setup

- Add a `Doxyfile` configured to output XML.
- Clone or install m.css.
- Create a `conf.py` for m.css with Doxygen XML path and theme settings.
- Run the m.css postprocessor script.

### Strengths

- Very clean, opinionated design — minimal visual clutter.
- Good syntax highlighting and code block rendering.
- Lightweight output — small HTML/CSS footprint.
- Handles C++ well, including templates and namespaces.
- Search functionality built in.

### Weaknesses

- Smaller community than doxygen-awesome-css or Sphinx.
- Maintained but with less frequent updates.
- Tighter coupling to specific Doxygen XML format — Doxygen version upgrades can break output.
- Fewer theme variants or customization hooks.
- Python dependency for the postprocessor.
- Limited narrative documentation support (similar to plain Doxygen).
- Qt-specific constructs (Q_OBJECT, signals/slots) may need manual attention.

### Best For

Projects that want a very clean, minimalist documentation site and don't mind a smaller ecosystem. Good for library/engine projects with technically sophisticated audiences.

---

## MkDocs + mkdocstrings (C++ handler)

MkDocs is a Markdown-based static site generator. The mkdocstrings plugin with its C++ handler uses Doxygen XML to inject API documentation into Markdown pages.

**Repositories**:
- [mkdocs/mkdocs](https://github.com/mkdocs/mkdocs)
- [mkdocstrings/mkdocstrings](https://github.com/mkdocstrings/mkdocstrings)
- Theme: [squidfunk/mkdocs-material](https://github.com/squidfunk/mkdocs-material)

### Setup

- Add a `Doxyfile` configured to output XML.
- Set up Python environment with MkDocs, mkdocstrings, and the C++ handler.
- Create `mkdocs.yml` with theme and plugin configuration.
- Write Markdown documentation pages with mkdocstrings directives.
- Build with `mkdocs build` or `mkdocs serve`.

### Strengths

- Material for MkDocs theme is arguably the best-looking documentation theme available.
- Pure Markdown authoring — lower barrier for contributors.
- Excellent search (lunr.js or server-side with Algolia).
- Built-in versioning support.
- Hot-reload development server for writing docs.
- Strong plugin ecosystem (macros, redirects, minification).
- Social cards, announcement bars, and other modern features.

### Weaknesses

- The C++ handler for mkdocstrings is less mature than the Python handler.
- May miss edge cases with Qt-specific patterns, macros, and property systems.
- Still requires Doxygen as a preprocessing step for XML.
- Three tools in the pipeline: Doxygen (XML) → mkdocstrings (bridge) → MkDocs (HTML).
- Auto-generating full API reference pages requires more manual configuration than Exhale.
- Breaking changes more likely as the C++ handler evolves.

### Best For

Projects that prioritize visual polish and Markdown-based workflows, and are willing to accept some risk from newer tooling. Excellent if the team already uses MkDocs for other projects.

---

## Plain Doxygen (Default Theme)

Standard Doxygen HTML output with no additional themes or tooling.

### Setup

- Add a `Doxyfile` to the project root.
- Run `doxygen`.

### Strengths

- Zero dependencies beyond Doxygen itself.
- Most battle-tested option — handles every C++ and Qt construct.
- Full-text search, class hierarchy graphs, collaboration diagrams.
- Fastest path from comments to published docs.
- Supports LaTeX/PDF output in addition to HTML.

### Weaknesses

- Visual design looks dated (frames-based layout from the early 2000s).
- Not responsive — poor experience on mobile or narrow viewports.
- No dark mode.
- Navigation can be difficult in large projects.
- Generates very large HTML output with inline styles.

### Best For

Internal developer documentation where functionality matters more than aesthetics. Also useful as a baseline to verify Doxygen comment coverage before adding a theme.

---

## Deployment Options

All options above produce static HTML and can be deployed to:

- **GitHub Pages** — Free, automated via GitHub Actions on push or release.
- **Read the Docs** — Free for open-source, native Sphinx support, versioned docs.
- **Netlify / Vercel** — Free tier available, preview deploys for PRs.
- **Self-hosted** — Any static file server (nginx, Apache, S3 + CloudFront).

---

## Recommendation

For wiRedPanda specifically:

1. **Start with Doxygen + doxygen-awesome-css** — minimal effort, immediate results from existing comments, and a GitHub Action can automate deployment to GitHub Pages.

2. **Upgrade to Sphinx/Breathe later** if there's a need for tutorials, user guides, or contributor documentation alongside the API reference.

The incremental approach avoids over-engineering the docs infrastructure while still producing a professional result from day one.
