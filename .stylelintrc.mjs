export default {
  extends: ['stylelint-config-standard'],
  overrides: [
    {
      files: ['**/*.astro'],
      customSyntax: 'postcss-html',
    },
  ],
  rules: {
    // Tailwind v4 at-rules
    'at-rule-no-unknown': [true, { ignoreAtRules: ['theme', 'custom-variant', 'apply'] }],
    // Astro scoping pseudo-class
    'selector-pseudo-class-no-unknown': [true, { ignorePseudoClasses: ['global'] }],
    // Tailwind v4 requires @import 'tailwindcss' (string, not url())
    'import-notation': 'string',
  },
};
