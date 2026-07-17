# Release 5.2.0 review — commits since 5.1.3

Tracking file for a deep review of every commit between the last real release
(`5.1.3`, 2026-06-24) and current `HEAD` (tagged `5.2.0-rc1`), before promoting
the RC to a final release. See the plan at
`/home/torres/.claude/plans/can-you-deep-review-sprightly-kazoo.md` for full
methodology.

- Range: `5.1.3..HEAD`
- Total commits: 393
- Excluded (translation-only, listed below for transparency, not reviewed): 63
- Reviewable: 330

## Excluded commits (translation/i18n sync, not hand-reviewed)

<details>
<summary>63 commits — Weblate syncs + lupdate scaffolding</summary>

- `dbbd5db8c` Update translation sources
- `498ff6af3` Update translation sources
- `96a42df35` Update translation sources
- `cf39bea34` Update translation sources
- `1371ae13a` Update translation sources
- `aad23dec0` Update translation sources
- `93aa8b139` Update translation sources
- `a9596bfed` i18n: translate UI strings and exercise/tour catalog into all 38 languages
- `99c3b322f` i18n: mark English source strings as finished
- `48958e727` Update translation sources
- `b26523e03` Update translation sources
- `ec047c019` Update translation sources
- `994632a49` Update translation sources
- `d0dcbb9b9` Update translation sources
- `247ab9b28` Update translation sources
- `5fb2e2e83` Update translation sources
- `1cdcd3369` Update translation sources
- `7167d5039` Update translation sources
- `731220e52` Translate new UX-audit UI strings across all 39 language catalogs
- `fcd620c9d` Update translation sources
- `a8ddf2046` Translated using Weblate (Arabic)
- `a304578dc` Translated using Weblate (Bulgarian)
- `634da24c1` Translated using Weblate (Bengali)
- `11c1e813c` Translated using Weblate (Czech)
- `6dc8183ac` Translated using Weblate (Danish)
- `49483b23a` Translated using Weblate (German)
- `e8c3c3b2f` Translated using Weblate (Greek)
- `603315eea` Translated using Weblate (Spanish)
- `b8a9a93cd` Translated using Weblate (Estonian)
- `6e950ae74` Translated using Weblate (Persian)
- `61e2e7c9e` Translated using Weblate (Finnish)
- `89d92db21` Translated using Weblate (French)
- `70b738a74` Translated using Weblate (Hebrew)
- `f9f7fce82` Translated using Weblate (Hindi)
- `abaf742f2` Translated using Weblate (Croatian)
- `57c08448d` Translated using Weblate (Hungarian)
- `41957f9c8` Translated using Weblate (Indonesian)
- `4cf635092` Translated using Weblate (Italian)
- `dce7a6122` Translated using Weblate (Japanese)
- `2351c355e` Translated using Weblate (Korean)
- `1de6fb880` Translated using Weblate (Lithuanian)
- `779de4408` Translated using Weblate (Latvian)
- `54f351a37` Translated using Weblate (Malay)
- `5f1a6dc3e` Translated using Weblate (Norwegian Bokmål)
- `b6fd487f2` Translated using Weblate (Dutch)
- `d1311f151` Translated using Weblate (Polish)
- `983c0900a` Translated using Weblate (Portuguese)
- `39eca5d2b` Translated using Weblate (Portuguese (Brazil))
- `51b352612` Translated using Weblate (Romanian)
- `1519b896c` Translated using Weblate (Russian)
- `75b18b071` Translated using Weblate (Slovak)
- `7c9c8526c` Translated using Weblate (Swedish)
- `15af064bf` Translated using Weblate (Thai)
- `bed654114` Translated using Weblate (Turkish)
- `cee187b86` Translated using Weblate (Ukrainian)
- `83b23a920` Translated using Weblate (Vietnamese)
- `ed4739c7c` Translated using Weblate (Chinese (Simplified Han script))
- `d5829ada8` Translated using Weblate (Chinese (Traditional Han script))
- `f86bb02e8` Update translation sources
- `daef4a258` Update translation sources
- `72bb5ca6a` Update translation sources
- `542600e38` Update translation sources
- `ae0bb2d9a` Update translation sources

</details>

## Reviewable commits (330)

Chronological order (oldest first). Category tag from the conventional-commit
prefix. Checked off as reviewed; grouped commits (e.g. a serialization-format
chain) are reviewed together and checked off together.

