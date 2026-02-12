import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All


    
    
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

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
 

            

            SectionHeader { text: "Board" }

            TextSwitch {
                text: "Show coordinates (A–J / 1–10)"
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
                       ? "Cell size: Auto"
                       : ("Cell size: " + Math.round(value) + " px")
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
                text: "Set to Auto (0) to let the app fit both boards automatically."
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader { text: "AI" }

            ComboBox {
                width: parent.width
                label: qsTr("Difficulty")
                enabled: !engine || engine.setupMode || engine.gameOver
                description: qsTr("Easy: random shots. Medium: basic target mode. Hard: target + parity hunt.")
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
                description: qsTr("Controls AI speed and message timing")
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
                text: 'The AI delay makes the "AI is thinking…" state visible and can reduce accidental rapid taps.'
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

        }
    }
}