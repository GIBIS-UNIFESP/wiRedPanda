# wiRedPanda Site

This branch contains the source code for the official wiRedPanda website. The site is built using Astro and Tailwind CSS. We hope you enjoy using the wiRedPanda software!

If you'd like to see more:

- Visit the live **[wiRedPanda site here](https://gibis-unifesp.github.io/wiRedPanda)**.
- Explore the **[wiRedPanda software repository here](https://github.com/GIBIS-UNIFESP/wiRedPanda)**.

## API documentation

The header's "API Docs" link points at `/api/`, which is Doxygen output generated from the
`master` branch's C++ sources. CI builds it on every deploy, so it only exists in a local
checkout once you generate it:

```sh
npm run docs
```

That initialises the `doxygen-awesome-css` submodule, checks `origin/master` out into
`master-src/` as a worktree, runs Doxygen, and copies the result to `public/api/`. It needs
`doxygen` and `graphviz` on your PATH, takes about ten seconds, and is safe to re-run. The
generated `docs/`, `master-src/` and `public/api/` directories are gitignored.

Skip it if you are not working on the API-docs link — everything else builds without it.
