import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    // settings.bestTimes -> list of { name: string, seconds: int, difficulty: int(0..2) }

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

    readonly property var easyTimes: timesForDifficulty(0)
    readonly property var mediumTimes: timesForDifficulty(1)
    readonly property var hardTimes: timesForDifficulty(2)

    readonly property bool hasAny: (easyTimes.length + mediumTimes.length + hardTimes.length) > 0

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height

        PullDownMenu {
            MenuItem {
                text: qsTr("Clear best times")
                onClicked: if (settings) settings.clearBestTimes()
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

            // Sections
            function section(title, model) {
                // dummy
                return null
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
                        text: "#" + (index + 1) + ".  " + whoOf(modelData) + "  " + fmt(secOf(modelData)) + (shotsOf(modelData) > 0 ? "  (#shots: " + shotsOf(modelData) + ")" : "")
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
                        text: "#" + (index + 1) + ".  " + whoOf(modelData) + "  " + fmt(secOf(modelData)) + (shotsOf(modelData) > 0 ? "  (#shots: " + shotsOf(modelData) + ")" : "")
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
                        text: "#" + (index + 1) + ".  " + whoOf(modelData) + "  " + fmt(secOf(modelData)) + (shotsOf(modelData) > 0 ? "  (#shots: " + shotsOf(modelData) + ")" : "")
                    }
                }
                Item { width: 1; height: Theme.paddingMedium }
            }
        }
    }
}
