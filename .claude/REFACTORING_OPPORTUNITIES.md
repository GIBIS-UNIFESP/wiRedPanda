# Refactoring Opportunities

Comprehensive review of the entire codebase for simplification and deduplication opportunities.
All findings validated against actual source code on 2025-04-05.

---

## High Priority

### 1. Merge `ChangeInputSizeCommand` / `ChangeOutputSizeCommand` (Commands.cpp:855-1031)

Constructor, `redo()`, and `undo()` are structurally identical across ~180 lines, differing only in input→output direction. Could be parameterized into a single `ChangePortSizeCommand(bool isInput)`. ~90 lines saved.

### 2. `LoggingUndoCommand` base class (Commands.cpp, 24 methods)

Every `undo()` and `redo()` starts with `qCDebug(zero) << text()`. A base class with `undoImpl()`/`redoImpl()` virtual methods would centralize logging. ~24 lines saved.

### 3. `ElementsCommand` base with ID storage pattern (Commands.cpp, 10+ classes)

`m_ids` + `m_scene` + `CommandUtils::findElements()` is repeated across RotateCommand, MoveCommand, MorphCommand, FlipCommand, ChangeInputSizeCommand, ChangeOutputSizeCommand, UpdateBlobCommand, etc. A shared base with `storeIds()` and `elements()` accessors would clean up all constructors. ~40 lines saved.

### 4. Shared `stripAccents()` (ArduinoCodeGen.cpp:36-42, SystemVerilogCodeGen.cpp:82-88)

Byte-for-byte identical implementations. Move to a shared CodeGen utility. ~7 lines saved.

### 5. Shared `removeForbiddenChars()` (ArduinoCodeGen.cpp:44-57, SystemVerilogCodeGen.cpp:67-80)

Near-identical logic (Arduino calls `stripAccents()` inline; SystemVerilog calls it separately). Move to shared utility with the minor difference parameterized. ~14 lines saved.

### 6. Merge `IC::loadInputElement()` / `loadOutputElement()` (IC.cpp:518-585)

Same structure: create Node proxy, set label, set port properties, re-route connections, delete original. Differ only in direction (output→input ports and vice versa). A single `loadBoundaryElement(elm, isInput)` would halve this. ~30 lines saved.

### 7. Merge `IC::loadInputsLabels()` / `loadOutputsLabels()` (IC.cpp:617-657)

Identical label-building logic: `label + portName + [genericProperties]`. Only differs in which port list is iterated and which label map is written to. Extract `buildPortLabel()` helper. ~18 lines saved.

### 8. Color-to-index mapping duplicated across 4 display elements

Led.cpp:156-167, Display7.cpp:168-177, Display14.cpp:140-149, Display16.cpp:150-159 all implement the same `setColor()` string→index mapping. Led uses different indices (0,2,4,6,8 for off/on pairs) but Display7/14/16 are identical (0,1,2,3,4). Extract to a shared utility or base class method.

### 9. AudioBox / Buzzer — near-identical audio output classes

Both have identical: hardware detection (line 59-67 each), `refresh()` (play/stop guard pattern), `play()`/`stop()` state management, `volume()`/`mute()` accessors, label positioning (`m_label->setPos(64, 34)`). Only the audio backend differs (QMediaPlayer vs ToneGenerator/QAudioSink). An `AudioOutputElement` base class would eliminate ~80 lines of duplication.

### 10. InputSwitch / InputButton — duplicated `setOn(bool)`, `setAppearance()`, `appearanceStates()`

`setOn(bool, int)` at InputSwitch.cpp:74-81 and InputButton.cpp:123-130 are identical. `setAppearance()` at InputSwitch.cpp:134-144 and InputButton.cpp:132-142 are identical. Only mouse behavior differs (toggle vs momentary). Move shared logic to `GraphicElementInput` base class.

---

## Medium Priority

### 11. `ensureFileExtension()` helper (MainWindow.cpp, 5 sites)

The pattern `if (!fileName.endsWith(".ext")) fileName.append(".ext")` appears at lines 739, 777, 1181, 1210, 1818 with different extensions (.panda, .ino, .sv). ~10 lines saved.

### 12. `getSelectedIC()` helper (MainWindow.cpp:1746-1806)

`embedSelectedIC()` and `extractSelectedIC()` both validate selection → check empty → check IC type → cast to `IC*` with near-identical code. ~10 lines saved.

