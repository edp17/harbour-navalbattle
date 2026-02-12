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
            }

            SectionHeader { text: qsTr("How to play") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("Tap a cell on the enemy board (Enemy waters) to fire a shot. "
                          + "A hit marks a ship segment; a miss marks water. "
                          + "The enemy then takes its turn automatically.")
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

                // Each row: image + name + length
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
                            text: "<span style='color:" + modelData.color + ";'>" + modelData.name + "</span> (" + modelData.len + ")"
                        }
                    }
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    text: qsTr("Ships may be placed next to each other.")
                    font.pixelSize: Theme.fontSizeSmall
                }
            }

            SectionHeader { text: qsTr("Ship colours") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                text: qsTr("Your ships are colour-coded to help identify them: ")
                      + "<span style='color:" + page.carrierColor + ";'>" + qsTr("Carrier") + "</span>, "
                      + "<span style='color:" + page.battleshipColor + ";'>" + qsTr("Battleship") + "</span>, "
                      + "<span style='color:" + page.cruiserColor + ";'>" + qsTr("Cruiser") + "</span>, "
                      + "<span style='color:" + page.submarineColor + ";'>" + qsTr("Submarine") + "</span>, "
                      + "<span style='color:" + page.destroyerColor + ";'>" + qsTr("Destroyer") + "</span>. "
                      + qsTr("Enemy ship colours are only shown when you reveal the enemy fleet.")
            }

            SectionHeader { text: qsTr("Settings") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("You can change the cell size, AI delay, and the hold time of messages (Hit/Miss and AI thinking). "
                          + "You can also set your player name and toggle coordinate labels.")
            }

            SectionHeader { text: qsTr("Timer and Best times") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("The timer starts when the battle begins and stops when the game ends. "
                          + "Best times are recorded only when you win, and only the top 10 fastest wins are kept. "
                          + "Each best time entry includes the player name and the completion time.")
            }

            SectionHeader { text: qsTr("Reveal enemy fleet") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                text: qsTr("At the end of the game you can optionally reveal the enemy fleet to see remaining ships. "
                          + "During development a debug menu item may also expose this option.")
            }

            Item { height: Theme.paddingLarge; width: 1 }
        }
    }
}
