/*
    Copyright (C) 2026 edp17 and chatGPT

    This file is part of harbour-navalbattle.

    The harbour-navalbattle is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The harbour-navalbattle is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with the harbour-navalbattle. If not, see <http://www.gnu.org/licenses/>.
*/
import QtQuick 2.0
import Sailfish.Silica 1.0

import "../components"

Page {
    // Keep engine placement rules in sync with Settings
    Binding {
        target: engine
        property: "noTouchRule"
        value: settings ? settings.noTouchRule : false
        when: engine !== null && settings !== null
    }

    id: page

    // Provided as context properties from C++ (main.cpp)
    // - engine (GameEngine)
    // - settings (SettingsManager)

        // Auto cell size: pick a comfortable size for the current layout.
    // If it doesn't fit vertically, the page will scroll (preferred over tiny boards).
    function autoCellPx(gridSize) {
        var margins = 2 * Theme.horizontalPageMargin
        var gutter = (settings && settings.showCoordinates === false) ? 0 : 18
        var availW = page.width - margins - gutter
        var pxW = Math.floor(availW / gridSize)

        // Sensible minimum: avoid "tiny" boards.
        var minPx = Math.round(Theme.itemSizeSmall * 0.60)
        var px = Math.max(pxW, minPx)

        // Avoid absurdly large cells on tablets.
        var maxPx = Math.round(Theme.itemSizeLarge * 0.95)
        return Math.min(px, maxPx)
    }

    function playerNameSafe() {
        if (settings && settings.playerName && settings.playerName.trim().length > 0)
            return settings.playerName.trim();
        return qsTr("You");
    }

    function fleetTitleText() {
        var n = playerNameSafe();
        // Apostrophe escaping for QML string literal
        return qsTr("%1's fleet").arg(n);
    }

    function fmtTime(sec) {
        sec = Math.max(0, Math.floor(sec || 0))
        var mm = Math.floor(sec / 60)
        var ss = sec % 60
        function pad2(v) { return (v < 10) ? ("0" + v) : ("" + v) }
        return pad2(mm) + ":" + pad2(ss)
    }


    function coordText(x, y) {
        if (x === undefined || y === undefined) return "";
        if (x < 0 || y < 0) return "";
        return String.fromCharCode('A'.charCodeAt(0) + x) + ", " + (y + 1)
    }

    function formatBattleAction() {
        if (!engine) return "";
        var a = engine.lastAction || "";
        if (a.length === 0) return "";

        // Setup-phase messages are shown inside the setup panel.
        if (engine.setupMode) return "";

        // Game over: keep the engine message (overlay already says win/lose).
        if (engine.gameOver) return a;

        // Player just fired -> we are in AI turn now
        if (!engine.playerTurn) {
            var c1 = coordText(engine.lastPlayerShotX, engine.lastPlayerShotY);
            if (a === "Miss") return playerNameSafe() + " fired at " + c1 + " - Miss";
            if (a.indexOf("Hit ") === 0) return playerNameSafe() + " fired at " + c1 + " - Hit " + a.substring(4);
            if (a.indexOf("Sunk ") === 0) return playerNameSafe() + " fired at " + c1 + " - Sunk " + a.substring(5);
            return a;
        }

        // AI just fired -> it is our turn
        var c2 = coordText(engine.lastAiShotX, engine.lastAiShotY);
        if (a === "AI missed") return "AI fired at " + c2 + " - Miss";

        var hitPrefix = "AI hit your ";
        if (a.indexOf(hitPrefix) === 0) return "AI fired at " + c2 + " - Hit your " + a.substring(hitPrefix.length);

        var sunkPrefix = "AI sunk your ";
        if (a.indexOf(sunkPrefix) === 0) return "AI fired at " + c2 + " - Sunk your " + a.substring(sunkPrefix.length);
        return a;
    }

    function allPlaced() {
        if (!engine) return false;
        var p = engine.setupPlaced; // QVariantList -> JS array
        if (!p || p.length === 0) return false;
        for (var i = 0; i < p.length; ++i) {
            if (!p[i]) return false;
        }
        return true;
    }

    function nextUnplacedIndex(fromIdx) {
        if (!engine) return 0;
        var p = engine.setupPlaced;
        var n = (p && p.length) ? p.length : 5;
        var idx = fromIdx;
        for (var k = 0; k < n; ++k) {
            var j = (idx + k) % n;
            if (!p || p.length === 0 || !p[j]) return j;
        }
        return fromIdx;
    }

    Component.onCompleted: {
        if (engine && engine.setupMode) engine.rebuildSetupHistory()
        if (engine && settings) {
            engine.aiDelayMs = settings.aiDelayMs;
            engine.aiThinkingHoldMs = settings.thinkingHoldMs;
            engine.hitMissHoldMs = settings.hitMissHoldMs;
            engine.aiDifficulty = settings.aiDifficulty;
        }
    }

    onStatusChanged: {
        if (status === PageStatus.Activating && engine && settings) {
            engine.aiDelayMs = settings.aiDelayMs;
            engine.aiThinkingHoldMs = settings.thinkingHoldMs;
            engine.hitMissHoldMs = settings.hitMissHoldMs;
            engine.aiDifficulty = settings.aiDifficulty;
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.implicitHeight + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                id: pageHeader
                title: "" // custom title row so timer can live on the same line

                Label {
                    id: headerTimerLabel
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryColor
                    text: (engine && !engine.setupMode) ? fmtTime(engine.elapsedSeconds) : "00:00"
                }

                Label {
                    id: headerTitleLabel
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: Theme.fontSizeLarge
                    color: Theme.primaryColor
                    text: qsTr("Naval Battle")
                }
            }

            // Battle messages (during setup these are shown inside the setup panel)
            Item {
                width: parent.width
                height: (!engine || engine.setupMode) ? 0 : actionLabel.implicitHeight
                visible: engine && !engine.setupMode

                Label {
                    id: actionLabel
                    anchors.left: parent.left
                    anchors.leftMargin: Theme.horizontalPageMargin
                    anchors.right: parent.right
                    anchors.rightMargin: Theme.horizontalPageMargin
                    anchors.verticalCenter: parent.verticalCenter
                    truncationMode: TruncationMode.Fade
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.secondaryColor
                    text: formatBattleAction()
                }
            }

            // Enemy board title
            Label {
                width: parent.width
                text: qsTr("Enemy waters")
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                horizontalAlignment: Text.AlignHCenter
            }

            // Enemy board
            BoardGrid {
                id: enemyGrid
                anchors.horizontalCenter: parent.horizontalCenter
                gridSize: 12
                cellPxOverride: (settings && settings.cellSizePx > 0) ? settings.cellSizePx : autoCellPx(gridSize)
                showCoordinates: settings ? settings.showCoordinates : true
                showShips: engine ? engine.revealEnemyFleet : false
                showSunkShipSprites: settings ? settings.showSunkShipSprites : true
                tintShips: settings ? settings.tintEnemyShips : false
                shipKinds: engine ? engine.enemyShipKinds : []
                cells: engine ? engine.enemyCells : []
                highlightX: engine ? engine.lastPlayerShotX : -1
                highlightY: engine ? engine.lastPlayerShotY : -1
                enabled: engine ? (!engine.gameOver && !engine.setupMode) : false
                interactive: engine ? (!engine.gameOver && !engine.setupMode && engine.playerTurn) : false
                logger: engine
                title: "enemy"
                onCellClicked: {
                    if (engine && engine.debug) {
                        engine.debug("enemyGrid cellClicked x=" + x + " y=" + y
                                     + " enabled=" + enemyGrid.enabled
                                     + " setupMode=" + engine.setupMode
                                     + " playerTurn=" + engine.playerTurn
                                     + " gameOver=" + engine.gameOver)
                    }
                    if (engine) engine.playerFire(x, y)
                }
            }

            // Setup panel (only during placement)
            Item {
                width: parent.width
                height: engine && engine.setupMode ? setupPanel.implicitHeight : 0
                visible: engine && engine.setupMode

                Column {
                    id: setupPanel
                    width: parent.width
                    spacing: Theme.paddingSmall

                    Label {
                        width: parent.width
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryHighlightColor
                        horizontalAlignment: Text.AlignHCenter
                        text: qsTr("Place your fleet")
                    }

                    Label {
                        width: parent.width
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                        horizontalAlignment: Text.AlignHCenter
                        truncationMode: TruncationMode.Fade
                        text: engine ? engine.lastAction : ""
                    }
                    // Two-row layout for setup actions (centered block)
                    Column {
                        id: setupButtons
                        spacing: Theme.paddingSmall
                        anchors.horizontalCenter: parent.horizontalCenter

                        // Keep the 2x2 block visually centered regardless of page width
                        property int twoColWidth: Math.floor((Math.min(parent.width - 2 * Theme.horizontalPageMargin,
                                                                      Theme.itemSizeHuge * 4)
                                                             - Theme.paddingSmall) / 2)

                        width: twoColWidth * 2 + Theme.paddingSmall

                        Row {
                            width: parent.width
                            spacing: Theme.paddingSmall
                            anchors.horizontalCenter: parent.horizontalCenter

                            Button {
                                id: shipButton
                                width: setupButtons.twoColWidth
                                anchors.verticalCenter: parent.verticalCenter
                                enabled: !allPlaced()
                                text: {
                                    // display current ship selection
                                    var names = [
                                        qsTr("Carrier (5)"),
                                        qsTr("Battleship (4)"),
                                        qsTr("Cruiser (3)"),
                                        qsTr("Submarine (3)"),
                                        qsTr("Destroyer (2)")
                                    ];
                                    var idx = engine ? engine.setupSelectedIndex : 0;
                                    if (idx < 0 || idx >= names.length) idx = 0;
                                    return names[idx];
                                }
                                onClicked: {
                                    if (!engine) return;
                                    var idx = (engine.setupSelectedIndex + 1) % 5;
                                    idx = nextUnplacedIndex(idx);
                                    engine.setupSelectedIndex = idx;
                                }
                            }

                            Button {
                                id: orientButton
                                width: setupButtons.twoColWidth
                                anchors.verticalCenter: parent.verticalCenter
                                text: (engine && engine.setupHorizontal) ? qsTr("Horizontal") : qsTr("Vertical")
                                onClicked: {
                                    if (engine) engine.rotateSetupShip();
                                }
                                enabled: !allPlaced()
                            }
                        }

                        Row {
                            width: parent.width
                            spacing: Theme.paddingSmall
                            anchors.horizontalCenter: parent.horizontalCenter

                            Button {
                                id: autoButton
                                width: setupButtons.twoColWidth
                                anchors.verticalCenter: parent.verticalCenter
                                text: qsTr("Auto-place")
                                onClicked: {
                                    if (engine) engine.autoPlacePlayerFleet();
                                }
                                enabled: !allPlaced()
                            }

                            Button {
                                id: undoButton
                                width: setupButtons.twoColWidth
                                anchors.verticalCenter: parent.verticalCenter
                                text: qsTr("Undo")
                                enabled: engine && engine.setupMode && engine.setupHistoryCount > 0
                                onClicked: { if (engine) engine.undoSetupShip(); }
                            }
                        }

                        Row {
                            width: parent.width
                            spacing: Theme.paddingSmall
                            anchors.horizontalCenter: parent.horizontalCenter

                            Button {
                                id: startBattleButton
                                width: setupButtons.twoColWidth * 2 + Theme.paddingSmall
                                anchors.verticalCenter: parent.verticalCenter
                                text: qsTr("Start battle")
                                enabled: allPlaced()
                                onClicked: {
                                    if (engine) engine.startBattle();
                                }
                            }
                        }


                    }
                }
            }

            // Player board title
            Label {
                width: parent.width
                text: fleetTitleText()
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                horizontalAlignment: Text.AlignHCenter
            }

            // Player board
            BoardGrid {
                id: playerGrid
                anchors.horizontalCenter: parent.horizontalCenter
                gridSize: 12
                cellPxOverride: (settings && settings.cellSizePx > 0) ? settings.cellSizePx : autoCellPx(gridSize)
                showCoordinates: settings ? settings.showCoordinates : true
                shipKinds: engine ? engine.playerShipKinds : []
                showShips: true
                cells: engine ? engine.playerCells : []
                highlightX: engine ? engine.lastAiShotX : -1
                highlightY: engine ? engine.lastAiShotY : -1
                enabled: engine ? (engine.setupMode && !engine.gameOver) : false
                logger: engine
                title: "player"
                onCellClicked: {
                    if (!engine) return;
                    if (!engine.setupMode) return;
                    var ok = engine.placeSetupShip(engine.setupSelectedIndex, x, y)
                }
            }
        }

        PullDownMenu {
            MenuItem {
                text: qsTr("New game")
                onClicked: if (engine) engine.newGame();
            }

            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }

            MenuItem {
                text: qsTr("Best times")
                onClicked: pageStack.push(Qt.resolvedUrl("BestTimesPage.qml"))
            }

            MenuItem {
                text: qsTr("Game Rules")
                onClicked: pageStack.push(Qt.resolvedUrl("RulesPage.qml"))
            }

            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        }

        // AI thinking popup (center)
        Item {
            anchors.fill: parent
            visible: engine && !engine.gameOver && !engine.setupMode && !engine.playerTurn && (settings ? settings.showThinkingPopup : true)
            Rectangle {
                width: Theme.itemSizeLarge * 4
                height: Theme.itemSizeLarge * 1.2
                anchors.centerIn: parent
                color: Theme.rgba("black", 0.6)
                radius: Theme.paddingLarge
                border.color: Theme.rgba(Theme.highlightColor, 0.6)
                border.width: 2
                Label {
                    anchors.centerIn: parent
                    text: qsTr("AI is thinking…")
                    font.pixelSize: Theme.fontSizeMedium
                    color: Theme.primaryColor
                }
            }
        }

        // Game over overlay (centered over lower part)
        Item {
            id: gameOverOverlay
            anchors.fill: parent
            visible: engine && engine.gameOver && (!settings || settings.showGameOverOverlay)

                        Rectangle {
                id: panel
                width: Math.min(parent.width - 2 * Theme.horizontalPageMargin, Theme.itemSizeLarge * 4.5)
                anchors.horizontalCenter: parent.horizontalCenter

                // Position overlay aligned to the top of the player's grid (under the fleet title).
                anchors.verticalCenter: parent.verticalCenter
                anchors.verticalCenterOffset: Math.round(parent.height * 0.25)

                color: Theme.rgba("black", 0.85)
                border.color: Theme.rgba(Theme.highlightColor, 0.85)
                border.width: 2
                radius: Theme.paddingLarge

                // Ensure the panel has a real size; otherwise background/border won't be visible.
                height: Math.min(overlayCol.implicitHeight + 2 * Theme.paddingLarge,
                                 parent.height - 2 * Theme.paddingLarge)
                clip: true

                SilicaFlickable {
                    anchors.fill: parent
                    contentHeight: overlayCol.implicitHeight + 2 * Theme.paddingLarge

                    Column {
                        id: overlayCol
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: Theme.paddingLarge
                        spacing: Theme.paddingMedium

                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            color: Theme.rgba(Theme.primaryColor, 0.85)
                            font.pixelSize: Theme.fontSizeExtraLarge
                            text: (engine && engine.playerWon) ? (playerNameSafe() + qsTr(" won!")) : (playerNameSafe() + qsTr(" lost!"))
                        }

                        Label {
                            width: parent.width
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter
                            color: Theme.rgba(Theme.primaryColor, 0.75)
                            text: (engine && engine.playerWon)
                                  ? qsTr("You sank all enemy ships.")
                                  : qsTr("The enemy sank your fleet.")
                        }

                        Button {
                            width: parent.width
                            text: qsTr("Reveal enemy fleet")
                            enabled: engine && !engine.revealEnemyFleet
                            onClicked: if (engine) engine.setRevealEnemyFleet(true)
                        }

                        Button {
                            width: parent.width
                            text: qsTr("New game")
                            onClicked: if (engine) engine.newGame()
                        }
                    }
                }
            }
        }
    }
}
