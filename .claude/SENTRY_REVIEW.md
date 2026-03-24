# Sentry Integration Review — wiRedPanda

**Date**: 2026-03-20
**Reviewed version**: current `tests` branch

---

## Current State

The Sentry integration is minimal: init at startup + exception capture in `Application::notify`. Two code locations and one CMake block:

| File | Lines | Purpose |
|------|-------|---------|
| `App/Main.cpp` | 35-50 | `sentry_init()` with DSN, release, database path; `sentry_close()` via `qScopeGuard` |
| `App/Core/Application.cpp` | 37-53 | Catches `std::exception` in `notify()`, creates Sentry event with English message + stack trace |
| `CMakeLists.txt` | 357-366 | `find_path` for `sentry.h`, defines `HAVE_SENTRY`, links `sentry` library |

All code is gated behind `#ifdef HAVE_SENTRY`. The `Pandaception` exception class carries both translated and English messages so Sentry reports are always in English.

---

## Issues Found

### 1. No user identification

All crash reports show `user: null`. We had to fingerprint users by OS version + geo-IP to distinguish machines. This makes it impossible to:
- Count how many unique users are affected
- Correlate multiple issues to the same user session
- Reach out for reproduction steps

**Fix**: Generate a random anonymous ID on first launch and persist it via `QSettings`:

```cpp
// After sentry_init(options) in Main.cpp
auto userId = QSettings().value("sentry/userId").toString();
if (userId.isEmpty()) {
    userId = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSettings().setValue("sentry/userId", userId);
}
sentry_value_t user = sentry_value_new_object();
sentry_value_set_by_key(user, "id", sentry_value_new_string(userId.toStdString().c_str()));
sentry_set_user(user);
```

**Priority**: High — directly impacts triage and impact assessment.

---

### 2. No breadcrumbs

Zero breadcrumbs in any crash report. For WIREDPANDA-FA we had to manually reconstruct the sequence `cloneDrag → QDrag::exec → timer → Simulation::update` from the stack trace alone. Breadcrumbs would have shown the exact user actions leading up to the crash.

**Key places to add `sentry_add_breadcrumb()`**:
- File open / save / autosave
- Simulation start / stop / pause
- Undo / redo commands (`Scene::receiveCommand`)
- Drag operations (`cloneDrag`, `startDrag`, `dropEvent`)
- Tab switches
- Element creation / deletion
- Waveform dialog open

**Priority**: High — most impactful improvement for debugging.

---

### 3. No custom tags

The only tags are auto-set by the SDK: `release`, `os`, `environment`, `level`. Adding custom tags would improve filtering in Sentry:

| Tag | Source | Why |
|-----|--------|-----|
| `qt.version` | `QT_VERSION_STR` | Identify Qt-specific regressions |
| `app.interactive` | `Application::interactiveMode` | Distinguish GUI vs headless/test runs |
| `file.format_version` | Current `.panda` format version | Correlate crashes with file format issues |

```cpp
sentry_set_tag("qt.version", QT_VERSION_STR);
```

**Priority**: Medium — improves filtering and triage.

---

### 4. Database path is relative

`Main.cpp:43` sets the crash database to `".sentry-native"` (relative to CWD). If the user launches wiRedPanda from different directories, the crash database is created in different places — queued crash reports from a previous session may never be flushed.

**Fix**: Use an absolute path under the app's data directory:

```cpp
const auto dbPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/sentry-native";
sentry_options_set_database_path(options, dbPath.toStdString().c_str());
```

**Priority**: Medium — affects crash report delivery reliability.

---

### 5. No environment configuration

All events show `environment: production`. There's no `sentry_options_set_environment()` call. This means debug builds, CI test runs, and release builds all report to the same environment.

**Fix**:

```cpp
#ifdef QT_DEBUG
sentry_options_set_environment(options, "development");
#else
sentry_options_set_environment(options, "production");
#endif
```

**Priority**: Medium — prevents dev/test noise from polluting production data.

---

### 6. Exception stack traces are from the catch site, not the throw site

In `Application::notify` (Application.cpp:50):
```cpp
sentry_value_set_stacktrace(exc, NULL, 0); // captures stack at THIS point
```

