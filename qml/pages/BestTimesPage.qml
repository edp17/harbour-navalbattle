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

    // settings.bestTimes -> list of { name: string, seconds: int, difficulty: int(0..2), pace: int(-1..2) }

    function fmt(sec) {
        sec = (sec || 0)
        var m = Math.floor(sec / 60)
        var s = sec % 60
        var mm = (m < 10 ? "0" : "") + m
        var ss = (s < 10 ? "0" : "") + s
        return mm + ":" + ss
    }

    function whoOf(e) {
        return (e && e.name && e.name.length > 0) ? e.name : qsTr("Player")
    }

        function shotsOf(e) {
        return (e && e.shots !== undefined) ? Number(e.shots) : 0
    }

function secOf(e) {
        return (e && e.seconds !== undefined) ? Number(e.seconds) : 0
    }

    function diffOf(e) {
        return (e && e.difficulty !== undefined) ? Number(e.difficulty) : 1
    }

    function paceOf(e) {
        return (e && e.pace !== undefined) ? Number(e.pace) : -1
    }

    function paceLabel(p) {
        if (p === 0) return qsTr("Slow")
        if (p === 2) return qsTr("Fast")
        if (p === 1) return qsTr("Normal")
        return qsTr("Unknown")
    }

    function paceSuffix(e) {
        var p = paceOf(e)
        if (p < 0) return ""
        return "  (" + qsTr("Pace") + ": " + paceLabel(p) + ")"
    }

    function shotsSuffix(e) {
        var shots = shotsOf(e)
        return shots > 0 ? "  (" + qsTr("%1 shots").arg(shots) + ")" : ""
    }

    function timesForDifficulty(d) {
        var src = (settings && settings.bestTimes) ? settings.bestTimes : []
        var out = []
        for (var i = 0; i < src.length; ++i) {
            var e = src[i]
            if (diffOf(e) === d) out.push(e)
        }
        // Already sorted in C++, but keep stable just in case
        out.sort(function(a, b) { return secOf(a) - secOf(b) })
        if (out.length > 10) out = out.slice(0, 10)
        return out
    }

    property var easyTimes: []
    property var mediumTimes: []
    property var hardTimes: []

    readonly property bool hasAny: (easyTimes.length + mediumTimes.length + hardTimes.length) > 0

    function refreshTimes() {
        easyTimes = timesForDifficulty(0)
        mediumTimes = timesForDifficulty(1)
        hardTimes = timesForDifficulty(2)
    }

    function requestClearBestTimes() {
        if (!settings || !hasAny) return
        Remorse.popupAction(page, qsTr("Best times cleared"), function() {
            if (settings) settings.clearBestTimes()
        })
    }

    Component.onCompleted: refreshTimes()

    Connections {
        target: settings
        onBestTimesChanged: page.refreshTimes()
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Clear best times")
                enabled: page.hasAny
                onClicked: page.requestClearBestTimes()
            }
        }

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: qsTr("Best times") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2*Theme.horizontalPageMargin
                wrapMode: Text.WordWrap
                color: Theme.secondaryColor
                visible: !hasAny
                text: qsTr("Beat the enemy fleet.")
            }

            Column {
                width: parent.width
                visible: easyTimes.length > 0
                spacing: Theme.paddingSmall
                SectionHeader { text: qsTr("Easy") }
                Repeater {
                    model: easyTimes
                    delegate: Label {
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2*Theme.horizontalPageMargin
                        truncationMode: TruncationMode.Fade
                        text: "#" + (index + 1) + ".  " + whoOf(modelData) + "  " + fmt(secOf(modelData)) + shotsSuffix(modelData) + paceSuffix(modelData)
                    }
                }
                Item { width: 1; height: Theme.paddingMedium }
            }

            Column {
                width: parent.width
                visible: mediumTimes.length > 0
                spacing: Theme.paddingSmall
                SectionHeader { text: qsTr("Medium") }
                Repeater {
                    model: mediumTimes
                    delegate: Label {
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2*Theme.horizontalPageMargin
                        truncationMode: TruncationMode.Fade
                        text: "#" + (index + 1) + ".  " + whoOf(modelData) + "  " + fmt(secOf(modelData)) + shotsSuffix(modelData) + paceSuffix(modelData)
                    }
                }
                Item { width: 1; height: Theme.paddingMedium }
            }

            Column {
                width: parent.width
                visible: hardTimes.length > 0
                spacing: Theme.paddingSmall
                SectionHeader { text: qsTr("Hard") }
                Repeater {
                    model: hardTimes
                    delegate: Label {
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2*Theme.horizontalPageMargin
                        truncationMode: TruncationMode.Fade
                        text: "#" + (index + 1) + ".  " + whoOf(modelData) + "  " + fmt(secOf(modelData)) + shotsSuffix(modelData) + paceSuffix(modelData)
                    }
                }
                Item { width: 1; height: Theme.paddingMedium }
            }
        }
    }
}