### 13. Merge `setInputSize()` / `setOutputSize()` (GraphicElement.cpp:726-761)

Identical constraint check, grow/shrink logic, and `updatePortsProperties()` call, differing only in which port list is operated on. ~15 lines saved.

### 14. Merge `simUpdateInputs()` / `simUpdateInputsAllowUnknown()` (GraphicElement.cpp:821-870)

Only difference is the error condition on line 832 vs 858. The rest (~20 lines) is duplicated. Parameterize with `bool allowUnknown`. ~20 lines saved.

### 15. `forAllPorts()` or `ports()` accessor (GraphicElement.cpp, 6 methods)

`rotatePorts()` (326-339), `updatePortsProperties()` (380-437), `updateTheme()` (578-595), `itemChange()` (464-474), `highlight()` (912-937) all have two near-identical loops — one for inputs, one for outputs. A combined accessor or helper lambda would clean all of them. ~30 lines saved.

### 16. `isSupportedDropFormat()` helper (Scene.cpp, 3 sites)

Same 4 MIME format strings checked in `dragEnterEvent` (643), `dragMoveEvent` (658), and `dropEvent` (674, 716). ~12 lines saved.

### 17. Split `dropEvent()` (Scene.cpp:669-754)

85-line method handling 3 scenarios (new element drop, clone drag, default). Extract `handleNewElementDrop()` and `handleCloneDrag()` for clarity.

### 18. `ToggleTruthTableOutputCommand` undo == redo (Commands.cpp:1042-1068)

Both methods are byte-for-byte identical. Toggle is self-inverse, so `undo()` can just call `redo()`. ~7 lines saved.

### 19. `MorphCommand::transferConnections()` port loop dedup (Commands.cpp:733-775)

Two loops (input ports 733-752, output ports 755-775) with near-identical structure. Differ in `setEndPort`/`setStartPort` and `startPort`/`endPort` accessors. Extract `transferPortConnections(portCount, isInput)`. ~20 lines saved.

### 20. BeWavedDolphin cell ops dedup (BeWavedDolphin.cpp:619-660)

`on_actionSetTo0_triggered`, `on_actionSetTo1_triggered`, `on_actionInvert_triggered` share the same selection-iterate-setCellValue-run pattern. Extract `applyToSelectedCells(valueFn)`. ~15 lines saved.

### 21. BeWavedDolphin zoom dedup (BeWavedDolphin.cpp:808-845)

Three methods (zoomIn, zoomOut, resetZoom) with identical column iteration structure. Extract `adjustColumnWidths(widthFn)`. ~15 lines saved.

### 22. ICRegistry rollback logic duplicated (ICRegistry.cpp:238-256, 289-304)

`embedICsByFile()` and `extractToFile()` catch blocks have identical rollback: deserialize header, create portMap/context, reload elements from snapshot. Extract `rollbackElementUpdates()` helper. ~12 lines saved.

### 23. Simulation `buildTxMap()` duplicated (Simulation.cpp:374-384, 424-430)

`connectWirelessElements()` and `sortSimElements()` both build a `QHash<QString, GraphicElement*> txMap` with the same Tx-label-to-element logic. Extract to a shared `buildTxMap()` helper. ~10 lines saved.

### 24. QNEInputPort / QNEOutputPort destructor duplication (QNEPort.cpp:195-205, 271-280)

Both destructors drain the connection list with identical loops, differing only in `setEndPort(nullptr)` vs `setStartPort(nullptr)`. Extract to a base class helper. ~8 lines saved.

### 25. Mux / Demux — select line calculation repeated 5 times

The `while ((1 << numSelectLines) < size)` pattern appears in Mux.cpp:55-62, Mux.cpp:161-169, Demux.cpp:68-71, Demux.cpp:87-89, Demux.cpp:194-196. Extract `static int calculateSelectLines(int dataCount)`. ~15 lines saved.

### 26. Mux / Demux — select value encoding duplicated (Mux.cpp:181-185, Demux.cpp:210-214)

Identical bit manipulation loop to decode select line statuses into an integer. Extract to shared utility. ~8 lines saved.

### 27. PropertyShortcutHandler — prev/next mirror methods (PropertyShortcutHandler.cpp:16-170)

