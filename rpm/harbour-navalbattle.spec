Name:       harbour-navalbattle
Summary:    Naval Battle
Version:    0.1
Release:        164
Group:      Applications/Games
License:    GPLv3+
URL:        https://example.invalid/
Source0:    %{name}-%{version}.tar.bz2

BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(sailfishapp)
BuildRequires:  cmake
BuildRequires:  make
BuildRequires:  gcc-c++

Requires:       sailfishsilica-qt5

%description
Battleship-style naval battle game for Sailfish OS. Single-player vs AI.

%prep
%setup -q -n %{name}

%build
%cmake .
%cmake_build

%install
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/86x86/apps
mkdir -p %{buildroot}%{_datadir}/icons/hicolor/108x108/apps
install -m 0644 icons/86x86/harbour-navalbattle.png %{buildroot}%{_datadir}/icons/hicolor/86x86/apps/harbour-navalbattle.png
install -m 0644 icons/108x108/harbour-navalbattle.png %{buildroot}%{_datadir}/icons/hicolor/108x108/apps/harbour-navalbattle.png
%cmake_install

%files
%defattr(-,root,root,-)
%{_datadir}/icons/hicolor/86x86/apps/harbour-navalbattle.png
%{_datadir}/icons/hicolor/108x108/apps/harbour-navalbattle.png
%{_bindir}/%{name}
%{_datadir}/%{name}/qml
%{_datadir}/applications/%{name}.desktop
%{_datadir}/metainfo/%{name}.appdata.xml

%changelog
* Tue Feb 03 2026 Miklós <n/a> - 0.1-164
- Settings: fix ComboBox currentIndex binding loop; disable Pace during battle

* Tue Feb 03 2026 Miklós <n/a> - 0.1-163
- Settings: move Show coordinates into Board section

* Tue Feb 03 2026 Miklós <n/a> - 0.1-162
- Hide AI delay and Hit/Miss duration sliders (use Pace)
- Rules: remove lengths from Ship colours section

* Tue Feb 03 2026 Miklós <n/a> - 0.1-161
- Phase 21D: use latest GamePage.qml; fix Rules ship row alignment; replace 3 timers UI with Pace; improve Auto cell size

* Tue Feb 03 2026 Miklós <n/a> - 0.1-160
- Fix GamePage.qml stray brace after id

* Tue Feb 03 2026 Miklós <n/a> - 0.1-159
- Fix GamePage.qml syntax and restore rebuildSetupHistory in onCompleted

* Tue Feb 03 2026 Miklós <n/a> - 0.1-158
- Fix QML: merge Component.onCompleted, break TextSwitch binding loop, define shotsOf()

* Tue Feb 03 2026 Miklós <n/a> - 0.1-157
- Build fix: showGameOverOverlay API, rebuildSetupHistory, lambda capture

* Tue Feb 03 2026 Miklós <n/a> - 0.1-156
- Phase 21C: disable difficulty changes during battle
- Add player shot counter and show in best times
- Add option to show/hide game over overlay
- Improve Rules ships section with images
- Rebuild setup undo history after restore

* Tue Feb 03 2026 Miklós <n/a> - 0.1-155
- Fix: package correct GamePage.qml (setup UI)

* Tue Feb 03 2026 Miklós <n/a> - 0.1-154
- Phase 21B: remap difficulty and add heatmap AI for Hard
- Update GamePage.qml to latest

* Tue Feb 03 2026 Miklós <n/a> - 0.1-153
- Restore correct GamePage.qml (setup UI + reveal) after regression

* Tue Feb 03 2026 Miklós <n/a> - 0.1-152
- Difficulty tuning: Easy/Medium/Hard AI behavior
- Update GamePage.qml (user fix)

* Tue Feb 03 2026 Miklós <n/a> - 0.1-151
- Fix missing closing brace in GamePage.qml

* Tue Feb 03 2026 Miklós <n/a> - 0.1-150
- Center setup buttons into 3 rows and fix Undo placement

* Tue Feb 03 2026 Miklós <n/a> - 0.1-149
- Fix Undo row layout warning and prevent ship sprite hits from blocking game-over detection
* Tue Feb 03 2026 Miklós <m@local> - 0.1-148
- Fix About page braces, add setup undo, ship-colored hit cells

* Tue Feb 03 2026 Miklos <you@example.com> 0.1-147
- Phase 21A: add setup undo and color hit cells by ship; fix About page braces
* Tue Feb 03 2026 Miklós <unknown> - 0.1-146
- UI: restore About page layout; fix Settings string quoting; make AI thinking popup toggle effective.
- Board: fix enemy tint overlay, improve ship sprite scaling for vertical orientation, and tweak opacities.

* Tue Feb 03 2026 Miklós <unknown> - 0.1-145
- Fix build: restore correct includes and Q_PROPERTY accessors for new settings.

