TEMPLATE = subdirs
SUBDIRS = lib app test

# Build order dependencies
app.depends = lib
test.depends = lib

# Skip tests in WASM builds (tests are not needed for web deployment)
wasm {
    SUBDIRS -= test
}