- [x] `2fdd2f54b` **ci** ci(deps): bump actions/cache from 5 to 6
- [x] `88889e13d` **fix** fix(codegen): faithful feedback-IC settling in Arduino export + testbench (F63)
- [x] `2b1c224cd` **chore** chore(goldens): baseline-regenerate fixtures in current environment
- [x] `b3d12eb03` **fix** fix(simulation): order-independent topological priorities for DAGs (F1)
- [x] `d92020354` **fix** fix(elements): enforce TruthTable 2048-bit key invariant in setkey (F2)
- [x] `905b79835` **fix** fix(elements): Mux/Demux out-of-range select yields Unknown (F3)
- [x] `2992e058d` **fix** fix(elements): setInputs restores the port index invariant (F18)
- [x] `47a7bde25` **fix** fix(scene): context-menu Paste accepts current clipboard format (F4)
- [x] `07edfa0df` **fix** fix(render): wire paths in MCP modes via dedicated rendering flag (F5)
- [x] `886041d1d` **fix** fix(ui): recovered autosaves count as modified on exit (F6)
- [x] `105787712` **fix** fix(app): runtime observation batch from the deep review (F36-F43)
- [x] `9054ae0cc` **fix** fix(app): low-severity batch from the deep review (F8-F16 App slice)
- [x] `542af9174` **fix** fix(mcp): low-severity batch from the deep review (F21 MCP slice)
- [x] `35dccc237` **fix** fix(mcp): bounds-check toggle_truth_table_output position (F17)
- [x] `4abba0c0c` **fix** fix(mcp): observation batch from the deep review (F44-F47)
- [x] `ba67c5668` **refactor** refactor(mcp): event-driven stdin reader, clean shutdown on EOF
- [x] `4c1e19d63` **feat** feat(ic): hover preview shows IC filename and skips IC ports
- [x] `61c046571` **fix** fix(build): Windows version metadata follows project version (F27)
- [x] `7c6128578` **ci** ci(coverage): measure MCP/Server C++ coverage via the MCP suite
- [x] `7a2660930` **fix** fix(freedesktop): single canonical MIME type + explicit binary fixtures
- [x] `52d6d3b43` **feat** feat(sim): non-blocking sequential semantics; resolve multi-cycle CPU SV divergence
- [x] `d858659ac` **fix** fix(generators): generator circuit fixes part 1 (F26, F31)
- [x] `23fe5aa33` **fix** fix(generators): generator circuit fixes part 2 (F26, F32-F35)
- [x] `518d91517` **refactor** refactor(generators): textbook L1 flip-flops — 7474 D-FF and 7476 master-slave JK (F55, F56)
- [x] `d046ce6e9` **feat** feat(level4): full 74161 control set on the 4-bit binary counter (F101)
- [x] `22ae5a649` **fix** fix(level2): drop dead OR-of-8 gate in priority encoder (F58)
- [x] `577dd059a` **feat** feat(level4): count-enable + parallel load on johnson/ring counters (F102)
- [x] `592ac41fb` **refactor** refactor(level3): build alu_selector_5way from 4x level2_mux_2to1 (F96)
- [x] `83e450303` **feat** feat(level3): add 74LS85 cascade inputs to the 4-bit comparator (F98)
- [x] `deb54da3a` **feat** feat(level4): add async Reset to ram_4x1/8x1 for RAM-family parity (F54)
- [x] `6fbe6dfeb` **feat** feat(level9): real 16-bit fetch stage — IR, programming port, raw fields (F53)
- [x] `1ba5cf713` **fix** fix(level5): implement the 4-bit PC's reset and fix load/inc priority (F52)
- [x] `40ea40e73` **fix** fix(level7): chain 16-bit ALU shifts across the byte boundary (F61)
- [x] `48425b602` **feat** feat(level6): add async Reset to ram_8x8 for RAM-family parity (F54)
- [x] `55474e0d4` **feat** feat(level8/9): complete decode/memory stages and make the CPUs programmable (F54, F80-F82)
- [x] `571bbe3e7` **fix** fix(codegen): TruthTable multi-output, SV rotary inputs, SV open-failure throw (F19, F23, F22)
- [x] `eaa872724` **refactor** refactor(codegen): SystemVerilog exporter is a pure structural translator
- [x] `78dc3302a` **fix** fix(codegen): JK SystemVerilog export uses falling edge to match the gate circuit (F67-F68)
- [x] `a2eb28e8a` **test** test(codegen): seeded-random engine-differential stimulus for sequential SV export
- [x] `471612970` **fix** fix(arduino): globally-unique variable names + recursive testbench state decls
- [x] `e8aab7e0b` **feat** feat(arduino): non-blocking flip-flop semantics + clocked CPU validation
- [x] `a0808ccae` **test** test(arduino): skip multi-cycle CPU testbench when arduino-cli/simavr absent
- [x] `0fa25d28a` **test** test(level1): cover JK async-under-clock-high and contention inputs (F65-F66)
- [x] `7927d157d` **test** test(level1): exhaustive D flip-flop Q_bar and async-override-clock coverage
- [x] `d424017b7` **test** test(level3): cover ALU-selector op2-dominant don't-care opcodes
- [x] `98008980c` **test** test(level4): cover ram_8x1 async Reset (F54)
- [x] `74fdff708` **test** test(level5): assert modulo-counter Overflow output
- [x] `b028aef38` **test** test(level5): exercise register-file second read port
- [x] `1cf9cd6ae` **test** test(level5): exercise 8x8 register-file second read port
- [x] `b61b3fc08` **test** test(level6): cover stack-memory LIFO round-trip
- [x] `705928fd8` **test** test(level7): assert 16-bit ALU flags and cover NOT (F69-F70)
- [x] `9e8392025` **test** test(level7): exercise CPU program counter increment (F71)
- [x] `f9876ac7d` **test** test(level7): cover instruction-memory write/read round-trip (F72)
- [x] `1e41438b0` **test** test: triage placeholder assertions and tautological tests (F24, F28-F30)
- [x] `25edc83cf` **test** test(level9): share the 8-bit CPU ISA header (fixes mega-unity ODR)
- [x] `03bf6d1f4` **fix** fix(level9): update Arduino multi-cycle test for the F80 programming interface (F93)
- [x] `f00b291e7` **other** docs/chore: fix docs drift, config, and dead assets (F48-F51)
- [x] `45e9c6c4e` **test** test(level4): cover ram_4x1 async Reset (F54 RAM-family parity)
- [x] `a16cc78dc` **test** test(level6): align stack-pointer tests with fixed circuits (F26, F31-F35)
- [x] `8d657a6fa` **test** test(level7): add per-bit port-isolation coverage to datapaths (F73)
- [x] `6f6fc39e9` **test** test(level7): cover register hold paths in PC and instruction register (F74)
- [x] `f405a7b08` **test** test(level8): add per-bit data-path isolation to the pipeline stages (F78)
- [x] `fea396674` **test** test(level8): assert fetch-stage hold path (F79)
- [x] `f13503a39` **test** test(level5+inline): cover clock-gated decoder writeEnable gating and use the real simple_and AND-gate inline fixture
- [x] `f69bcd15a` **test** test(level6): cover 8-bit program-counter reset and stack-memory data paths
- [x] `00b886de9` **test** test(level2): drop dead using in decoder_4to16 test
- [x] `51906af1a` **refactor** refactor(generators): unify IC loading via bare-name instantiate_ic
- [x] `f540ffbf0` **feat** feat(level2): add 74148 EI/EO cascade ports to the priority encoder (F99)
- [x] `0e93d3f46` **feat** feat(level2): add 74180 cascade-in to parity generator and checker (F100)
- [x] `25c9f30c7` **feat** feat(level2): add active-high Enable to the binary decoders (74138-style)
- [x] `c4c69819a` **feat** feat(level2): add active-high Enable to the multiplexers (74153-style strobe)
- [x] `1a9a72bdb` **refactor** refactor(convention): standardize control-port casing to PascalCase (F105)
- [x] `8103964fb` **docs** docs(generators): fix stale Usage-line filenames in module docstrings
- [x] `08fc23d0c` **feat** feat(scene): show connected ports' labels when hovering a port
- [x] `d870164d9` **feat** feat(ui): add a View menu option to enable/disable the IC hover preview
- [x] `e2b302c0f` **fix** fix(ci): enable ENABLE_MCP_SERVER for coverage builds regardless of branch
- [x] `36d8c62c9` **fix** fix(fuzz): resolve -Wsign-conversion errors indexing QList with FuzzedDataProvider
- [x] `7b4f9459b` **fix** fix: default port status to Unknown and fix updateTheme for input ports
- [x] `a8a5c1130` **fix** fix: make port colors uniformly reflect signal status
- [x] `d4412cc67` **feat** feat: distinguish input/output ports by shape
- [x] `19412488d` **fix** fix: update unconnected output ports in simulation Phase 3
- [x] `de24e0e2e` **fix** fix: reapply wire status pen when the theme changes
- [x] `07efc1477` **fix** fix(tests): eliminate defects and vacuous assertions found in deep test review
- [x] `41f171601` **refactor** refactor: lifecycle — Q_DISABLE_COPY_MOVE, delete invariant-breaking copy ctors
- [x] `a1906d9e8` **style** style: drop get-prefix from public accessor APIs
- [x] `96b0e2c21` **refactor** refactor: const-correctness for accessor methods
- [x] `dd285ba3b` **refactor** refactor: switch exhaustiveness and enum underlying types
- [x] `b9a85aa7e` **refactor** refactor: QMap → QHash where insertion ordering is not required
- [x] `eb4147bbf` **refactor** refactor: QPair → std::pair, QList<QString> → QStringList
- [x] `b861224d5` **refactor** refactor: validate inputs in MCP element/file/theme/simulation handlers
- [x] `b79ef9104` **refactor** refactor: input validation, logging, and null guards across IO/BeWavedDolphin/MCP
- [x] `fd90ffb3f` **refactor** refactor: network hardening — transfer timeout, ssl errors, reply size cap, https URLs
- [x] `7f0c64831` **style** style: parenthesize (std::min) / (std::max) for Windows MSVC compatibility
- [x] `9a33cbf3c` **refactor** refactor: ownership — unique_ptr<QMimeData> and unique_ptr<StdinReader>
- [x] `8b49da0df` **refactor** refactor: thread-safety — Q_ASSERT thread-affinity in shared caches and managers
- [x] `7ec4e6cde` **refactor** refactor: thread-safety — queued connections and atomic stop flag
- [x] `abae69006` **perf** perf: hoist regex compilation, reuse findChildren, eliminate hash double-lookup
- [x] `357c057c9` **refactor** refactor: copy-init, const correctness residuals, .size(), and removeForbiddenChars perf
- [x] `09cd66798` **fix** fix(simulation): snapshot topology vectors in update() before iteration
- [x] `f4f886ff8` **fix** fix(io): probe-based header detection in readPandaHeader / readDolphinHeader
- [x] `78484d8a2` **fix** fix(ic): close qScopeGuard double-free in IC load paths
- [x] `98e5982d9` **ci** ci(codeql): disable Unity/PCH for the C++ build so paths-ignore actually works
- [x] `7fa304338` **refactor** refactor(layering): move gridSize/resolveContextDir to Core, drop dead Scene includes
- [x] `eab30aaf2` **refactor** refactor(layering): drive IC hover preview via signals, not MainWindow
- [x] `b19135401` **refactor** refactor(layering): remove Application's MainWindow member
- [x] `5b437926c` **refactor** refactor(layering): inject SimulationHost interface into Simulation
- [x] `242f5bb2f` **refactor** refactor(layering): relocate ICRegistry from Element/ to Scene/
- [x] `11922c42e` **refactor** refactor(layering): RAII auto-unregister for ItemWithId; drop Nodes->Scene coupling
- [x] `68245bf61` **refactor** refactor(layering): make WorkSpace::save pure; move file dialogs to WorkspaceManager
- [x] `35fbd0d4f` **refactor** refactor(mcp): split SimulationHandler into Simulation + IC + History handlers
- [x] `6c0ed41ad` **refactor** refactor(element): extract ElementOrientation from GraphicElement
- [x] `3fc03ef0e` **refactor** refactor(element): cache ElementMetadata reference in GraphicElement
- [x] `1774418e0` **refactor** refactor(element): extract ElementPorts from GraphicElement
- [x] `6241be661` **refactor** refactor(element): promote GraphicElementSerializer to a real collaborator class
- [x] `381477c00` **refactor** refactor(element): decompose IC into ICLoader + ICRenderer + ICSimulation
- [x] `b3939192b` **refactor** refactor(wiring): integrate the imported QNE port/connection classes
- [x] `7b4b784a8` **refactor** refactor(wiring): centralize port serial-id formula, drop dead vestiges
- [x] `8ee5d6ce3` **refactor** refactor(ui): dedupe the File-Conflict dialog in WorkspaceManager
- [x] `aa776cdf1` **fix** fix(ic-gen): extract generator dependencies via ast, not line toggling
- [x] `961c78904` **test** test(ic): replace tautological settling-time checks with one-hot decode
- [x] `eec8324fc` **test** test(arduino): classify gate-level feedback circuits as sequential
- [x] `9d906cc16` **fix** fix(cpu): control-unit reset clears state, control signals decode per-state
- [x] `c0da782be` **test** test(cpu): remove dead 'enable' param from PC/IR helpers; real flag persistence
- [x] `fc227e4af` **fix** fix(examples): correct JK->D wiring and set safe defaults in jkflipflop.panda
- [x] `476292d0e` **style** style: fix include ordering in ConnectionManager and PortHoverLabel
- [x] `e698cd1df` **fix** fix(ci): make the AppImage dynamic loader executable for no-FUSE hosts
- [x] `f1a5b8cad` **fix** fix(element): rotation must not clobber procedural render pixmaps
- [x] `08581be06` **ci** ci(appimage): dump loader diagnostics when the no-FUSE release check fails
- [x] `34839f334` **fix** fix(element): keep decorative icons and name labels upright when rotating
- [x] `743a989a8` **fix** fix(ci): run the no-FUSE AppImage check with a clean environment
- [x] `dd3d1c98d` **fix** fix(scene): property-only edits must not reinitialize the simulation
- [x] `876e43188` **fix** fix: InputSwitch toggle no longer bypasses undo and dirty tracking
- [x] `0420a15ba` **docs** docs(fuzz): document the full 12-harness suite and tooling
- [x] `a3155b7ad` **build** build(presets): add fuzz_undo to fuzzer build targets
- [x] `7b0f11dfd` **test** test(fuzz): pin regression reproducers bugD-I + oom_* under ASan
- [x] `da28b7e95` **fix** fix(serial): reject non-finite element positions on load
- [x] `b89aaf5ea` **fix** fix(serial+ic): reject non-finite element rotation; harden IC preview/pixmap
- [x] `5cc1f9186` **style** style: fix include ordering across App/Tests/MCP
- [x] `c481e8b0b` **feat** feat(exercise): circuit-challenge Exercise system
- [x] `a859d7911` **feat** feat(tour): guided UI Tour system
- [x] `4830e2a3a` **fix** fix(mcp): accept absolute paths in instantiate_ic for headless automation clients
- [x] `43541ef92` **test** test(ic): add TestICFixtureLayout oracle for element-overlap regressions
- [x] `8866c8df9` **feat** feat(mcp): expose real element/IC geometry from create_element and instantiate_ic
- [x] `1ad22c0a5` **fix** fix(generators): eliminate element overlap across IC fixtures
- [x] `fc70cf321` **test** test(ic): extend TestICFixtureLayout to catch label overlaps
- [x] `686938f0b` **fix** fix(generators): clear label collisions across 19 IC fixtures
- [x] `8a550aa19` **fix** fix(generators): content-aware band gaps for level9 CPU generators
- [x] `5ca1dd46b` **fix** fix(generators): realign per-bit hold/load-mux/FF rows in the 4-bit counters
- [x] `a9c50d1f1` **fix** fix(generators): reposition shared-control fan-out and dense-fixture collisions
- [x] `731dfc693` **fix** fix(generators): widen label clearance margins for cross-platform font metrics
- [x] `a18475826` **other** (feat) [MinimapWidget, Workspace, Settings, MainWindow, Tests] :: add resizable minimap overview widget
- [x] `4b0a22af5` **feat** feat(exercise): add exercises for combinational circuits
- [x] `d432be66e` **feat** feat(exercise): add exercises for sequential circuits
- [x] `4f2bd5a9e` **fix** fix(build): eliminate cast-align violations on armhf/riscv64 (#453)
- [x] `0b1fd3e18` **fix** fix(test): silence -Wconversion in new dolphin header test
- [x] `0aacb3adb` **docs** docs: switch star history chart to sealed-token endpoint
- [x] `7cb03d3fa` **fix** fix(exercise,tour): retranslate overlay text live on language change
- [x] `52500b40d` **fix** fix(test): check QFile::open() return value in tour/exercise fixtures
- [x] `ea6ada857` **fix** fix(io): bound embedded-IC blob registry deserialization on file load
- [x] `fee8b54d8` **ci** ci: work around flaky packages.microsoft.com apt mirror before installing Qt
- [x] `8cbee0575` **fix** fix(build): install Examples/MIME and fix native-install content discovery
- [x] `ac62d81e0` **fix** fix(freedesktop): drop second main category from wiredpanda.desktop
- [x] `bdb49a3ca` **ci** ci: smoke-test cmake --install on every build matrix entry
- [x] `172e136d8` **fix** fix(ci): package the Linux AppImage via cmake --install
- [x] `e94db4971` **fix** fix(ci): package Windows portable ZIP via cmake --install
- [x] `e511ab95c` **feat** feat(build): add real macOS .app bundle support via MACOSX_BUNDLE
- [x] `a80a4245c` **ci** ci: add workflow_dispatch dry-run mode to deploy.yml
- [x] `49ee38ad1` **feat** feat(packaging): add AUR PKGBUILD and automated publish workflow
- [x] `859ee7937` **ci** ci: add functional launch checks to Windows/macOS release validation
- [x] `f421aff5a` **fix** fix(ci): stop splicing untrusted github.head_ref into a translate.yml script
- [x] `a54547edf` **ci** ci: add actionlint validation to lint.yml
- [x] `bfa55721c` **fix** fix(ci): fail hard on missing Examples in Windows/macOS release validation
- [x] `f462c0a84` **fix** fix(ic): give the 16-bit RISC CPU a real register file, decode stage, and data memory
- [x] `f871366f9` **test** test(ic): add dedicated coverage for the register file, memory stage, and decoder
- [x] `4fbfb3532` **fix** fix(ic): make RegisterFileBuilder's decoder dependency visible to the generator scanner
- [x] `b045210b3` **fix** fix(ic): make register-file/bus-mux/counter builder dependencies visible to the generator scanner
- [x] `1cf717ef1` **fix** fix(ic): tie off unused ALU OperandA bits in the 16-bit RISC CPU
- [x] `9e58fcf5f` **test** test(ic): add SystemVerilog export coverage for the 32x16 register file and 16-bit memory stage
- [x] `b7a89ceb3` **test** test(ic): strengthen the multi-cycle CPU's chained-instruction regression check
- [x] `8a662ee36` **test** test(ic): reuse one fetch-stage fixture instead of rebuilding it per test
- [x] `76a08a93f` **test** test(ic): reuse one instruction-memory-interface fixture instead of rebuilding it per test
- [x] `2e54cb3b6` **test** test(ic): make the 8-bit instruction decoder's one-hot check exhaustive
- [x] `e151cb229` **fix** fix(ic): stop re-deriving the execution datapath's Zero flag from Result bits
- [x] `8dcb8e570` **docs** docs(ic): remove stale comment claiming Push/Pop aren't wired yet
- [x] `bd69656d8` **test** test(ic): add a direct Reset test for the 8x8 RAM
- [x] `a413ee9b4` **test** test(ic): strengthen the 8x8 register file's test coverage
- [x] `d70d3f33a` **docs** docs(test): move the stack memory interface SV export test into its Level 6 section
- [x] `b9e859802` **test** test(ic): remove testRippleCarry, a 100% duplicate of testAdder4Bit
- [x] `654c8f9b5` **test** test(ic): fix mislabeled isolation/sequential-write rows in the 8x1 RAM test
- [x] `aad663633` **fix** fix(test): correct backwards segment-bit-position comment in BCD decoder test
- [x] `f31df503e` **test** test(sv): add missing SystemVerilog export coverage for level2_decoder_5to32
- [x] `45013fc09` **docs** docs(ic): document the Enable input in decoder/mux generator docstrings
- [x] `5649c10c4` **fix** fix(test): remove testCpuFullAdder duplicate of testFullAdder
- [x] `b35368f76` **fix** fix(element): stop custom appearance paths from silently failing or spamming errors
- [x] `3f47450b5` **fix** fix(audio,dolphin): apply the same custom-file path-resolution fix to AudioBox and BeWavedDolphin
- [x] `16c70f6cd` **refactor** refactor(io): dedup FileUtils::copyPandaDeps into Serialization::copyPandaFile
- [x] `871958f9e` **fix** fix(ic): make embedded-IC blob rename its own undo step, not an UpdateCommand side effect
- [x] `098131e34` **fix** fix(ic): reject colliding blob renames and track pasted blobs in undo
- [x] `36dc3b4ec` **fix** fix(scene): make property-shortcut commands fully and atomically undoable
- [x] `6a2344379` **fix** fix(scene): make MorphCommand's undo() symmetric with redo() for dropped connections
- [x] `2ace285a5` **fix** fix(ic): close two IC-loading safety gaps found in a simulation-engine audit
- [x] `527bed2c9` **fix** fix(clock): resync instead of bursting after a large wall-clock gap
- [x] `5db406cb2` **test** test(clock): make TestClocksAdvanced tests exercise the behavior they claim
- [x] `700a41cc5` **fix** fix(mcp): close a path-traversal write and a leaked IC in ICHandler
- [x] `753391ffb` **fix** fix(dolphin): stop dangling scene references and unbounded export
- [x] `c86015e7d` **fix** fix(export): bound Scene's grid loop and CircuitExporter's image buffer
- [x] `4eb683b3d` **fix** fix(export): share CircuitExporter's bounded renderer with MCP export_image
- [x] `6645b816d` **fix** fix(workspace): stop WorkSpace::save() from corrupting state on failure
- [x] `bd45496a8` **fix** fix(update): validate GitHub API URLs before download/open
- [x] `c3f85f72b` **fix** fix(exercise): clamp negative minCount from step JSON
- [x] `bd6279ae6` **fix** fix(clipboard): bound the clone-drag ghost image allocation
- [x] `703885117` **fix** fix(codegen): stop IC labels from injecting lines into generated code
- [x] `85aba6239` **fix** fix(element): bound the appearance pixmap caches via QPixmapCache
- [x] `4011e82b8` **fix** fix(dolphin): bound DolphinClipboard::paste()'s item count against the stream
- [x] `8697bcd0c` **fix** fix(dnd): bound the drag-and-drop payload's icFileName/blobName reads
- [x] `d5443c8c2` **fix** fix(io): bound the old-format trigger QKeySequence read
- [x] `7f5530282` **fix** fix(ic): bound IC dependency-chain recursion depth in two more places
- [x] `2f5e817f2` **fix** fix(element): reject non-finite/non-positive Buzzer frequency
- [x] `1a0b6504c` **fix** fix(mcp): bound stdin line buffering against a client that never sends '\n'
- [x] `3da52545b` **fix** fix(ui): stop routing keyboard shortcuts through translation
- [x] `bf603c52e` **fix** fix(led): stop translating binary-digit appearance state labels
- [x] `92d138212` **fix** fix(ui): stop embedding file-dialog glob patterns in translated strings
- [x] `d4640f456` **fix** fix(undo): restore built-in appearance on undo/redo/rollback snapshots
- [x] `c33f63f84` **fix** fix(render): call prepareGeometryChange() before swapping appearance pixmaps
- [x] `5f3cbbb94` **fix** fix(scene): don't shrink the scene rect at the instant of mouse-up
- [x] `6ee9d0011` **refactor** refactor(io): unify external-file-path handling behind SerializationPurpose
- [x] `5ecf1d880` **fix** fix(build): add missing SerializationContext.h includes
- [x] `744e9a713` **fix** fix(elements): rotate/flip a big IC/Mux/Demux/TruthTable about its true centre
- [x] `b609b2f4d` **refactor** refactor: dedup Exercise/Tour browser dialogs, engines, and test helpers
- [x] `c30b2746c` **refactor** refactor(ui): replace Exercises/Tours browser dialogs with dynamic menus
- [x] `524b9609a` **feat** feat(ui): auto-start the built-in UI tour on first launch
- [x] `68426abd2` **fix** fix(elements): keep name label anchored in place when an element rotates or flips
- [x] `ca7896672` **fix** fix(elements): reindex scene BSP tree on port-count and orientation changes
- [x] `6ad45ad96` **fix** fix(dolphin): close out remaining audit items in beWavedDolphin
- [x] `9ef59b168` **chore** chore(tests): remove superseded TestLengthDialog test
- [x] `9b15482bc` **fix** fix(build): register 47 test classes missing from CMake ALL_TESTS
- [x] `02330d093` **fix** fix(dolphin): allow closing the waveform window when askConnection is false
- [x] `cbaa96631` **fix** fix(ui): guard LabeledSlider paintEvent against a zero tick interval
- [x] `e210774a9` **docs** docs(core): fix stale guardedSlot comment describing removed deferred dispatch
- [x] `2a9503b90` **fix** fix(tests): sandbox QStandardPaths so tests stop leaking real AppData files
- [x] `0df901cb9` **test** test(dnd): drive ICDropZone/TrashButton drop handlers directly
- [x] `8cc71ddab` **test** test(workspace): make autosave tests trigger real circuit changes
- [x] `c26ce5036` **test** test(workspace): remove vacuous fileops tests, exercise real paths
- [x] `8ba46d4d7` **test** test(ui): strengthen vacuous assertions across several Unit test files
- [x] `e3c74ab05` **fix** fix(fuzz): restore fuzzer harness build after SerializationOptions change
- [x] `55d8f353f` **fix** fix(tests): stop invoking LabeledSlider::paintEvent via raw sendEvent
- [x] `488aac42c` **fix** fix(scene): stop redoing full-scene work on every structural edit
- [x] `72ae03a76` **fix** fix(mcp): clear selection before adding a new element via create_element
- [x] `df3165fc8` **feat** feat(ui): clarify the appearance-state selector for multi-state elements
- [x] `d62c7f1fb` **feat** feat(ui): group ElementEditor into labeled sections, fix Delay slider overlap
- [x] `b3465b983` **fix** fix(render): clear stale device-cache tile on appearance size change
- [x] `cd2d86d69` **fix** fix(ui): refresh appearance tile preview immediately after change
- [x] `554976200` **fix** fix(minimap): show the whole canvas instead of zooming to the item box
- [x] `be483515c` **fix** fix(minimap): fill the whole widget instead of letterboxing
- [x] `18dddd910` **fix** fix(ui): correct Buzzer tone scale octave jump (A7/B7 -> A6/B6)
- [x] `5ebe3808d` **fix** fix(ui): cycle elements in row-major order on Tab, guard against hang
- [x] `fb6d673fc` **fix** fix(ui): escape palette search query so metacharacters match literally
- [x] `094c877c3` **fix** fix(ui): move a removed IC file to the trash instead of hard-deleting
- [x] `ddcc89659` **fix** fix(scene): ignore key auto-repeat so triggers and pan fire once per hold
- [x] `149fa3ef9` **test** test: replace hollow QVERIFY(true) tests with real assertions
- [x] `e38a5de02` **ci** ci: run the gui-labeled test suite headless on Linux
- [x] `5dbc085f6` **feat** feat(ui): add a Zoom to Fit action
- [x] `c3dbbdfa1` **feat** feat(scene): nudge the selection with arrows; open dropped .panda files
- [x] `943753878` **feat** feat(ui): put undo/redo on the toolbar and add a status-bar indicator
- [x] `41c342460` **feat** feat(ui): generate the Shortcuts and Tips help from the live actions
- [x] `f899340e7` **content** content(exercises): use palette names instead of code identifiers
- [x] `49cb42aa2` **feat** feat(ui): show toolbar labels by default
- [x] `b3244c363` **feat** feat(palette): double-click a palette item to add it to the canvas
- [x] `5aeb8745f` **feat** feat(tabs): number unsaved "New Project" tabs so they stay distinct
- [x] `8636536de` **feat** feat(scene): explain why a rejected wire disappears
- [x] `4638957e6` **feat** feat(edit): add Duplicate (Ctrl+D)
- [x] `37d3333ba` **feat** feat(ui): show prettified titles in the Examples menu
- [x] `166289841` **feat** feat(scene): add Flip and Select All to the context menus
- [x] `5deaf4982` **feat** feat(view): raise the zoom-in cap from ~2x to ~4.8x
- [x] `266d82593` **fix** fix(ui): keep the IC preview popup on-screen near a corner
- [x] `77d1a8990` **feat** feat(ui): show the current file (and modified state) in the window title
- [x] `8a13b4415` **feat** feat(tabs): allow reordering and middle-click to close
- [x] `904b09fe0` **fix** fix(ic): don't claim "self-contained" when embedding stops early
- [x] `50fb4a251` **fix** fix(ic): warn on a name collision when adding a file-based IC
- [x] `44e4f730c` **feat** feat(tabs): mark autosave-recovered tabs with "(recovered)"
- [x] `bdf271db2` **refactor** refactor(ui): stop shipping placeholder strings to the translation catalog
- [x] `ea5e4437c` **feat** feat(view): accept Ctrl++ as well as Ctrl+= for zoom in
- [x] `3d7f4f363` **fix** fix(bwd): reconcile shortcuts that clashed with the main window
- [x] `1bdfb40a4` **feat** feat(ui): show a drop hint on the IC embed/extract zones
- [x] `14241ca32` **feat** feat(ic): offer to save instead of dead-ending on "save the project first"
- [x] `8fe6747de` **feat** feat(palette): describe each element in a tooltip
- [x] `f0b21a846` **fix** fix(arduino): explain pin overflow and pick the largest board on fallback
- [x] `b4a3fea24` **feat** feat(update): add a global "check for updates automatically" toggle
- [x] `ec6889659` **docs** docs(ux): track the remaining UX-audit tasks
- [x] `1e453cd5f` **feat** feat(element): inline double-click rename/edit for labelable elements (#7/#38)
- [x] `a624a4fed` **feat** feat(scene): add Align and Distribute tools for multi-selections (#16)
- [x] `d9c3a51fd` **feat** feat(ui): consolidate Exercises+Tours into a Learn menu; move Report Translation Error under Help (#8)
- [x] `d423beaa3` **feat** feat(a11y): add accessible names and whatsThis to canvas, palette, minimap, IC drop zones (#14)
- [x] `de7da70d4` **fix** fix(a11y): scale Exercise/Tour overlay text with the application font (#14)
- [x] `005265b62` **feat** feat(dolphin): add undo/redo for waveform cell edits (#19)
- [x] `ec2de617b` **docs** docs(ux): mark all 5 remaining heavy UX-audit features done
- [x] `e214ac590` **fix** fix(ci): correct translation-status PR comment and a precedence bug in translate.yml
- [x] `9b956a28f` **fix** fix(i18n): translate leftover InputSwitch/Led code identifiers in ExerciseTour catalogs
- [x] `5ea5f8a9f` **feat** feat(minimap): add drag-to-move and drag-to-resize interaction
- [x] `e62fb480e` **feat** feat(minimap): persist geometry, replace corner picker with free positioning
- [x] `ec40e6a3c` **test** test(minimap): cover resize/move interaction and geometry persistence
- [x] `d66ec5c81` **feat** feat(minimap): add visual affordances for the move/resize handles
- [x] `427eb12ab` **fix** fix(minimap): restore persisted geometry reliably at startup
- [x] `8d159c68e` **fix** fix(audio): mute AudioBox, not just Buzzer, when the simulation pauses
- [x] `31232eba2` **fix** fix(ic): retitle an open sub-circuit tab when its blob is renamed
- [x] `6fbfedce2` **fix** fix(ic): skip redundant sub-circuit reload on a cosmetic property edit
- [x] `269457bf9` **fix** fix(element): render remaining raster overlays as vectors (crisp at any zoom)
- [x] `c7c282cf6` **fix** fix(wiring): pad port bounding rect so its hover ring isn't clipped at high zoom
- [x] `7cebfce35` **fix** fix(tour): register waveform/element-editor demo circuits with the simulation
- [x] `d36376514` **fix** fix(tour): disconnect stale stepChanged handler before replaying a tour/exercise
- [x] `fead2b48f` **fix** fix(ui): stop forcing Open dialog into ./Examples on unsaved tabs
- [x] `d02d38db0` **perf** perf(ui): cache the per-element QSvgRenderer instead of rebuilding it per instance
- [x] `37a81f396` **perf** perf(ui): cache Scene::itemsBoundingRect() to fix interactive pan/zoom sluggishness
- [x] `96e89dd22` **perf** perf(wiring): skip Connection path rebuild when endpoints didn't move
- [x] `03b582aa7` **perf** perf(wiring): stop status-pen changes from forcing a BSP-tree re-index
- [x] `e0f3f6fc0` **perf** perf(ui): stop device-caching wires
- [x] `9c1286f63` **perf** perf(ui): adapt wire antialiasing to the measured paint budget
- [x] `12fc35cd9` **perf** perf(ui): cheapen hover hit-testing over dense wire bundles
- [x] `ea4eeb97e` **perf** perf(ui): quantize the scene rect to stop full BSP re-indexes
- [x] `985c9f7ae` **perf** perf(sim): make the deferred-commit tick path allocation-free
- [x] `91a6908cd` **perf** perf(sim): skip provably-idle simulation ticks
- [x] `262d6e2d5` **test** test: replace one-shot modal dismissers with a polling AutoDismisser
- [x] `f2e68190c` **ci** ci: extend the gui test suite beyond the Linux-only headless step
- [x] `29696ccf7` **build** build(win): define NOMINMAX project-wide
- [x] `cd20a3331` **feat** feat(io): compress .panda file payloads with zlib (Rev100)
- [x] `2daad6883` **chore** chore: migrate bundled .panda fixtures to the compressed format
- [x] `6d81fd95c` **refactor** refactor(element): name the serialized defaults that ctors set behind the header's back
- [x] `0196fcedb` **feat** feat(io): elide defaults and derive port serialIds in portable .panda payloads
- [x] `85144be37` **chore** chore: re-migrate bundled .panda fixtures to the slim Rev101 format
- [x] `008787496` **fix** fix(scripts): don't mistake a bare snapshot directory for completed pins

## Findings

Findings are appended here as review proceeds, each with an explicit
disposition: **fixed** / **not-a-bug-because** / **deferred-with-reason**.
None are dropped silently.

### Group: serialization format chain (Rev100/Rev101)

`cd20a3331`, `2daad6883`, `6d81fd95c`, `0196fcedb`, `85144be37`, `008787496`.

No findings — traced port-serialId derivation (`Port::globalIndex()` vs.
`makeSerialId(savedId, position)`), the decompression-bomb bound in
`readPayload()`, the appearance-slot empty-vs-nullopt `forReading()` edge
case, the `AudioOutputElement`/`Buzzer`/`Clock` default-constant refactor for
behavior-preservation, and the `ICRegistry` blob-splicing call sites that
needed extra handling once `readPreamble()` stopped leaving the device
positioned. All backed by real round-trip/legacy-load/bomb-guard tests, and
`v100.0`/`v5.1.0` backward-compat fixture pins keep both the fat-Rev100
explicit-serialId path and pre-compression path exercised going forward.

### Group: perf-tuning chain (11 commits)

`91a6908cd`, `985c9f7ae`, `ea4eeb97e`, `12fc35cd9`, `9c1286f63`, `e0f3f6fc0`,
`03b582aa7`, `96e89dd22`, `37a81f396`, `d02d38db0`, `abae69006`.

No findings — traced the idle-tick-skip invariant (confirmed Clock/
GraphicElementInput don't override `updateLogic()`, so the early
`clearOutputChanged()` on clocks/inputs can't hide a real change from the
main sweep), the deferred-commit CoW-detach fix, the scene-rect quantization
(monotonic non-shrinking during drag preserved), the adaptive wire-AA control
loop (gates on measured time not a count proxy, matching prior user
guidance; restore path never keys on the degraded renderer's own "fast
enough" reading), the `Connection`/`Port` status-pen BSP-reindex bypass
(single `setPen()` call site correctly still guarded on width change), and
the shared `QSvgRenderer` cache (only `render()`/`isValid()` called on the
shared instance — no mutation, safe to share on the single GUI thread).

### FINDING: comparator ComparatorBuilder's EqualIn cascade input never defaults high, contradicting its own doc comment

**File:** `Tests/Integration/IC/Generators/ic_builder_helpers.py:1151` (`ComparatorBuilder.create()`,
introduced in `83e450303`, still present at HEAD — affects `level3_comparator_4bit`,
`level3_comparator_4bit_equality` is a separate, untouched builder and not affected).

**Root cause:** `eq_in = await self.create_element("InputSwitch", ..., "EqualIn")` creates a
plain switch with no follow-up `set_input_value` call. `InputSwitch`'s constructor defaults
to OFF (`GraphicElementInput::setOn(false)`, confirmed while reviewing the Rev101 elision
commit), and that saved-off state is exactly what an unconnected IC boundary port resolves
to for an embedder that never wires it (the same mechanism the sibling `feat(level6): 8-bit
ALU` commit relies on, and correctly forces high via `set_input_value` for `SubCarryIn` —
`GreaterIn`/`LessIn` here are fine since they're *meant* to default low). The comment two
lines above the code (repeated verbatim in the commit message) says "tie EqualIn high ...
for standalone use," but nothing in the generator does that.

**Failure scenario:** any future circuit that embeds `level3_comparator_4bit` (or
`level4_comparator_4bit`, which shares the same `ComparatorBuilder`) without explicitly
wiring `GreaterIn`/`EqualIn`/`LessIn` — i.e. every standalone, non-cascaded use, exactly the
case the comment claims is handled — gets `equal_final = and_equal AND EqualIn = and_equal
AND 0`, permanently `0`. The `Equal` output stays low even when `A == B`. `Greater`/`Less`
are unaffected (their cascade inputs correctly default low, matching intent).

**Why it hasn't surfaced:** both `TestLevel3Comparator4bit.cpp` and
`TestLevel4Comparator4bit.cpp` explicitly wire a switch to `EqualIn` and call
`tieStandalone()` (`swEqIn->setOn(true)`) before every case, so neither test suite ever
exercises the true unconnected-default path. No generator currently embeds
`level3_comparator_4bit`/`level4_comparator_4bit` as a sub-component (grepped
`Tests/Integration/IC/Generators/*.py`) — `level5_modulo_counter_4bit` uses the separate,
unaffected `level3_comparator_4bit_equality` — and neither IC ships in `App/Resources` or
`Examples/`, so no current end-user-facing circuit is affected. It's a latent defect against
the component's own documented contract, not a currently-triggered regression.

**Proposed fix:** mirror the `SubCarryIn` pattern from the ALU commit:
```python
set_eq = await self.mcp.send_command("set_input_value", {"element_id": eq_in, "value": True})
if not set_eq.success:
    self.log_error("Failed to default EqualIn high")
    return False
```

**Disposition:** confirmed bug, not yet fixed (deferred to the fix-landing pass).

### Group: IC generator "deep review" series (18 commits)

`88889e13d`, `2b1c224cd`, `d858659ac`, `23fe5aa33`, `518d91517`, `d046ce6e9`,
`22ae5a649`, `577dd059a`, `592ac41fb`, `83e450303`, `deb54da3a`, `6fbe6dfeb`,
`1ba5cf713`, `40ea40e73`, `48425b602`, `55474e0d4`, plus the earlier
`52d6d3b43` (non-blocking sequential semantics, reviewed with the fix/feat
batch above).

One finding (above: `ComparatorBuilder`'s `EqualIn` default). Otherwise
clean across a large, high-risk cluster (flip-flop gate-level rebuilds, ALU
carry/shift-fill byte-boundary chaining, RAM/counter/PC async-reset
polarity, the multi-cycle CPU's gated-clock→clock-enable redesign, one-hot
instruction decode). Verified by hand: the Johnson/ring counter seed and
feedback-tap polarity (active-low Preset/Clear, straight vs. inverted
feedback), the 8-bit ALU's SHL/SHR mux-select polarity end-to-end (the
comment text describing it was briefly inconsistent across commits, but the
actual wiring was correct throughout), the 16-bit ALU's carry and shift-fill
chaining across the byte boundary bit-by-bit, the 74LS85 comparator cascade
equations, the multi-cycle CPU's phase-gated write-enable OR-of-normal/
programming paths, and the register-file/PC/RAM active-high-Reset→
active-low-~Clear inversion pattern (used consistently ~8 times across this
series, always correctly). Cross-checked several against pre-existing
project memory from the original deep-review-v3 effort (F52-F54, F61, F96,
F98, F101-F102 all match); no discrepancies found between that record and
the code actually shipped in this range.

### Group: codegen fixes (4 commits)

`571bbe3e7`, `eaa872724`, `78dc3302a`, `a2eb28e8a`.

No findings. `571bbe3e7`'s TruthTable multi-output bit-index formula (`256*out +
row`) matches the C++ engine's own indexing verified earlier (`d92020354`),
consistently across the engine, Arduino, and SystemVerilog backends.
`eaa872724`'s replacement of the port-signature behavioral shortcut with a real
reachability-based (`findFeedbackElements`) structural translator was traced by
hand — the DFS correctly restricts traversal to combinational gate types (never
crosses a real flip-flop/IC boundary) and correctly detects only genuine
self-reachable cycles, not merely "reaches some cycle." `78dc3302a`/`a2eb28e8a`
are test-only; the seeded-random differential stimulus that subsumes the
hand-written edge-polarity vectors uses a fixed `constexpr` seed
(`0x5EEDC0DEU`), so it's fully reproducible — no CI flakiness risk from the
switch to randomized stimulus.

### FINDING: MCP set_element_properties rejects legitimate negative Clock delay

**File:** `MCP/Server/Handlers/ElementHandler.cpp` (`handleSetElementProperties`, added in
`b861224d5`, still present at HEAD).

**Root cause:** the new validation added `if (newDelay < 0) { return createErrorResponse(
"Parameter 'delay' must be non-negative", ...); }`. But `hasDelay` is `true` only for
`Clock` (checked — no other element sets it), and `Clock::setDelay()`
(`App/Element/GraphicElements/Clock.cpp:209-217`) explicitly documents and handles the
full `[-1, 1]` range via `std::clamp(delay, -1.0, 1.0)`, with the very next function
(`resetClock()`) commenting "Negative delays advance the clock (trigger earlier), positive
delays delay it." Negative delay is an intentional, documented feature, not an invalid
input. (Note: `Clock.h`'s own doc comment on the getter says the range is "[0, 1)" —
that header comment is itself stale/wrong relative to the .cpp implementation and is
likely what misled this validation; not a new defect from this commit, but worth fixing
alongside it.)

**Failure scenario:** an MCP client (e.g. an AI agent building a circuit) calling
`set_element_properties` on a `Clock` with a negative `delay` to phase-advance it now gets
a `ValidationError` instead of the value being applied — a capability that works fine via
direct API use (`Clock::setDelay(-0.3)`) or a hand-crafted `.panda` file is unreachable
through the MCP surface. `frequency > 0` and `volume ∈ [0,1]` in the same commit are
correctly scoped (verified against `Clock::setFrequency`'s `qFuzzyIsNull` guard and
`AudioOutputElement`'s volume clamp); only the `delay` bound is wrong.

**Proposed fix:** change the bound to `if (newDelay < -1.0 || newDelay > 1.0)` (matching
`Clock::setDelay`'s own clamp range), and fix `Clock.h`'s getter doc comment from
"[0, 1)" to "[-1, 1]" to match the implementation.

**Disposition:** confirmed bug, not yet fixed (deferred to the fix-landing pass).

### FINDING: priority_encoder_8to3's EI cascade input never defaults high, same bug class as the comparator's EqualIn

**File:** `Tests/Integration/IC/Generators/create_level2_priority_encoder_8to3.py:88`
(introduced in `f540ffbf0`, still present at HEAD).

**Root cause:** identical to the comparator `EqualIn` finding above. `ei_input = await
self.create_element("InputSwitch", ..., "EI")` with no follow-up `set_input_value` call.
The doc comment (repeated in the commit message) says "Tie high for standalone use," but
`InputSwitch` defaults OFF, and every output is gated through `AND(_, ei_input)` or derived
from it (`addr_gated[i]`, `valid_gated`, `eo_gate`).

**Failure scenario:** any future embedder of `level2_priority_encoder_8to3` that doesn't
explicitly wire `EI` — i.e. every standalone use, exactly the case the comment claims is
handled — gets `addr[0..2]=000`, `valid=0`, `EO=0` permanently, regardless of the data
inputs. The encoder is silently disabled by default instead of behaving as a plain 8-to-3
priority encoder.

**Why it hasn't surfaced:** `TestLevel2PriorityEncoder8to3.cpp` explicitly wires a switch to
`EI` and calls `f.ei->setOn(true)` before every case (same pattern as the comparator tests),
so the true unconnected-default path is never exercised. No generator currently embeds
`level2_priority_encoder_8to3` as a sub-component (grepped
`Tests/Integration/IC/Generators/*.py`). Latent, not a currently-triggered regression.

**Proposed fix:** same pattern as the comparator fix:
```python
set_ei = await self.mcp.send_command("set_input_value", {"element_id": ei_input, "value": True})
if not set_ei.success:
    self.log_error("Failed to default EI high")
    return False
```

**Disposition:** confirmed bug, not yet fixed (deferred to the fix-landing pass).

### FINDING: MinimapWidget::applyResize() doubles the size delta and drifts the anchor edge on top/left drags

**File:** `App/UI/MinimapWidget.cpp:365-370` (introduced in `5ea5f8a9f`, still present at HEAD;
touched only cosmetically by `37a81f396`'s cache-source swap, bug untouched).

**Root cause:** `geom.setSize(QSize(newWidth, newHeight))` keeps the rect's *current* top-left
fixed and recomputes its bottom-right from it. The following `geom.setTop(...)` (and
`geom.setLeft(...)`) then anchors on THAT already-shifted bottom/right, not the original
`oldGeom` bottom/right — so the two edge-setters compound instead of independently placing
one edge while preserving the other.

**Failure scenario:** drag the widget's *top* (or *left*, or a corner touching either) resize
handle. Verified with a standalone `QRect` repro (`oldGeom = QRect(100,100,220,160)`, drag
top edge up by 20px → `newHeight=180`): the resulting rect is `top=80, height=200`, i.e. the
bottom edge drifts from 259 to 279 instead of staying fixed at 259, and per-mouse-move the
error compounds every frame (each 1px drag step grows the widget by 2px and shifts the far
edge by 1px in the same direction) — the minimap visibly grows ~2x faster than the cursor
and slides away from its anchored edge instead of resizing in place. Same issue for `left`.
`top`+`right`/`bottom`+`left` diagonal handles inherit whichever half of this is active.

**Proposed fix:** build the new rect from the correct anchor directly instead of mutating
edges sequentially:
```cpp
const int newX = left ? oldGeom.right() - newWidth + 1 : oldGeom.left();
const int newY = top  ? oldGeom.bottom() - newHeight + 1 : oldGeom.top();
geom = QRect(newX, newY, newWidth, newHeight);
```
This replaces the `geom.setSize(...)` + conditional `setTop()`/`setLeft()` block entirely.

**Disposition:** confirmed bug, not yet fixed (deferred to the fix-landing pass).

### Group: minimap widget feature (9 commits)

`a18475826`, `554976200`, `be483515c`, `d423beaa3`, `5ea5f8a9f`, `e62fb480e`,
`ec40e6a3c`, `d66ec5c81`, `427eb12ab`.

One finding (above: `applyResize()` top/left anchor bug). Otherwise clean —
traced the render-cache invalidation (`m_cacheSrc` correctly added once the
source rect started tracking sceneRect/viewport instead of a static item
box), the fill-not-letterbox aspect-expansion math (verified symmetric grow
formula and post-expansion scale equality), the Settings corner-picker →
free-geometry migration (no dangling `MinimapCorner` references left
anywhere), and the startup restore race fix (`isVisible()` guard +
`showEvent()` backstop timer). Confirmed the test suite exercises
`ResizeMode::Right` only for `applyResize()`'s aspect-preservation test,
never `Top`/`Left` — consistent with the anchor bug going unnoticed.

### Group: Exercise/Tour system (`c481e8b0b`, `a859d7911` + follow-ons)

`c481e8b0b` (ExerciseEngine/Overlay/BrowserDialog, `ExerciseTourResources`
discovery/translation), `a859d7911` (TourEngine/Overlay/BrowserDialog —
structurally near-identical to Exercise, later deduped by `b609b2f4d`).

Read both engines, both overlays (incl. `TourOverlay::repositionCallout()`'s
spotlight-vs-callout placement math — uses side-effect-free `QRect::adjusted()`
throughout, not the `setSize()+setTop()` pattern behind the Minimap bug above,
and the below/above/centered fallback chain is internally consistent), both
browser dialogs, and the `ExerciseTourResources` discover/scan/translate
implementation against its own CLAUDE.md-documented contract (built-in →
AppData → install-relative → Documents-fallback merge order, `mergeUnique()`
collision policy, `preferredContentDir()` never touching `managedContentDir()`)
— matches exactly.

Three real bugs were found by tracing `startExercise()`/`startTour()` and the
demo-circuit `clickTarget()` handlers, and all three turned out to be
independently caught and fixed later in-range by the same author — verified
by reading each fix commit's diff, not just trusting the subject line:

- `startExercise()`/`startTour()` connect a new `this`-scoped `stepChanged`
  lambda (driving `clickTarget()`) on every call without ever disconnecting
  the previous one, since `m_exerciseEngine`/`m_tourEngine` are long-lived
  MainWindow members created once in the ctor. Replaying/switching
  exercises or tours in one session accumulates duplicate handlers, so
  toggle-style `clickTarget()` actions (e.g. `actionPlay->trigger()`) fire
  N times per step. **Fixed by `d36376514`** (adds
  `disconnect(m_exerciseEngine, &ExerciseEngine::stepChanged, this, nullptr)`
  / same for `m_tourEngine`, immediately before reconnecting).
- The Tour's `setupElementEditorDemo`/`setupWaveformDemo` demo circuits were
  added via raw `scene->addItem()`, bypassing
  `Scene::setCircuitUpdateRequired()` — the demo Clocks never entered the
  Simulation's cached topology and so never ticked. **Fixed by `7cebfce35`**
  (routed through `AddItemsCommand`, matching every other add-element call
  site).
- `ExerciseEngine::loadFromResource()` read `minCount` straight from
  end-user-writable step JSON with no lower bound; a negative value made
  `count < minCount` vacuously true, permanently satisfying (skipping) that
  requirement. **Fixed by `c3f85f72b`** (`qMax(0, ...)` clamp + regression
  test).

One finding remains open (not caught by a later commit — see below).

### FINDING: unknown element/connection type name in Exercise/Tour step JSON silently makes the step permanently unsolvable, with no diagnostic

**File:** `App/Exercise/ExerciseEngine.cpp`, `validateElements()` (~line 169)
and `validateConnections()` (~line 196-197).

**What's wrong:** `ElementFactory::textToType(req.typeName)` (and
`fromTypeName`/`toTypeName`) returning `ElementType::Unknown` — e.g. from a
typo'd `"type"` field in a custom exercise's JSON, since Exercise/Tour
content is explicitly end-user-writable per `ExerciseTourResources`' own
documented architecture — makes `validateElements()`/`validateConnections()`
return `false` unconditionally. Since `onCircuitChanged()` only ever calls
`advanceStep()` when `validateCurrentStep()` returns `true`, the step can
never be satisfied by any circuit the user builds: the exercise silently
gets permanently stuck on that step, with no error surfaced anywhere. This
is the same "unvalidated content field" class as the `minCount` bug fixed by
`c3f85f72b`, but for the `type`/`fromType`/`toType` fields, and it was never
caught. Every *other* malformed-content path in this feature
(`ExerciseTourResources::scan()`'s missing id/title, malformed JSON, missing
file) logs a `qWarning()`; this one doesn't, so a content author gets no
hint about why their exercise won't advance.

**Failure scenario:** A custom exercise JSON (built-in or end-user-authored
via the documented `Exercises` folder) has `{"type": "AndGate"}` instead of
`{"type": "And"}` in `requiredElements`. The user builds a perfectly correct
circuit; the step never advances; nothing in the UI or logs indicates why.

**Proposed fix:** Add a `qWarning()` when `textToType()` returns `Unknown`
in both `validateElements()` and `validateConnections()`, mirroring
`ExerciseTourResources::scan()`'s existing warning convention — cheap,
consistent with the codebase's established pattern for this content-authoring
risk class, and gives a content author (including the project's own
contributors authoring built-in exercises) something to grep the log for.

**Disposition:** deferred — low severity (UX/content-authoring gap, not a
crash or data-safety issue; every bundled exercise's `type` fields were
verified against `ElementFactory::textToType()`'s known names, so no
shipped content is currently affected). Will land alongside the other
findings' fixes in Step 4.

### Group: IC fixture layout — overlap/label-collision series (10 commits)

`4830e2a3a`, `43541ef92`, `8866c8df9`, `1ad22c0a5`, `fc70cf321`,
`686938f0b`, `8a550aa19`, `5ca1dd46b`, `a9c50d1f1`, `731dfc693`.

No findings. `4830e2a3a` fixes `instantiate_ic`'s absolute-path rejection
that silently broke every Python generator embedding a sub-IC (verified the
new `.contains("..")` guard is still traversal-safe for the real absolute
paths the generators construct via `IC_COMPONENTS_DIR / component`, no
extension double-suffixing). `43541ef92` adds `TestICFixtureLayout`, a
pairwise `sceneBoundingRect()`-intersection oracle over every bundled
`.panda` fixture (2px tolerance; read its full implementation, including
the later label-vs-element/label-vs-label extension from `fc70cf321` folded
into the same file) — its effectiveness is independently corroborated by
every subsequent commit in this cluster citing a specific before/after
overlap count that the oracle produced. `8866c8df9` exposes real rendered
element/IC geometry from `create_element`/`instantiate_ic` so generators can
place neighbors from true footprint instead of guessed spacing constants.
The remaining six are position-only generator fixes; spot-verified
`1ad22c0a5` (the two "genuine structural bug" cases, not just spacing:
`level6_alu_8bit`'s aliased column-index formulas and
`level6_stack_memory_interface`'s RAM-inside-mux-row placement — confirmed
no `connect()` calls changed, only `create_element` coordinates) and traced
one regenerated SV golden (`level4_ripple_adder_4bit.sv`) end-to-end to
confirm the diff is pure named-port-binding reordering from the new
Y-sorted port order, not a semantic change — consistent with the
`eaa872724` structural-translator invariant already verified earlier.
`731dfc693` is itself evidence the oracle works as intended: it fixes 13
fixtures that only failed on Windows CI's different default font metrics,
caught by the same tolerance-based check.

### Group: exercise content, IO hardening, CI security, install/packaging (18 commits)

`4b0a22af5`, `d432be66e`, `4f2bd5a9e`, `0b1fd3e18`, `0aacb3adb`, `7cb03d3fa`,
`52500b40d`, `ea6ada857`, `fee8b54d8`, `8cbee0575`, `ac62d81e0`, `bdb49a3ca`,
`172e136d8`, `e94db4971`, `e511ab95c`, `a80a4245c`, `49ee38ad1`, `859ee7937`,
`a54547edf`, `bfa55721c`, `f421aff5a`.

No findings. Highlights actually verified, not just read:

- `4b0a22af5`/`d432be66e` (new bundled exercises): cross-checked every
  `type`/`fromType`/`toType` string in every `App/Resources/Exercises/*.json`
  and `Tours/*.json` file against the real `Enums::ElementType` Q_ENUM
  member names (`App/Core/Enums.h`) — none are affected by the open
  unknown-type finding above.
- `4f2bd5a9e` (cast-align fixes): verified all three replacements
  (`Display7::convertColor` → `QImage::pixel()/setPixel()`,
  `ToneGenerator::readData` → per-sample `memcpy`, `Serialization::
  readDolphinHeader` → `QVarLengthArray` scratch buffer) preserve exact
  behavior; the scratch-buffer size can't overflow because `magicHeader %
  2 != 0` is already rejected earlier in the same function.
- `ea6ada857` (IO bound fix): confirmed `readBoundedBlobMap()` — the same
  helper `ClipboardManager::paste()` already used correctly — bounds the
  entry count against `bytesAvailable()/8` before looping, closing the one
  remaining raw `QDataStream >> QMap<QString,QByteArray>` call site.
- `f421aff5a` (CI script-injection fix): confirmed no other unguarded
  `${{ github.head_ref }}`/PR-title/branch-name splice into a `run:` block
  exists anywhere under `.github/workflows/`; the one remaining
  `${{ github.head_ref }}` use is an `actions/checkout` `with: ref:` input
  (action-input context, not shell-text splicing, so out of scope by the
  same reasoning the commit itself applies to the repo-name field).
- `172e136d8` (AppImage via cmake --install): traced `X-AppImage-Version`'s
  new `@PROJECT_VERSION@` source back through `configure_file`'s ties to
  the version-bump-before-tag convention (10+ prior releases in git log all
  bump `project(wiredpanda VERSION ...)` immediately before tagging) —
  consistent with `61c046571`'s established Windows-metadata precedent, not
  a divergence risk.
- `49ee38ad1` (AUR publish workflow): the `secrets.*`-inside-`if:` mistake
  the commit message warns about is already avoided in the committed file
  via the `env: HAS_AUR_KEY` indirection — verified no `if:` in the file
  references `secrets.` directly.
- `e511ab95c` (macOS bundle): cross-checked the new
  `Contents/Resources/Examples` install destination against
  `InstallRelativePaths::candidates()`'s existing `Q_OS_MACOS` runtime
  search path — they agree. Author explicitly flags this path as unverified
  without real macOS CI; noted, not independently re-verifiable here.

### Group: 16-bit RISC CPU register file, decode stage, data memory (7 commits)

`f462c0a84`, `f871366f9`, `4fbfb3532`, `b045210b3`, `1cf717ef1`,
`9e58fcf5f`, `b7a89ceb3`.

No findings. `f462c0a84` replaces a fake register file (DestReg was a raw
ALU operand, not an address — the CPU never actually had the 32-register
file its own docstring claimed) with a real one; the interesting new code
is `RegisterFileBuilder._build_read_mux_tree()`, a binary mux-tree builder
needed because a flat 32-input read mux exceeds the engine's 11-port Mux
cap. Hand-traced its address decomposition for the 32-register/5-bit case:
level 0 groups `sources` into four 8-entry blocks selected by address bits
0-2, level 1 selects among the four level-0 results using bits 3-4 — a
standard, correct hierarchical select decomposition (group index = addr >>
3, exactly the bits left after removing the 3 LSBs consumed within-group).
`f871366f9`'s `TestLevel9RegisterFile32X16::testMultiAddressStorage()`
independently confirms this at the simulation level: writes distinct words
to one register in each of the four mux-tree groups (0, 8, 16, 24 and more)
and reads each back, which is exactly the test that would catch a
cross-group aliasing bug in the tree if the hand-trace above had missed
one — it doesn't. `4fbfb3532`/`b045210b3` are pure dev-tooling fixes
(making an f-string-computed dependency name visible to
`run_all_generators.py`'s literal-only dependency scanner) with a
self-verifying `ValueError` guard on mismatch and an explicit
"byte-identical output" claim, confirmed by the diff containing no `.sv`
golden changes. `1cf717ef1` (tie off unused ALU bits) is a
defense-in-depth fix for behavior that was already correct by default-port
resolution; also confirmed by the absence of any `.sv` golden diff in that
commit. `9e58fcf5f`/`b7a89ceb3` are test-only; the latter is a genuinely
good catch — the old multi-cycle-CPU chained-instruction test's final-only
assertion (`AND` with `0x20`) could pass even with a miscomputed
intermediate `ADD` result, which the new edge-by-edge assertions close.

### Group: MCP security hardening + undo/redo module audit (6 commits)

`700a41cc5`, `2ace285a5`, `871958f9e`, `098131e34`, `36dc3b4ec`,
`6a2344379`.

No findings — this cluster is itself a self-contained "Nth pass of a
deep-review audit" series (the commit messages number their own passes:
sixth/seventh/eighth), and every fix in it is backed by a real regression
test, several of which are notable for rewriting a PRE-EXISTING test that
had been asserting the buggy behavior as correct
(`testRenameBlobCollisionOverwrites` → `testRenameBlobCollisionRejected`,
`testBlobNameCollisionDuringRename`). Verified in detail, not just read:

- `700a41cc5` (MCP path traversal + IC leak): hand-traced
  `handleExtractIC`'s confinement check
  (`cleanFileName != cleanContextDir && !cleanFileName.startsWith(cleanContextDir + '/')`)
  against the classic prefix-without-separator bypass
  (`/project` vs `/project-evil`) — correctly guarded by including the
  trailing `/` in the `startsWith` check. Confirmed the `unique_ptr<IC>`
  leak fix (`loadFile()` now called before `release()`) is the correct
  RAII ordering.
- `2ace285a5` (IC-reload UAF + unbounded recursion): confirmed
  `reloadTargetsAtomically()`'s single `SimulationBlocker` spans the
  *entire* target loop in all three callers (`onFileChanged`,
  `embedICsByFile`, `extractToFile`), and hand-verified each caller's
  `setCircuitUpdateRequired()`/rollback call count is unchanged from the
  pre-refactor code on both the success and exception paths (exactly one
  call either way, never both).
- `871958f9e`/`098131e34` (blob rename/paste undo gaps): confirmed
  `UpdateBlobCommand::undo()`'s `m_oldBlob.isEmpty()` branch correctly maps
  to `removeBlob()` (not `setBlob()` with empty bytes), matching
  `embedICsByFile()`'s new explicit `cmd->setOldBlob(QByteArray())` for a
  freshly-registered (not replaced) blob.
- `36dc3b4ec` (property-shortcut macro grouping): consistent
  `needsMacro = selected.size() > 1` gating, matching the pattern
  `098131e34`'s `ClipboardManager::paste()` fix already established.
- `6a2344379` (MorphCommand undo/redo symmetry): the author explicitly
  flags this gap as unreachable via the current GUI/MCP (QUndoStack's
  contiguous-frontier invariant prevents the triggering sequence) but
  fixes it anyway as a structural-symmetry issue, with a real test that
  bypasses the undo system directly (mirroring "a separate user action
  would") to reach it.

### Group: clock resync, export DoS hardening, WorkSpace/Dolphin save safety (7 commits)

`527bed2c9`, `5db406cb2`, `bd45496a8`, `c86015e7d`, `4eb683b3d`,
`6645b816d`, `753391ffb`.

No findings. `527bed2c9` reuses the *already-accepted* `resetClock()`
force-HIGH resync (previously only reachable via explicit
`Simulation::start()`) for a new triggering condition (a >5s wall-clock
gap, e.g. OS suspend) — not a new state-forcing behavior, just extending
an existing one; the fixed 5s threshold (not scaled to clock frequency) is
deliberately chosen so ordinary jitter on high-frequency clocks can't
misfire it. `bd45496a8`'s `isSafeGitHubUrl()` does an exact host-equality
check (`== "github.com"`), correctly avoiding the lookalike-domain bypass
class (`github.com.evil.com`) its own test suite exercises.
`c86015e7d`/`4eb683b3d` are the same DoS-hardening effort in two commits —
the first fixes `CircuitExporter::renderToImage`'s proportional pixel-buffer
allocation and `Scene::drawBackground`'s unbounded grid-point loop (the
grid fix is scene-level, so it also transparently protects every other
`scene->render()` caller, PDF included); the second closes the sibling gap
where MCP's `export_image` PNG branch had re-implemented — and never
inherited — the same size cap, by extracting `renderScaledImage()` to a
single shared implementation. Checked whether `export_image`'s SVG/PDF
branches (still calling `scene->render()` directly, not through the shared
helper) have a parallel gap: they don't — neither format allocates a
raster buffer proportional to canvas size, and the grid-point hang is
already closed at the `Scene::drawBackground()` level regardless of which
export path calls `render()`. `6645b816d` (`WorkSpace::save()`) and
`753391ffb` (`BewavedDolphin`) both fix a dangling-reference-after-mutation
class of bug reachable specifically via MCP clients bypassing the GUI's
usual interaction ordering — a recurring theme across this whole session's
MCP-adjacent findings.

### Group: systematic unbounded-allocation/recursion hardening pass (11 commits)

`bd6279ae6`, `703885117`, `85aba6239`, `4011e82b8`, `8697bcd0c`,
`d5443c8c2`, `7f5530282`, `2f5e817f2`, `1a0b6504c`, plus `b35368f76` and
`3f47450b5` (external-file path resolution, reviewed just above).

No findings — a self-contained, systematic audit sweep closing the same two
bug classes (unbounded allocation sized from untrusted input; unbounded
recursion with only cycle detection, no depth cap) everywhere they'd been
found once already this session, each with real regression tests. Verified
beyond a single read where the diff looked genuinely novel rather than a
mechanical reuse of an already-verified pattern:

- `703885117` (label→comment injection into generated Arduino/SV):
  grepped every remaining raw `ic->label()`/`elm->label()` use in both
  codegen files after the fix — all route through
  `removeForbiddenChars()` (identifier sanitization, strips non-alnum
  including newlines) before reaching `m_stream`, confirming the 6 sites
  wrapped in the new `sanitizeComment()` were genuinely the only unsafe
  ones, not a partial fix.
- `1a0b6504c` (stdin line-buffering bound): hand-traced
  `StdinReader::run()`'s new byte-at-a-time state machine (the one
  non-mechanical rewrite in this batch, replacing `std::getline`) through
  four cases — normal newline-terminated line, immediate EOF, a final
  unterminated line, and an oversized line hitting the cap followed by
  either EOF or a subsequent normal line — all resolve correctly, matching
  `std::getline`'s original semantics for the non-overflow cases.
- `8697bcd0c` (drag-and-drop payload bound): confirmed all 4 raw-read call
  sites (`SceneDropHandler` ×2, `ICDropZone`, `TrashButton`) were migrated
  to the new shared `readDragDropPayload()`, none left on the old
  unbounded path.
- `7f5530282` (IC dependency-chain depth cap): confirmed both recursive
  call sites in `ICRegistry::makeBlobSelfContained()` correctly thread
  `depth + 1`.

### Group: translation-safety, undo/geometry/BSP crash fixes, Exercise/Tour dedup, UX-audit fix batch (34 commits)

`3da52545b`, `bf603c52e`, `92d138212`, `d4640f456`, `c33f63f84`,
`5f3cbbb94`, `6ee9d0011`, `5ecf1d880`, `744e9a713`, `b609b2f4d`,
`c30b2746c`, `524b9609a`, `68426abd2`, `ca7896672`, `6ad45ad96`,
`9ef59b168`, `9b15482bc`, `02330d093`, `cbaa96631`, `e210774a9`,
`2a9503b90`, `0df901cb9`, `8cc71ddab`, `c26ce5036`, `8ba46d4d7`,
`e3c74ab05`, `55d8f353f`, `488aac42c`, `72ae03a76`, `b3465b983`,
`cd2d86d69`, `18dddd910`, `5ebe3808d`, `fb6d673fc`, `094c877c3`,
`904b09fe0`, `50fb4a251`, `266d82593`, `3d7f4f363`, `f0b21a846`,
`bdf271db2`, `ddcc89659`.

No findings across this whole stretch. Highlights actually independently
verified (not just read):

- `3da52545b`/`bf603c52e`/`92d138212`: three real, user-visible
  translation-corrupts-machine-token bugs (Spanish "Del" breaking
  QKeySequence parsing, blank LED-appearance dropdown rows, translator
  edits able to corrupt file-dialog globs) — each fix is mechanical and
  matches its own bug report exactly.
- `d4640f456`/`c33f63f84`/`ca7896672`: closed the exact two remaining gaps
  flagged by pre-existing project knowledge of the `prepareGeometryChange()`
  invariant (`setPixmap()`'s failure/fallback branch, and
  `setPortSize()`/`applyOrientation()`) — `ca7896672` fixes a real
  Sentry-tracked crash (WIREDPANDA-KS).
- `6ee9d0011` (ExternalFilePath/SerializationPurpose unification): traced
  `forWriting()`'s unconditional bare-filename truncation for
  `PortableFile` against `WorkSpace::save()`'s now-unconditional copy loop
  — verified `FileUtils::copyToDir()`/`Serialization::copyPandaFile()`
  both no-op correctly on a same-directory re-save, so the truncation is
  never left dangling. `e3c74ab05` (11 days later) independently confirms
  the mandatory-parameter compile-time invariant this refactor introduced
  actually caught 7 stale fuzzer harnesses, each fixed with the
  purpose matching its real production analogue and re-verified against
  the full historical crash corpus.
- `b609b2f4d` (Exercise/Tour engine dedup into `StepEngineCore<T>`):
  cross-checked the new template against every previously-verified
  ExerciseEngine/TourEngine behavior from earlier in this review — the
  `c3f85f72b` `qMax(0, ...)` minCount clamp and the `d36376514`
  stale-connection-disconnect fix (untouched by this diff, confirmed by
  its absence from the changed hunks) both survive intact.
- `744e9a713`/`68426abd2`: the IC/Mux/Demux/TruthTable pivot-center fix and
  the label-anchor fix are both real, user-reported geometry bugs with
  end-to-end scene-position regression tests that specifically exercise
  combined rotation+flip — exactly the case a matrix-composition-order
  mistake would fail on, and don't.
- `488aac42c`/`72ae03a76`: perf fixes (measured 563s→32s and 381s→97s on
  large MCP-driven builds) that remove eager work in favor of an
  *already-existing*, already-asserted lazy-invalidation path
  (`Simulation::update()`'s `if (!m_initialized && !initialize())`,
  labeled with its own historical "Bug 5"/H2-cluster-fix provenance) —
  not a new invariant being introduced under time pressure.
- `2a9503b90`: confirms this repo's own test suite had been leaking real
  files into the reviewer's `~/.local/share` for a long time before this
  fix — worth knowing before drawing conclusions from any local `ctest`
  run's side effects, though this session's checkout already has the fix.

### Group: final UX-audit tail — inline rename, tabs/menus, MCP-adjacent fixes, CI/build (33 commits)

`e214ac590`, `9b956a28f`, `8d159c68e`, `31232eba2`, `6fbfedce2`,
`269457bf9`, `c7c282cf6`, `fead2b48f`, `1e453cd5f`, `a624a4fed`,
`d9c3a51fd`, `005265b62`, `ec2de617b`, `77d1a8990`, `8a13b4415`,
`44e4f730c`, `ea5e4437c`, `1bdfb40a4`, `14241ca32`, `8fe6747de`,
`b4a3fea24`, `ec6889659`, `262d6e2d5`, `f2e68190c`, `29696ccf7`.

No findings. Highlights actually verified in depth (not just read):

- `e214ac590` (CI translation-status bug): confirmed the real root cause —
  a completed Qt `.ts` `<translation>` tag has *no* `type` attribute at
  all (only `type="unfinished"` exists), so the old `type="finished"`
  match was structurally guaranteed to always return 0, for every
  language including English. The `&&`/`||` operator-precedence fix in
  the same commit is a textbook case (`A && B || C` parses as
  `(A && B) || C`), correctly parenthesized.
- `6fbfedce2` (skip redundant IC reload on cosmetic edit): the
  `blobUnchanged` skip-condition is a 3-way AND (name match, non-empty
  internal state, byte-exact content match) — traced why this can only
  ever cause a *missed* optimization, never a stale-state correctness
  bug, since any mismatch on any of the three conditions falls through
  to the always-safe reload path.
- `1e453cd5f` (inline label rename): hand-traced the
  `returnPressed`+`editingFinished` double-signal risk (Qt fires both on
  Enter) — `commit()`'s `m_target` guard makes a second call from either
  signal a no-op, since `close()` nulls `m_target` synchronously before
  either signal handler could re-enter.
- `29696ccf7` (Windows NOMINMAX): confirms `LabeledSlider.cpp`'s
  `std::numeric_limits<int>::min()` usage (already verified correct C++
  when `d62c7f1fb` was reviewed earlier) was blocked only by a
  `windows.h` macro collision on Sentry-enabled release builds, not by
  any logic defect — cross-confirms that earlier review.
- `a624a4fed` (Align/Distribute): algebraically re-derived
  `distributeHorizontally()`'s gap formula and confirmed the running
  `cursor` position for the last repositioned element always lands
  exactly on the (unrepositioned, anchored) last element's real edge —
  the distribution is mathematically self-consistent, not an
  accumulated-rounding coincidence.

## Step 3 — baseline build + test verification (done)

`cmake --preset debug` + `cmake --build --preset debug`: clean configure,
clean build (65/65 targets, zero warnings surfaced in the tail of the
build log). `ctest --preset debug`: **191/191 tests passed, 100%**, no
flakes (including `TestAudioBox`/`TestComponents`, the known
parallel-flake pair — see `project_flaky_parallel_tests` — which didn't
trip this run). Total test time 35.87s. Tree is healthy independent of
the historical line-by-line review above.

## Step 4 — fixes for confirmed findings (next)

Two findings need code changes; both were dispositioned as real, both
still open (not superseded by any later in-range commit):

1. `MinimapWidget::applyResize()` top/left anchor bug (doubles the resize
   delta, drifts the anchor edge) — see the FINDING entry above for the
   proposed fix.
2. `ExerciseEngine::validateElements()`/`validateConnections()` silently
   make a step permanently unsolvable on an unknown element/connection
   type name, with no diagnostic — proposed fix: add `qWarning()` on
   `ElementFactory::textToType()` returning `Unknown`, mirroring
   `ExerciseTourResources::scan()`'s existing convention.

The other two originally-found bugs (`ic_builder_helpers.py`'s
`ComparatorBuilder`/`priority_encoder_8to3` missing `set_input_value`
defaults, and the MCP `set_element_properties` Clock-delay range
validation) — need to be re-verified against current `HEAD` before
landing fixes, since 165 additional commits were reviewed after they
were first found and either could have been fixed incidentally in the
interim (as happened repeatedly elsewhere in this review). Per the plan,
fix commits land on a new branch off current `HEAD` (`master` is
protected), never rewriting the existing 393-commit history.