* Tue Feb 03 2026 Miklós <unknown> - 0.1-144
- Ship sprite options (sunk visibility, tint), no-touch rule and popup toggles; best times by difficulty; About page fix
* Tue Feb 03 2026 Miklós <unknown> - 0.1-143
- Fix ship sprite origin detection across hit/sunk cells (avoid duplicate sprites)
* Tue Feb 03 2026 Miklós <you@example.invalid> - 0.1-142
- Fix packaging: install ship sprite PNG assets under share/harbour-navalbattle/qml.

* Tue Feb 03 2026 Miklós <you@example.invalid> - 0.1-141
- Add ship sprite graphics and render ships as images on the board.

* Mon Feb 02 2026 Miklós - 0.1-140
- Fix About page syntax and replace source link with centered button
- Improve Cover page layout: centered title/icon/status and use packaged icon path

* Mon Feb 02 2026 Miklós - 0.1-138
- Update Rules page to match current rules (battle timer, fleet adjacency, ship colour legend)
- Refresh About page layout and add source link

* Mon Feb 02 2026 Miklós - 0.1-137
- Fix game over overlay positioning and Reveal enemy fleet button enable logic

* Mon Feb 02 2026 Miklós <miklós@localhost> 0.1-136
- Fix: ensure game over state emits property change signals (player victory/defeat).
- UI: restore working game over overlay sizing/background (from prior version).
* Mon Feb 02 2026 Miklós <miklós@localhost> 0.1-135
- Fix game over overlay sizing/background/frame

* Mon Feb 02 2026 Miklós <you@example.invalid> - 0.1-134
- Fix game over overlay positioning (no illegal anchors) and restore panel background/frame
- Center setup button block; fix two-column width binding; disable placement buttons once all ships are placed
- Right-align page title in header
- Make manual placement respect the same no-touch rule used by auto-place/AI
- Improve AI hit messaging to include ship name

* Mon Feb 02 2026 Miklós <you@example.invalid> - 0.1-133
- UX polish: battle timer start, header timer placement, setup panel layout, AI popup, game over overlay, and action message formatting

* Mon Feb 02 2026 Miklós <you@example.invalid> - 0.1-132
- Phase 16: finalize manual placement (Column A fix) and remove debug logging

* Thu Jan 29 2026 Miklós <you@example.invalid> - 0.1-86
- Phase 9.1/9.2: remove end-game status duplication, add game timer, and harden restore of game-over state

* Wed Jan 29 2026 Miklós <you@example.invalid> - 0.1-85
- Fix Phase 8c: player name in fleet title/overlay text; adjust turn label size; restore save/load on startup

* Wed Jan 29 2026 Miklós <you@example.invalid> - 0.1-84
- Fix: always show reveal debug menu; use player name in turn and fleet title; bind overlay win/lose text

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-83
- Fix SettingsPage: remove settings shadowing; bind directly to context SettingsManager; tidy layout

* Thu Jan 29 2026 Miklós - 0.1-82
- Fix player name setting build (no missing member variable)
* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-81
- Phase 8c: add player name setting and use it in turn message; fix pulley menu entries for About/Rules

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-80
- Fix Phase 8b: add About and Game rules entries to pulley menu

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-79
- Phase 8b: add About and Game rules pages; hide win/lose text under title; clean thinking timer residue

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-78
- Fix GamePage.qml syntax: clean thinkingTimer handler

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-77
- Fix timers: bind AI delay setting to engine; enforce minimum visible thinking popup

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-76
- Fix thinking popup: enforce minimum visible duration using thinkingHoldMs

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-75
- Fix SettingsManager.h: declare setHitMissHoldMs/setThinkingHoldMs methods

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-74
- Fix SettingsManager.h: declare setHitMissHoldMs/setThinkingHoldMs for Q_PROPERTY WRITE

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-73
- Fix SettingsManager.h: add missing setHitMissHoldMs/setThinkingHoldMs declarations

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-72
- Fix SettingsManager header: declare setters for hit/miss and thinking durations

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-71
- Phase 8a: remove unused GameOverDialog; hide header status on game over; add configurable hit/miss and thinking durations

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-70
- Phase 7 final2: fix debug menu gating; persist/restore gameOver & reveal state; prevent actions after game over

