# Documentation Generation Options

Comparison of documentation toolchains for the wiRedPanda codebase (fully Doxygen-commented C++/Qt).

## Quick Comparison

| Tool | Setup Time | Visual Quality | Maintenance | Narrative Docs | C++/Qt Support | Risk |
|------|-----------|----------------|-------------|----------------|----------------|------|
| Doxygen + awesome-css | ~10 min | Good | Near zero | Basic | Excellent | None |
| Sphinx + Breathe | Hours | Great | Medium | Excellent | Good | Low |
| Doxygen + m.css | ~1 hr | Great | Low-medium | Basic | Excellent | Medium |
| MkDocs Material | Hours | Best | Higher risk | Excellent | Maturing | **Dead project** |
| Plain Doxygen | ~5 min | Poor | Zero | Basic | Excellent | None |

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

### Build Test Results (2026-04-09)

- Built successfully with zero errors against the full wiRedPanda codebase.
- Output: 50 MB in `build/docs/html-awesome/`.
- No patches or workarounds required.

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
- **Stable project with long track record** — no maintainership drama.

### Weaknesses

- Two toolchains to maintain: Doxygen (XML pass) + Sphinx (HTML pass).
- Breathe can struggle with complex C++ templates and overloaded functions.
- Qt signals/slots sometimes need manual Breathe directives to render correctly.
- Slower builds: Doxygen XML generation + Sphinx processing.
- Exhale auto-generation can produce noisy page hierarchies for large codebases.
- Python environment dependency adds CI complexity.
- More configuration surface area — more things can break on upgrades.
- **RST syntax is widely disliked** — MyST-parser (Markdown for Sphinx) is a workaround but adds another dependency.

### Build Test Results (2026-04-09)

- Built successfully with 116 warnings (mostly cosmetic Exhale directory-parent issues).
- Output: 78 MB in `build/docs/sphinx/_build/` (largest of all options).
- Used Furo theme. No patches required.

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
- **Smallest output size** of all options tested (3.0 MB).

### Weaknesses

- Smaller community than doxygen-awesome-css or Sphinx.
- Maintained but with less frequent updates.
- **Tighter coupling to specific Doxygen XML format — Doxygen version upgrades can break output.**
- Fewer theme variants or customization hooks.
- Python dependency for the postprocessor.
- Limited narrative documentation support (similar to plain Doxygen).
- Qt-specific constructs (Q_OBJECT, signals/slots) may need manual attention.

### Build Test Results (2026-04-09)

- **Required 4 patches to work with Doxygen 1.16.1:**
  1. `latex2svg.py` — libgs search path missing `/usr/lib/x86_64-linux-gnu/`.
  2. `doxygen.py` — `extract_metadata()` filter did not include `interface` compound kind (early return skipped interface XML files entirely).
  3. `doxygen.py` — `parse_xml()` private-struct filter did not include `interface`.
  4. `doxygen.py` — two `compound.kind` assignment sites needed `interface` → `class` normalization.
- After patches: built successfully with warnings only (parameter description mismatches, programlisting fallbacks).
- Output: 3.0 MB in `build/docs/html-mcss/`.
- **Confirms the document's warning about Doxygen version coupling** — this is a real maintenance risk.

### Best For

Projects that want a very clean, minimalist documentation site and don't mind a smaller ecosystem. Good for library/engine projects with technically sophisticated audiences. **Not recommended if you upgrade Doxygen regularly.**

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

### Build Test Results (2026-04-09)

- Built successfully (hand-written content only, no auto-generated API from Doxygen XML since the C++ handler is too immature to wire up automatically).
- Output: 2.7 MB in `build/docs/mkdocs-output/`.
- MkDocs Material 9.7.6 displays a warning about MkDocs 2.0 incompatibility on every build.

### MkDocs Project Status: Dead (as of 2026-04-09)

**Do not adopt MkDocs for new projects.** The project has collapsed due to maintainership drama, and the ecosystem is fragmenting into incompatible successors.

#### Timeline of Collapse

