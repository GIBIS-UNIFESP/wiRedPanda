# Patch the WASM HTML shell to include the service worker and favicon.
# Usage: cmake -DHTML_FILE=index.html -P patch_wasm_html.cmake

file(READ "${HTML_FILE}" content)

# Add service worker script after wiredpanda.js
string(REPLACE
    "<script src=\"wiredpanda.js\"></script>"
    "<script src=\"wiredpanda.js\"></script>\n<script src=\"coi-serviceworker.min.js\"></script>"
    content "${content}"
)

# Add favicon after <title>
string(REGEX REPLACE
    "(<title>[^<]*</title>)"
    "\\1\n<link rel=\"icon\" type=\"image/x-icon\" href=\"favicon.ico\">"
    content "${content}"
)

file(WRITE "${HTML_FILE}" "${content}")