* Tue Jan 28 2026 Miklós <you@example.invalid> - 0.1-68
- Adjust game-over overlay vertical offset to 0.3 screen height

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-67
- Fix GamePage overlay: rebuild with correct braces/ids, constrain height, prevent button overlap; hide status win/lose line when game over

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-66
- Fix GamePage.qml: remove stray closing brace and duplicate id in main Column

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-65
- Fix QML syntax: add missing newline after implicitHeight in game-over overlay panel

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-64
- Fix overlay QML: remove extra brace and let panel size to content; reset reveal state on new game

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-63
- Fix: always show Reveal button at game over (disable after reveal); hide win/lose status line when game over

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-62
- Phase 7: prevent firing during AI thinking/turn or after game over; dim enemy grid while disabled

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-61
- Make debug reveal menu always visible; restore A-J coordinate text in status; reset reveal flag on newGame so overlay button reappears

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-60
- Fix build: add public getters for playerWon/revealEnemyFleet and expose revealEnemyFleet property

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-59
- Fix build: add missing playerWon()/revealEnemyFleet() getters for Q_PROPERTY

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-58
- Fix enemy fleet reveal: correct enemyCells logic (was revealing only cell C1); keep playerWon exposed for overlay text

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-57
- Fix save/load: use toVariantMap/fromVariantMap and open QFile before read/write

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-56
- Fix save/load: open file correctly (avoid QIODevice device not open warnings)

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-55
- Fix game-over overlay text/placement; add debug reveal menu; increase AI delay max; remove qWarning logs

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-54
- Fix: expose playerWon/revealEnemyFleet to QML; make reveal refresh boards and clear shot highlights; slightly smaller game-over panel

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-53
- Fix playerWon flag for game-over overlay; add 50% dark dimmer behind popup; clear last-shot highlights when revealing; reset reveal on new game

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-52
- Fix build: enemyCells reveal uses correct local cell variable

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-51
- Fix GamePage extra brace; make game-over popup centered; restore enemy hit/miss rendering while keeping reveal

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-50
- Fix build: enemyCells no longer references undefined LastShot enum

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-49
- Improve game-over overlay visibility; fix enemy reveal; extend AI delay max; keep action text visible longer; show shot coordinates as letter+number

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-48
- Phase 6: show game-over as centered overlay; add Reveal enemy fleet + New game buttons

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-47
- Fix revealEnemyFleet setter: refresh UI via emitAllChanged

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-46
- Fix GameEngine.h: declare setRevealEnemyFleet

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-45
- Fix GameEngine.cpp: close clearSavedGame and add proper setRevealEnemyFleet implementation

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-43
- Fix GameEngine.cpp: setRevealEnemyFleet was inserted inside clearSavedGame

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-42
- Phase 6: add game-over dialog and optional enemy reveal on game end

* Tue Jan 27 2026 Miklós <you@example.invalid> - 0.1-41
- Update desktop file: Silica app type and disable sailjail sandboxing

* Mon Jan 26 2026 Miklós <you@example.invalid> - 0.1-40
- Increase cell size slider maximum to 100

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-39
- Fix BoardGrid QML syntax (cellPx line break)

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-38
- Fix settings cell size persistence (SettingsManager adds cellSizePx) and remove BoardGrid cellPx binding loop

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-37
- Fix SettingsPage syntax; remove BoardGrid cellPx binding loop on rotation

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-36
- Fix cell size slider: commit via onValueChanged for Silica Slider

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-35
- Fix cell size setting to apply via explicit setter and keep BoardGrid coordinate visibility clean

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-34
- Fix undefined QML bindings; make cell size slider robust

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-33
- Phase 5d: add configurable cell size; enlarge status text and match board title size

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-32
- Phase 5c: enlarge boards by adjusting cell sizing and compacting header text

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-31
- Phase 5b: compact UI and force board sizing to show both grids; true AI thinking overlay

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-30
- Phase 5: compact GamePage layout and overlay AI thinking indicator

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-29
- Fix coordinate toggle: Repeater has no visible property; hide labels via delegates

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-28
- Use QSettings-backed C++ SettingsStore (no Nemo QML plugin dependency)

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-27
- Fix SettingsStore type availability and remove duplicate showCoordinates bindings

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-26
- Fix SettingsPage to accept passed settings object (coordinates toggle)

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-25
- Fix SettingsPage QML string quoting

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-24
- Replace Qt.labs.settings with QSettings-based SettingsManager (Sailfish compatible)

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-23
- Phase 4: add Settings page (coordinates toggle, AI delay slider)

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-22
- Fix Phase 3: restore last-shot coords from save; delay AI turn to show thinking indicator

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-21
- Phase 3 polish: persist last-shot highlights reliably; ensure AI thinking indicator is visible

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-20
- Fix Phase 3 build: add missing last-shot member fields in GameEngine

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-19
- Fix missing last-shot member variables for Phase 3 build

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-18
- Phase 3: add coordinate labels and highlight last shots on both grids

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-17
- Make Clear saved game reset to a fresh game to prevent re-saving old state

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-16
- Fix persistence startup order and remove erroneous debug string escaping

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-13
- Force save path under ~/.local/share/harbour-navalbattle and add save/load diagnostics

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-12
- Fix persistence: load before starting new game; stabilize AppDataLocation; add diagnostics

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-11
- Phase 2: QML cleanup and game state persistence (save/restore)

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-10
- Avoid QML engine property assignment: expose engine via QQmlContext contextProperty

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-9
- Fix CoverPage engine property (declare engine: null) to avoid QML assignment error

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-8
- Fix QML binding loop by renaming engine property to gameEngine

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-7
- Pass engine instance into GamePage/CoverPage; remove missing cover icon to avoid warnings

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-6
- Fix incomplete QQuickView/QGuiApplication includes in main.cpp

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-5
- Replace QRandomGenerator with qrand/qsrand for older Qt5 compatibility

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-4
- Use pkg-config(sailfishapp) in CMake (no FindSailfishApp)

* Sat Jan 24 2026 Miklós <you@example.invalid> - 0.1-3
- Use CMake-only RPM build macros (remove qmake)