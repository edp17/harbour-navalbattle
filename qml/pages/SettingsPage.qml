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

Page {
    id: page
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Best times")
                onClicked: pageStack.push(Qt.resolvedUrl("BestTimesPage.qml"))
            }
        }

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader { title: qsTr("Settings") }

            SectionHeader {
                text: qsTr("Player")
            }

            TextField {
                width: parent.width
                label: qsTr("Player name")
                placeholderText: qsTr("Optional")
                text: settings.playerName ? settings.playerName : ""
                EnterKey.enabled: true
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                onTextChanged: settings.playerName = text
            }

            SectionHeader { text: qsTr("Board") }

            TextSwitch {
                text: qsTr("Show coordinates (A–L / 1–12)")
                checked: settings ? settings.showCoordinates : true
                onCheckedChanged: if (settings) settings.showCoordinates = checked
            }


            Slider {
                id: cellSizeSlider
                width: parent.width
                minimumValue: 0
                maximumValue: 100
                stepSize: 1
                value: (settings && settings.cellSizePx !== undefined) ? settings.cellSizePx : 0
                label: (Math.round(value) === 0)
                       ? qsTr("Cell size: Auto")
                       : qsTr("Cell size: %1 px").arg(Math.round(value))
                onValueChanged: {
                    if (settings && settings.setCellSizePx) {
                        settings.setCellSizePx(Math.round(value))
                    }
                }
            }

            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.rgba(Theme.primaryColor, 0.65)
                text: qsTr("Set to Auto (0) to let the app fit both boards automatically.")
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader { text: qsTr("AI") }

            ComboBox {
                width: parent.width
                label: qsTr("Difficulty")
                enabled: !engine || engine.setupMode || engine.gameOver
                description: qsTr("Easy: checkerboard hunt. Medium: improved targeting. Hard: probability-based hunt.")
                currentIndex: settings ? settings.aiDifficulty : 1
                menu: ContextMenu {
                    MenuItem { text: qsTr("Easy") }
                    MenuItem { text: qsTr("Medium") }
                    MenuItem { text: qsTr("Hard") }
                }
                onCurrentIndexChanged: {
                    if (!settings) return
                    if (settings.aiDifficulty !== currentIndex)
                        settings.aiDifficulty = currentIndex
                }
            }
            ComboBox {
                label: qsTr("Pace")
                enabled: !engine || engine.setupMode || engine.gameOver
                description: qsTr("Controls how long the AI waits before firing")
                menu: ContextMenu {
                    MenuItem { text: qsTr("Slow") }
                    MenuItem { text: qsTr("Normal") }
                    MenuItem { text: qsTr("Fast") }
                }
                currentIndex: settings ? settings.aiPace : 1
                onCurrentIndexChanged: {
                    if (!settings) return
                    if (settings.aiPace !== currentIndex)
                        settings.aiPace = currentIndex
                }
            }




            Label {
                width: parent.width
                wrapMode: Text.WordWrap
                color: Theme.rgba(Theme.primaryColor, 0.65)
                text: qsTr("The selected pace controls how long the AI thinking state remains visible.")
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader { text: qsTr("Gameplay") }

            TextSwitch {
                text: qsTr("Allow ships to touch")
                description: qsTr("When off, ships must not touch each other, even diagonally.")
                checked: settings ? !settings.noTouchRule : true
                onCheckedChanged: if (settings) settings.noTouchRule = !checked
            }

            TextSwitch {
                text: qsTr("Show 'AI Thinking…' popup")
                checked: settings ? settings.showThinkingPopup : true
                onCheckedChanged: if (settings) settings.showThinkingPopup = checked
            }

            TextSwitch {
                text: qsTr("Show game over overlay")
                description: qsTr("Show the win/lose overlay at end of the game")
                checked: settings ? settings.showGameOverOverlay : true
                onCheckedChanged: {
                    if (!settings) return
                    if (settings.showGameOverOverlay !== checked)
                        settings.showGameOverOverlay = checked
                }
            }

            TextSwitch {
                text: qsTr("Show sunk ship images")
                description: qsTr("When enemy ships are sunk, show their images instead of only colored cells.")
                checked: settings ? settings.showSunkShipSprites : true
                onCheckedChanged: if (settings) settings.showSunkShipSprites = checked
            }

            TextSwitch {
                text: qsTr("Tint enemy ships")
                description: qsTr("Apply a subtle tint to enemy ship images when they are shown.")
                checked: settings ? settings.tintEnemyShips : false
                onCheckedChanged: if (settings) settings.tintEnemyShips = checked
            }

            SectionHeader { text: qsTr("Feedback") }

            TextSwitch {
                text: qsTr("Haptic feedback")
                description: qsTr("Vibrate when your shot reaches the target.")
                checked: settings ? settings.hapticFeedback : true
                onCheckedChanged: if (settings) settings.hapticFeedback = checked
            }

            TextSwitch {
                text: qsTr("Shot sounds")
                description: qsTr("Play different sounds for water and ship hits.")
                checked: settings ? settings.soundEffects : true
                onCheckedChanged: if (settings) settings.soundEffects = checked
            }
        }
    }
}