The stack trace is captured in the `catch` block, not at the `throw` site. This is why exception reports (F7, ET, F9) all show `sentry_new_function_transport` or `sentry__unwind_stack_libbacktrace` as the culprit instead of the actual code that threw. The real throw location is lost.

This is a fundamental limitation of the sentry-native C SDK for caught exceptions. The minidump backend (crashpad/breakpad) only captures full stack traces for actual crashes (SIGSEGV, etc.), not for caught C++ exceptions.

**Possible mitigations**:
- Capture the stack trace at the throw site inside the `Pandaception` constructor using platform-specific APIs (`CaptureStackBackTrace` on Windows, `backtrace()` on Linux/macOS), then pass the stored instruction pointers to `sentry_value_set_stacktrace(exc, ips, len)` in the catch block.
- Alternatively, include the source file and line number in the Sentry exception message via the `PANDACEPTION` macro (using `__FILE__` and `__LINE__`).

**Priority**: Medium — current exception reports are hard to triage without this.

---

### 7. No deduplication or rate limiting for exceptions

WIREDPANDA-F7 produced 10 identical events in 12 seconds (a timer loop retrying the same broken pixmap). This wastes Sentry quota and creates noise.

**Fix**: Use `sentry_options_set_before_send()` to deduplicate or rate-limit:

```cpp
sentry_value_t beforeSend(sentry_value_t event, void *hint, void *closure)
{
    // Simple rate limit: skip if same message was sent within last N seconds
    // Or track last message hash and suppress duplicates
    return event; // return event to send, or sentry_value_new_null() to drop
}

sentry_options_set_before_send(options, beforeSend, nullptr);
```

Alternatively, set a sample rate for non-fatal exceptions:
```cpp
sentry_options_set_sample_rate(options, 1.0); // 100% for crashes, handle exceptions separately
```

**Priority**: Low — mostly a quota/noise issue.

---

### 8. No extra context on events

Exception events carry only the error message. Adding context about the current scene state would help debugging:

```cpp
sentry_set_extra("scene.elementCount", sentry_value_new_int32(scene->elements().size()));
sentry_set_extra("scene.connectionCount", sentry_value_new_int32(scene->connections().size()));
sentry_set_extra("simulation.running", sentry_value_new_bool(simulation->isRunning()));
sentry_set_extra("file.path", sentry_value_new_string(currentFile.toStdString().c_str()));
```

This could be updated periodically or set in `Application::notify` before capturing the event.

**Priority**: Low — nice to have for complex debugging scenarios.

---

### 9. No `sentry_set_context` for device/runtime info

The minidump backend auto-populates some OS info for fatal crashes, but caught exceptions get almost no context. Explicitly setting contexts at init:

```cpp
sentry_value_t os_context = sentry_value_new_object();
sentry_value_set_by_key(os_context, "name", sentry_value_new_string(QSysInfo::productType().toStdString().c_str()));
sentry_value_set_by_key(os_context, "version", sentry_value_new_string(QSysInfo::productVersion().toStdString().c_str()));
sentry_value_set_by_key(os_context, "kernel_version", sentry_value_new_string(QSysInfo::kernelVersion().toStdString().c_str()));
sentry_set_context("os", os_context);
```

**Priority**: Low — mostly redundant for minidump crashes, useful for exceptions.

---

## Summary

| # | Issue | Priority | Effort | Status |
|---|-------|----------|--------|--------|
| 1 | No user identification | **High** | Small | **Fixed** |
| 2 | No breadcrumbs | **High** | Medium | **Fixed** |
| 3 | No custom tags | Medium | Small | **Fixed** |
| 4 | Database path is relative | Medium | Small | **Fixed** |
| 5 | No environment configuration | Medium | Small | **Fixed** |
| 6 | Exception stack traces from catch site | Medium | Medium | **Fixed** |
| 7 | No deduplication / rate limiting | Low | Medium | **Fixed** |
| 8 | No extra context on events | Low | Medium | Open |
| 9 | No device/runtime context for exceptions | Low | Small | Open |

Items 1–7 have been implemented. Items 8 and 9 remain as future improvements.
