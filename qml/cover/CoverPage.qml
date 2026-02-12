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
