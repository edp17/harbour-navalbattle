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

    // Keep ship colours consistent with BoardGrid.qml shipBaseColor().
    readonly property string destroyerColor: "white"
    readonly property string submarineColor: "yellow"
    readonly property string cruiserColor: "orange"
    readonly property string battleshipColor: "green"
    readonly property string carrierColor: Theme.highlightColor

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader { title: qsTr("Game Rules") }

            SectionHeader { text: qsTr("Goal") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("Sink all enemy ships before the enemy sinks your fleet.")
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader { text: qsTr("How to play") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("Tap a cell on the enemy board (Enemy waters) to fire a shot. "
                          + "A hit marks a ship segment; a miss marks water. "
                          + "The enemy then takes its turn automatically.")
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader { text: qsTr("Board and coordinates") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("The battle is played on a 12×12 grid. Coordinates use columns A–L and rows 1–12.")
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader { text: qsTr("Ships") }
            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                spacing: Theme.paddingSmall

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: qsTr("Standard fleet:")
                    font.pixelSize: Theme.fontSizeSmall
                }

                Repeater {
                    model: [
                        { img: "ship_carrier.png", name: qsTr("Carrier"), len: 5, color: page.carrierColor },
                        { img: "ship_battleship.png", name: qsTr("Battleship"), len: 4, color: page.battleshipColor },
                        { img: "ship_cruiser.png", name: qsTr("Cruiser"), len: 3, color: page.cruiserColor },
                        { img: "ship_submarine.png", name: qsTr("Submarine"), len: 3, color: page.submarineColor },
                        { img: "ship_destroyer.png", name: qsTr("Destroyer"), len: 2, color: page.destroyerColor }
                    ]
                    delegate: Row {
                        width: parent.width
                        spacing: Theme.paddingMedium
                        height: shipImg.height

                        Image {
                            id: shipImg
                            source: "../assets/ships/" + modelData.img
                            width: Theme.itemSizeLarge * 2.2
                            height: Theme.itemSizeSmall * 1.2
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }

                        Label {
                            width: parent.width - (Theme.itemSizeLarge * 2.2) - Theme.paddingMedium
                            height: shipImg.height
                            verticalAlignment: Text.AlignVCenter
                            wrapMode: Text.WordWrap
                            textFormat: Text.RichText
                            text: "   <span style='color:" + modelData.color + ";'>" + modelData.name + "</span> (" + modelData.len + ")"
                            font.pixelSize: Theme.fontSizeSmall
                        }
                    }
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    font.pixelSize: Theme.fontSizeSmall
                    text: qsTr("By default, ships may touch. You can enable the no-touch rule in Settings to prevent ships from touching each other (even diagonally).")
                }
            }

            SectionHeader { text: qsTr("Ship colours") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                text: qsTr("Ships are colour-coded to help identify them: ")
                      + "<span style='color:" + page.carrierColor + ";'>" + qsTr("Carrier") + "</span>, "
                      + "<span style='color:" + page.battleshipColor + ";'>" + qsTr("Battleship") + "</span>, "
                      + "<span style='color:" + page.cruiserColor + ";'>" + qsTr("Cruiser") + "</span>, "
                      + "<span style='color:" + page.submarineColor + ";'>" + qsTr("Submarine") + "</span>, "
                      + "<span style='color:" + page.destroyerColor + ";'>" + qsTr("Destroyer") + "</span>. "
                      + qsTr("Enemy ships are only shown when you reveal the enemy fleet.")
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader { text: qsTr("Settings") }
            Column {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                spacing: Theme.paddingSmall

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: qsTr("In Settings you can customize gameplay and the UI:")
                    font.pixelSize: Theme.fontSizeSmall
                }

                // Bullet list
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • set your player name");font.pixelSize: Theme.fontSizeSmall }
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • show/hide coordinates");font.pixelSize: Theme.fontSizeSmall }
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • choose cell size");font.pixelSize: Theme.fontSizeSmall }
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • AI difficulty");font.pixelSize: Theme.fontSizeSmall }
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • game pace");font.pixelSize: Theme.fontSizeSmall }
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • enable/disable the no-touch rule");font.pixelSize: Theme.fontSizeSmall }
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • enable/disable the 'AI Thinking…' popup");font.pixelSize: Theme.fontSizeSmall }
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • show/hide the game over overlay");font.pixelSize: Theme.fontSizeSmall }
                Label { width: parent.width; wrapMode: Text.WordWrap; text: qsTr("  • control how sunk enemy ships are displayed (images and tint)");font.pixelSize: Theme.fontSizeSmall }
            }

            SectionHeader { text: qsTr("Timer and Best times") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("The timer starts when the battle begins and stops when the game ends. "
                          + "Best times are recorded only when you win, and only the top 10 fastest wins are kept for each difficulty. "
                          + "Each entry includes the player name, completion time, and may also include shot count and pace.")
                font.pixelSize: Theme.fontSizeSmall
            }

            SectionHeader { text: qsTr("Reveal enemy fleet") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("At the end of the game you can optionally reveal the enemy fleet to see remaining ships. "
                          + "When enabled in Settings, sunk enemy ships can be shown using ship images, optionally tinted.")
                font.pixelSize: Theme.fontSizeSmall
            }

            Item { height: Theme.paddingLarge; width: 1 }
        }
    }
}