`prevMainProperty()` and `nextMainProperty()` are mirror switch statements (~50 lines each) differing only in `+1`/`-1` and `min`/`max` checks. Same for `prevSecondaryProperty()`/`nextSecondaryProperty()`. Could be parameterized with a direction enum. ~60 lines saved.

### 28. QNEConnection `setStartPort()` / `setEndPort()` duplication (QNEConnection.cpp:60-95)

Both follow identical detach-old/attach-new pattern differing only in which port member and which position setter. ~15 lines saved.

---

## Low Priority

### 29. File-based IC visibility helper (MainWindow.cpp:674, 1064)

Same 5 `setVisible()` calls duplicated in `openICInTab()` and `connectTab()`. ~6 lines saved.

### 30. Remove `MoveCommand::m_offset` (Commands.h:165)

Declared but never read or written. Dead code.

### 31. Duplicate `#include <QSaveFile>` (MainWindow.cpp:26-27)

Same include appears twice.

### 32. MainWindowUI action creation boilerplate (MainWindowUI.cpp:27-171)

42+ actions created with the same 3-line pattern (new QAction, setObjectName, setIcon). A `createAction()` helper would reduce repetition but this is generated-style UI code — low value.

### 33. QNEInputPort::updateTheme() is empty (QNEPort.cpp:261-263)

Override does nothing while QNEOutputPort's version (315-320) actually updates colors. Theme changes won't update existing input port colors until `setStatus()` is called again. May be intentional (input port colors are set in `setStatus()`), but worth documenting.

### 34. ICRegistry inconsistent file-watch existence check

`watchFile()` (line 56-58) doesn't check `QFileInfo::exists()` before adding, but `onFileChanged()` (line 102-104) does. Minor inconsistency.

### 35. Element version-aware load boilerplate across 6+ files

InputSwitch, InputButton, AudioBox, Buzzer, Display7, Clock all implement the same pattern: check `hasQMapFormat(version)` → load from QMap, else load legacy bare types. A template helper could reduce this but each element's keys differ.

---

## MCP Server (C++)

### 36. Element validation + retrieval duplicated 15+ times (ElementHandler, ConnectionHandler, SimulationHandler)

The pattern `validatePositiveInteger()` → `validateElementId()` → `itemById()` → `dynamic_cast<GraphicElement*>` → null check is repeated across virtually every handler method. ElementHandler.cpp alone has it at lines 142-153, 254-259, 308-313, 614-617, 673-676, 738-742, 791-795, 844-852. Extract to `BaseHandler::getValidatedElement(params, "element_id", errorMsg, requestId)`.

### 37. Exception handling boilerplate in 30+ handler methods

Every handler method wraps its logic in identical try/catch:
```cpp
} catch (const std::exception &e) {
    return createErrorResponse(QString("Failed to X: %1").arg(e.what()), requestId);
} catch (...) {
    return createErrorResponse("Failed to X: Unknown exception", requestId);
}
```
Extract to a helper or use a wrapper function that takes a lambda.

### 38. `getAvailableInputPorts()` / `getAvailableOutputPorts()` are near-clones (BaseHandler.cpp:266-316)

Identical loop logic differing only in `inputPort(i)`/`outputPort(i)` and `inputSize()`/`outputSize()`. Parameterize with `bool isOutput`. ~25 lines saved.

### 39. Port resolution duplicated for source/target (ConnectionHandler.cpp:87-125)

Source port resolution (lines 89-105) and target port resolution (107-125) have identical structure differing only in label/index parameter names and input vs output direction. Extract `resolvePort()` helper.

### 40. MCPProcessor command routing is a long if-else chain (MCPProcessor.cpp:145-174)

30 commands routed via string comparison chain. A `QHash<QString, Handler*>` dispatch map would be cleaner and more maintainable.

---

## MCP Client (Python)

### 41. Port parameter building is repetitive (mcp_circuit_builder.py:74-81)

Four near-identical `if "key" in conn_spec: params["key"] = conn_spec["key"]` lines. Use a loop over the key names.

### 42. Clock transition logic duplicated for rising/falling/level (mcp_circuit_builder.py:183-209)

Three branches with identical `send_command`/`sleep` patterns differing only in True/False values. Parameterize with a value-sequence lookup.

### 43. Input-setting and output-checking loops duplicated between truth table and state table validation (mcp_circuit_builder.py:96-145, 168-209)

