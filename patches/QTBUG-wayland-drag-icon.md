# QDrag pixmap not visible on Wayland (Qt 6.10+)

## Summary

`QDrag::setPixmap()` has no visible effect on Wayland since Qt 6.10. The drag operation works correctly (drop succeeds), but no icon is rendered at the cursor during the drag. Works fine on XCB and on Qt 6.9.x with Wayland.

## Environment

- **Affects**: Qt 6.10.0+ (tested on 6.10.3 and 6.11.0)
- **Works on**: Qt 6.9.3
- **Platform**: Wayland (Mutter/GNOME on Ubuntu)
- **Not affected**: XCB backend

## Steps to Reproduce

1. Build the "Draggable Icons" example (`qtwidgets/draganddrop/draggableicons`) with Qt 6.10+ on a Wayland session
2. Drag an icon
3. No pixmap follows the cursor during drag
4. Drop still works — the item appears at the drop location

## Root Cause

In Qt 6.10, `QWaylandWindow::sendExposeEvent()` was changed to use `SynchronousDelivery`:

```cpp
QWindowSystemInterface::handleExposeEvent<QWindowSystemInterface::SynchronousDelivery>(window(), rect);
```

In Qt 6.9, this was asynchronous:

```cpp
QWindowSystemInterface::handleExposeEvent(window(), rect);
```

This matters for drag icon surfaces (`QShapedPixmapWindow`) because of the following call sequence in `QWaylandDrag::startDrag()`:

1. `QBasicDrag::startDrag()` creates the icon window and calls `setVisible(true)`
2. `setVisible()` triggers `updateExposure()` → `sendExposeEvent()` → `paintEvent()` → backing store flush → `wl_surface.attach` + `wl_surface.commit`
3. `dataDevice()->startDrag()` sends `wl_data_device.start_drag(source, origin, icon_surface, serial)`

With **synchronous** delivery (6.10+), the buffer is committed to the icon surface **before** `start_drag`. The Wayland protocol trace shows:

```
wl_surface.attach(wl_buffer, 0, 0)   # buffer committed BEFORE start_drag
wl_surface.damage_buffer(0, 0, 68, 68)
wl_surface.commit()
wl_data_device.start_drag(source, origin, icon_surface, serial)
```

With **asynchronous** delivery (6.9), the paint/commit is deferred to the event loop and happens **after** `start_drag`:

```
wl_data_device.start_drag(source, origin, icon_surface, serial)
wl_surface.offset(-33, -33)           # hotspot applied
wl_surface.attach(wl_buffer, 0, 0)    # buffer committed AFTER start_drag
wl_surface.damage_buffer(0, 0, 68, 68)
wl_surface.commit()
```

Compositors (at least Mutter) expect the icon surface buffer to be committed after `wl_data_device.start_drag()`, not before. A buffer committed before `start_drag` is silently ignored.

Additionally, the asynchronous path also correctly applies the hotspot offset via `wl_surface.offset()` (set by `addAttachOffset(-drag()->hotSpot())` which runs after `startDrag`), whereas the synchronous path commits with offset (0,0) since the hotspot hasn't been set yet.

## Fix

Use asynchronous expose delivery for `QShapedPixmapWindow` in `sendExposeEvent()`, restoring the Qt 6.9 behavior for drag icon surfaces:

```diff
--- a/src/plugins/platforms/wayland/qwaylandwindow.cpp
+++ b/src/plugins/platforms/wayland/qwaylandwindow.cpp
@@ -612,6 +612,15 @@ void QWaylandWindow::sendExposeEvent(const QRect &rect)
     if (sQtTestMode) {
         mExposeEventNeedsAttachedBuffer = true;
     }
+
+    // Drag icon surfaces (QShapedPixmapWindow) need asynchronous expose delivery.
+    // Compositors expect the icon buffer to be committed after wl_data_device.start_drag(),
+    // not before. Asynchronous delivery defers the paint/commit to the event loop,
+    // matching the behavior of Qt 6.9 and earlier.
+    if (window()->inherits("QShapedPixmapWindow")) {
+        QWindowSystemInterface::handleExposeEvent(window(), rect);
+        return;
+    }
     QWindowSystemInterface::handleExposeEvent<QWindowSystemInterface::SynchronousDelivery>(window(), rect);
```

## Related Bugs

- QTBUG-136494 — "Set geometry before creating shell window" (introduced the `setGeometry` reordering in `setVisible`, though not the direct cause)
- QTBUG-98048 — Drag and drop with pixmap laggy on KDE/Wayland (different issue, fixed in 6.5)
- QTBUG-109826 — GUI items behave strangely after drag-and-drop on Wayland
