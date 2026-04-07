# Inline IC Investigation

## Problem Summary

Multiple bugs when editing inline ICs:
1. Dropping a file-based IC into an inline IC tab → save → close → reopen gives "Invalid file format"
2. Saving an inline IC tab shows file dialog instead of emitting blob
3. Inline IC tab title changes to parent filename after save
4. Root tab not marked dirty when inline IC child saves

## Root Causes Found

### Bug 1: "Invalid file format" on reopen

**Root cause**: `disconnectTab()` does NOT disconnect `Scene::icOpenRequested` connections. Each `connectTab()` call adds a new connection. After tab close/reopen cycles, stale connections accumulate.

When the user double-clicks an IC, the `icOpenRequested` lambda uses `m_currentTab` at signal-emission time. Due to stale connections, the lambda can fire while `m_currentTab` points to the wrong tab (e.g., the inline IC tab instead of the root tab). The wrong tab's blob registry doesn't contain the requested blob → empty blob → "Invalid file format" when parsing.

**Evidence from logs**:
```
m_currentTab: WorkSpace(0x5db43570ec30) isInlineIC: true tabIndex: 2
blobName: "chain_b" blobSize: 0 blobEmpty: true
WARNING: blob is empty! Registry keys: QList("chain_c", "counter")
```
The chain_b tab's registry has `chain_c` and `counter`, but not `chain_b` itself.

**Fix**: Add `disconnect(m_currentTab->scene(), &Scene::icOpenRequested, this, nullptr)` to `disconnectTab()`.

### Bug 2: File dialog when saving inline IC tab

**Root cause**: `MainWindow::on_actionSave_triggered()` (line 704-732) checks `currentFile().absoluteFilePath()`. For inline IC tabs, `currentFile()` returns empty → `fileName.isEmpty()` is true → opens file dialog. The code reaches `WorkSpace::save(fileName)` with a user-chosen path, bypassing the inline save branch which expects the `m_isInlineIC` check at line 95.

Actually, the inline save branch IS still at line 95 of `WorkSpace::save()` and would be entered regardless of the filename passed. But the file dialog itself is the bug — it shouldn't appear for inline IC tabs.

**Fix**: Check `m_currentTab->isInlineIC()` in `on_actionSave_triggered()` and call `save("")` directly without the file dialog.

### Bug 3: Tab title changes to parent filename

**Root cause**: The earlier fix `m_fileInfo = parent->fileInfo()` in `loadFromBlob()` set the inline IC's `m_fileInfo` to the parent's file. When `undoStack->setClean()` fires during inline save, the MainWindow updates the tab title using `m_fileInfo.fileName()`, showing "nested.panda" instead of "[chain_b]".

**Fix**: Don't set `m_fileInfo` in `loadFromBlob()`. Instead, fix `icListFile()` to walk up the parent chain.

### Bug 4: Root tab not marked dirty

**Root cause**: `onChildICBlobSaved()` pushes an `UpdateBlobCommand` to the root scene's undo stack. This should mark it as dirty. However, the tab title might not refresh because `setTabText()` is only called during specific events.

**Status**: Need to verify — may be a tab title refresh issue rather than a dirty-state issue.

## Architecture Notes

### Signal flow for inline IC open
```
IC::mouseDoubleClickEvent
  → emit requestOpenSubCircuit(id, blobName, file)
  → Scene::icOpenRequested (signal relay)
  → MainWindow lambda (connected in connectTab)
    → m_currentTab->scene()->icRegistry()->blob(blobName)
    → openICInTab(blobName, elementId, blob)
      → createNewTab() [changes m_currentTab!]
      → m_currentTab->loadFromBlob(blob, parent, id, contextDir)
```

### Signal flow for inline IC save
```
Ctrl+S
  → MainWindow::on_actionSave_triggered
    → currentFile().absoluteFilePath() [EMPTY for inline]
    → file dialog [BUG]
    → MainWindow::save(fileName)
      → m_currentTab->save(fileName)
        → WorkSpace::save → m_isInlineIC branch
          → embed file-backed ICs
          → serialize blob
          → emit icBlobSaved(parentICElementId, blob)
            → parent WorkSpace::onChildICBlobSaved
              → setBlob + loadFromBlob on all targets
              → push UpdateBlobCommand
```

### Tab lifecycle and connections
- `connectTab()` is called on every tab switch (in `tabChanged`)
- `disconnectTab()` is called before switching away
- `disconnectTab()` was missing `icOpenRequested` disconnect → stale connections accumulate
- `deleteLater()` is used for tab deletion → scene survives briefly after tab close

### m_fileInfo usage in Workspace
- `save()`: determines filename for non-inline saves (line 141)
- `fileInfo()`: returned to MainWindow for title, palette, etc.
- `isFromNewerVersion()`: checks `m_loadedVersion`, not `m_fileInfo`
- Setting `m_fileInfo` for inline tabs causes tab title to show parent filename

### Blob registry ownership
- Root workspace: has `QList("chain_b")` in its registry
- chain_b inline tab: has `QList("chain_c")` (and "counter" after drop)
- Each tab's scene has its OWN `ICRegistry` instance
- Blob lookup must happen on the correct tab's registry