Source: ["The Slow Collapse of MkDocs"](https://fpgmaas.com/blog/collapse-of-mkdocs/) by Florian Maas (Mar 22, 2026) and [Reddit discussion](https://www.reddit.com/r/Python/comments/1s0gfyb/the_slow_collapse_of_mkdocs/) (470 upvotes, ~80 comments).

- **2014**: @lovelydinosaur created MkDocs, went inactive shortly after.
- **2016-2021**: @waylan maintained MkDocs as sole maintainer.
- **2020**: @oprypin joined, friction with @waylan over contribution acceptance.
- **May 2021**: @oprypin publicly accused @waylan of gatekeeping; @waylan stepped down.
- **Jun 2021**: @oprypin became primary maintainer with @squidfunk (Material) and @ultrabug.
- **Mar 2024**: @oprypin unilaterally removed @squidfunk from the MkDocs org. @lovelydinosaur reinstated @squidfunk and stripped @oprypin's ownership.
- **Apr 2024**: @oprypin stepped down. Released v1.6.0, helped with v1.6.1 (Aug 2024). Community rally fizzled — only 2 people attended a scheduled call.
- **Aug 2024**: MkDocs 1.6.1 released. **Last meaningful release.**
- **Jul 2025**: Plugin author @facelessuser opened "Is this project abandoned?" discussion. Bug fix PRs sat unreviewed for months.
- **Jan 2026**: @lovelydinosaur announced MkDocs 2.0 — **a private rewrite that removes the entire plugin system**, the feature that made MkDocs successful. Community reaction was overwhelmingly negative. @twardoch: "Before MkDocs Material, MkDocs was a toy. The version 2 announcements suggest MkDocs is going back to its roots—becoming a toy again."
- **Feb 2026**: Material for MkDocs published a detailed analysis of v2 incompatibilities. MkDocs 2.0 repository moved to encode/ org with contributing guidelines explicitly discouraging issues or PRs.
- **Mar 9, 2026**: @oprypin seized PyPI access (still had credentials from maintainer days), locked out @lovelydinosaur. Backed down within 6 hours. @lovelydinosaur: "What the actual fuck? I'm the author and license holder."
- **Mar 15, 2026**: @oprypin launched ProperDocs — a drop-in MkDocs 1.x replacement.
- **Nov 2025-present**: @squidfunk's team put Material for MkDocs in maintenance mode and launched Zensical, a ground-up rewrite.

#### Competing Successors

| Project | Author | Approach | GitHub Stars |
|---------|--------|----------|--------------|
| **Zensical** | @squidfunk + @pawamoy | Ground-up rewrite, reads mkdocs.yml natively, 5x faster builds | ~3,700 |
| **ProperDocs** | @oprypin | Drop-in MkDocs 1.x fork, existing plugins work unchanged | ~21 |
| **MaterialX** | @jaywhj | Continuation of Material for MkDocs theme | Growing |

#### Community Sentiment (Reddit, Mar 2026)

- **Zensical is the clear community favorite** — multiple commenters report successful migrations. u/galateax (6-year MkDocs admin): "Squidfunk/Martin, Alex, and pawamoy/Timotheé have proven they're trustworthy members of the open-source community."
- **Same author (@lovelydinosaur) also abandoned HTTPX** — pattern of creating popular projects, going inactive, then returning with incompatible rewrites.
- u/pydry: "mkdocs + material is hands down the best markdown-to-html documentation generator out there" — but acknowledges it's now dead.
- u/UUDDLRLRBadAlchemy: "if you were considering MkDocs for code documentation, the people you'd have put your trust in are the ones who built Zensical."
- u/-techno_viking-: "Behaving like they did in this mkdocs drama in a paid job setting would've meant getting written up, getting fired and possible legal action."

#### Other Alternatives Mentioned by the Community

- **Docusaurus** (React-based, used by Meta): well-documented, good built-in features, React component support.
- **Astro/Starlight**: modern JS-based SSG with a clean default docs theme.
- **Quarto**: science-oriented, supports embedded executable code fragments.
- **Great Docs by Posit**: new entrant leveraging Quarto, reportedly well-maintained.
- **MyST-parser for Sphinx**: Markdown syntax for Sphinx, avoiding RST pain.

### Best For

~~Projects that prioritize visual polish and Markdown-based workflows.~~ **No longer recommended for any new project.** If you want the Material look, evaluate Zensical instead — but note it has no C++ documentation story yet and is too new to rely on for a C++/Qt project.

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

### Build Test Results (2026-04-09)

- Built successfully with zero errors.
- Output: 51 MB in `build/docs/html/`.

### Best For

Internal developer documentation where functionality matters more than aesthetics. Also useful as a baseline to verify Doxygen comment coverage before adding a theme.

---

## Build Test Summary (2026-04-09)

All five options were built against the full wiRedPanda codebase (App/ + MCP/Server/).

| # | Tool | Output Path | Size | Errors | Patches Needed |
|---|------|-------------|------|--------|----------------|
| 1 | Plain Doxygen | `build/docs/html/` | 51 MB | 0 | 0 |
| 2 | Doxygen + awesome-css | `build/docs/html-awesome/` | 50 MB | 0 | 0 |
| 3 | Sphinx + Breathe + Exhale | `build/docs/sphinx/_build/` | 78 MB | 116 warnings | 0 |
| 4 | Doxygen + m.css | `build/docs/html-mcss/` | 3.0 MB | 0 (after patches) | 4 |
| 5 | MkDocs + Material | `build/docs/mkdocs-output/` | 2.7 MB | 0 (hand-written only) | 0 |

Open any with: `xdg-open <output-path>/index.html`

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

1. **Start with Doxygen + doxygen-awesome-css** — minimal effort, immediate results from existing comments, zero maintenance risk, and a GitHub Action can automate deployment to GitHub Pages. Built and tested successfully with no issues.

2. **Upgrade to Sphinx + Breathe later** if there's a need for tutorials, user guides, or contributor documentation alongside the API reference. Use MyST-parser to avoid RST syntax pain.

3. **Avoid MkDocs** — the project is dead and fragmenting. Zensical (the successor) has no C++ documentation support and is too new to bet on.

4. **Avoid m.css** unless you are willing to patch it on every Doxygen upgrade. The output is the cleanest of all options, but the maintenance burden is real.

The incremental approach avoids over-engineering the docs infrastructure while still producing a professional result from day one.