Both `validate_truth_table` and `validate_sequential_state_table` repeat the same input-apply and output-check loops. Extract `_apply_input_values()` and `_check_output_values()`.

### 44. `validation_warnings` counter is dead code (mcp_infrastructure.py:43)

Initialized to 0 but never incremented or read anywhere.

---

## Flip-Flops and Logic Gates

### 45. Flip-flop `updateTheme()` identical across 6 files

DFlipFlop.cpp:73-78, DLatch.cpp:62-67, JKFlipFlop.cpp:79-84, SRFlipFlop.cpp:77-83, SRLatch.cpp:62-67, TFlipFlop.cpp:73-78 all contain identical `setPixmap(pixmapPath()); GraphicElement::updateTheme();`. Move to a shared base class.

### 46. Flip-flop output initialization identical across 6 files

All 6 flip-flop/latch constructors set `outputPort(0)->setDefaultStatus(Status::Inactive); outputPort(1)->setDefaultStatus(Status::Active);`. Extract to shared init.

### 47. Preset/Clear async override logic identical in 4 flip-flops

DFlipFlop.cpp:97-100, JKFlipFlop.cpp:111-114, SRFlipFlop.cpp:108-111, TFlipFlop.cpp:99-102 all contain the same `if (prst == Status::Inactive || clr == Status::Inactive)` block with identical Q/Q-bar assignments. Extract to utility function.

### 48. Clock rising-edge detection repeated in 4 flip-flops

`if (clk == Status::Active && m_simLastClk == Status::Inactive)` at DFlipFlop.cpp:92, JKFlipFlop.cpp:99, SRFlipFlop.cpp:98, TFlipFlop.cpp:92. Extract `isClockRisingEdge()` helper.

---

## Enums and Core

### 49. `nextElmType()` / `prevElmType()` are reverse-mirror switch statements (Enums.cpp:12-88)

Two 37-line switch statements defining forward and reverse morph cycles. The reverse is derivable from the forward map. Use a data-driven approach (array or map) as single source of truth. ~40 lines saved.

---

## Element Infrastructure

### 50. ElementLabel MIME serialization duplicated (ElementLabel.cpp:85-109, 111-123)

`startDrag()` and `mimeData()` build identical `QByteArray` with the same stream writes. Extract `serializeDragData()` helper. ~10 lines saved.

---

## UI Utilities

### 51. MIME type strings hardcoded across 4+ files (TrashButton, ICDropZone, Scene, ClipboardManager)

The strings `"wpanda/x-dnditemdata"` and `"application/x-wiredpanda-dragdrop"` appear in 8+ locations. Define as constants in a shared header.

### 52. ClockDialog / LengthDialog are structurally identical

Both have identical: constructor pattern, single-value accessor, -1-on-cancel behavior, slider-spinbox sync. A `NumericInputDialog` base or template would eliminate the duplication.

### 53. ICDropZone drag payload extraction duplicated (ICDropZone.cpp:27-40, 60-80)

`dragEnterEvent` and `dropEvent` both deserialize the same MIME data with identical stream reads. Extract `extractDragPayload()` helper.

---

## MCP Server

### 54. HandlerFactory::registerBuiltInHandlers() — 25 identical lambda registrations (HandlerFactory.cpp:99-185)

Each registration creates the same lambda differing only in handler type. Use a template helper `registerHandlerGroup<HandlerType>(category, commandList)` to collapse ~85 lines into ~5 calls.

### 55. MCP Python duplicate model definitions (domain/models.py vs type_models.py)

`CircuitElement`, `CircuitConnection`, `CircuitSpecification` are defined in both files with near-identical structure. Use one canonical definition.

---

## MCP Tests

### 56. Element creation + validation pattern repeated ~40 times across all test files

The create-element → validate-response → extract-id pattern is duplicated in every test. Extract `create_simple_circuit()` and `create_element_checked()` helpers to MCPTestBase.

### 57. Truth table test methods are near-clones (test_logic_gates.py)

AND, OR, XOR tests (~20 lines each) follow identical structure differing only in gate type and expected outputs. A parametrized `test_logic_gate_truth_table()` would reduce ~180 lines to ~30.

### 58. test_element_type_coverage.py — 5 identical test methods (lines 55-250)

Five methods with identical create-element loops differing only in the element type list. Parametrize into one method. ~150 lines saved.
