import QtQuick 2.0
import Sailfish.Silica 1.0

CoverBackground {
    id: cover

    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.paddingLarge
        spacing: Theme.paddingMedium

        // App name
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Naval Battle")
            font.pixelSize: Theme.fontSizeSmall
            font.bold: true
            truncationMode: TruncationMode.Fade
        }

        // Icon
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            source: "/usr/share/icons/hicolor/108x108/apps/harbour-navalbattle.png"
            width: Theme.iconSizeLauncher
            height: Theme.iconSizeLauncher
            fillMode: Image.PreserveAspectFit
            smooth: true
        }

        // Game status
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: engine ? (engine.gameOver ? qsTr("Game ended")
                                           : (engine.setupMode ? qsTr("Placing fleet")
                                                              : qsTr("Game in progress")))
                         : ""
            color: Theme.highlightColor
            truncationMode: TruncationMode.Fade
        }
    }
}
