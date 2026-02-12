import QtQuick 2.0
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0

Item {
    id: root

    property var cells: []
    // Optional: per-cell ship kind (0=none, 1=Destroyer, 2=Submarine, 3=Cruiser, 4=Battleship, 5=Carrier)
    property var shipKinds: []
    property int gridSize: 10
    // Whether the grid accepts taps. The page decides when this is enabled.
    property bool interactive: true
    property bool showCoordinates: true
    // When false, ships (cell value 2) are rendered like water. Useful for hiding enemy fleet.
    property bool showShips: false
    // When true, show ship sprites for fully sunk ships even if showShips is false (enemy grid).
    property bool showSunkShipSprites: false
    // Optional tint for ships (useful to distinguish enemy fleet)
    property bool tintShips: false
    // Use a solid tint color + separate opacity, because ColorOverlay already uses the
    // source alpha; mixing alpha into the color can make the result too subtle.
    property color tintColor: Theme.highlightColor
    property real tintOpacity: 0.35
    // Visual tuning (base opacity for *unsunk* ships).
    // Sunk ships use a higher opacity so they remain readable when revealed.
    property real shipSpriteOpacity: 0.65


    // When true, render ships using sprite images instead of per-cell coloured squares.
    // Ships are drawn behind the semi-transparent water cells.
    property bool useShipSprites: true

    // Optional label (used for debug output)
    property string title: ""

    // Optional highlight cell (x/y). Set to -1 to disable.
    property int highlightX: -1
    property int highlightY: -1

    // Optional logger hook (e.g. GameEngine with Q_INVOKABLE debug(QString))
    // Use an object reference rather than a bare function reference so the
    // invocation keeps the correct receiver.
    property var logger: null

    signal cellClicked(int x, int y)

    // Intentionally no logging here; callers can log if needed.

    // Coordinate gutter for labels
    // Gutter reserved for coordinate labels. Must NOT depend on cellPx to avoid binding loops.
    property int gutter: showCoordinates ? ((cellPxOverride > 0) ? cellPxOverride : 18) : 0

    // If >0, forces a specific cell size (used to fit both boards on screen)
    property int cellPxOverride: 0

    property int cellPx: (cellPxOverride > 0) ? cellPxOverride : 24

    width: gutter + cellPx * gridSize
    height: gutter + cellPx * gridSize

    function shipBaseColor(kind) {
        if (kind === 1) return "white"     // Destroyer
        if (kind === 2) return "yellow"    // Submarine
        if (kind === 3) return "orange"    // Cruiser
        if (kind === 4) return "green"     // Battleship
        if (kind === 5) return Theme.highlightColor // Carrier
        return Theme.highlightColor
    }

    function cellColor(v, kind) {
        if (v === 0) return Theme.rgba(Theme.primaryColor, 0.08)      // water/unknown
        if (v === 1) return Theme.rgba(Theme.primaryColor, 0.08)      // water internal
        // ships: when using sprites, keep cells as water and let the sprite layer draw ships
        if (v === 2) return Theme.rgba(Theme.primaryColor, 0.08)      // ship (drawn separately) / hidden
        if (v === 3) return Theme.rgba(Theme.primaryColor, 0.03)      // miss
        if (v === 4) return Theme.rgba(shipBaseColor(kind), 0.65)     // hit (ship-colored)
        if (v === 5) return Theme.rgba(shipBaseColor(kind), 0.75)     // sunk
        return Theme.rgba(Theme.primaryColor, 0.08)
    }

    function shipLength(kind) {
        if (kind === 1) return 2  // Destroyer
        if (kind === 2) return 3  // Submarine
        if (kind === 3) return 3  // Cruiser
        if (kind === 4) return 4  // Battleship
        if (kind === 5) return 5  // Carrier
        return 0
    }

    function shipSpriteFatness(kind) {
        // Non-uniform scaling makes ships appear less “narrow”.
        // Carrier already looks good; keep it closer to 1.0.
        if (kind === 5) return 1.08
        return 1.28
    }

function shipSprite(kind) {
        if (kind === 1) return "../assets/ships/ship_destroyer.png"
        if (kind === 2) return "../assets/ships/ship_submarine.png"
        if (kind === 3) return "../assets/ships/ship_cruiser.png"
        if (kind === 4) return "../assets/ships/ship_battleship.png"
        if (kind === 5) return "../assets/ships/ship_carrier.png"
        return ""
    }

    function idx(x, y) { return y * root.gridSize + x }

    function cellValAt(x, y) {
        if (!root.cells || root.cells.length <= 0) return 0
        if (x < 0 || y < 0 || x >= root.gridSize || y >= root.gridSize) return 0
        var i = idx(x, y)
        return (i >= 0 && i < root.cells.length) ? root.cells[i] : 0
    }

    function kindAt(x, y) {
        if (!root.shipKinds || root.shipKinds.length <= 0) return 0
        if (x < 0 || y < 0 || x >= root.gridSize || y >= root.gridSize) return 0
        var i = idx(x, y)
        return (i >= 0 && i < root.shipKinds.length) ? root.shipKinds[i] : 0
    }

    function isShipCell(x, y, kind) {
        var v = cellValAt(x, y)
        return (v === 2 || v === 4 || v === 5) && kindAt(x, y) === kind
    }

    function shipIsHorizontal(x, y, kind) {
        return isShipCell(x + 1, y, kind) || isShipCell(x - 1, y, kind)
    }

    function isShipOrigin(x, y, kind) {
        if (!isShipCell(x, y, kind)) return false
        var horiz = shipIsHorizontal(x, y, kind)
        if (horiz) return !isShipCell(x - 1, y, kind)
        return !isShipCell(x, y - 1, kind)
    }

    function shipFullySunk(ox, oy, kind, horiz, len) {
        // Fully sunk means every segment is marked as sunk (value 5).
        for (var i = 0; i < len; ++i) {
            var x = horiz ? (ox + i) : ox
            var y = horiz ? oy : (oy + i)
            if (cellValAt(x, y) !== 5) return false
        }
        return true
    }

    // Top letters
    Repeater {
        model: root.gridSize
        delegate: Label {
            visible: root.showCoordinates
            width: root.cellPx
            height: root.gutter
            x: root.gutter + index * root.cellPx
            y: 0
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: Theme.fontSizeTiny
            color: Theme.rgba(Theme.primaryColor, 0.7)
            text: String.fromCharCode("A".charCodeAt(0) + index)
        }
    }

    // Left numbers
    Repeater {
        model: root.gridSize
        delegate: Label {
            visible: root.showCoordinates
            width: root.gutter
            height: root.cellPx
            x: 0
            y: root.gutter + index * root.cellPx
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: Theme.fontSizeTiny
            color: Theme.rgba(Theme.primaryColor, 0.7)
            text: (index + 1).toString()
        }
    }

    // Ship sprite layer (drawn behind the cell rectangles)
    Repeater {
        model: root.gridSize * root.gridSize
        delegate: Item {
            // Per-index helpers
            property int cx: (index % root.gridSize)
            property int cy: Math.floor(index / root.gridSize)
            property int kind: kindAt(cx, cy)
            property bool horiz: shipIsHorizontal(cx, cy, kind)
            property int len: shipLength(kind)
            property bool sunk: (len > 0) ? shipFullySunk(cx, cy, kind, horiz, len) : false

            visible: root.useShipSprites && kind > 0 && isShipOrigin(cx, cy, kind) && len > 0 && (root.showShips || (root.showSunkShipSprites && sunk))

            // Ensure we sit behind the cell rectangles.
            z: -1

            x: root.gutter + cx * root.cellPx
            y: root.gutter + cy * root.cellPx
            width: horiz ? (len * root.cellPx) : root.cellPx
            height: horiz ? root.cellPx : (len * root.cellPx)

            // Keep ship graphics inside their bounding box
            clip: true

            // Use an explicit transform list so we can scale thickness without stretching length.
            // (Scaling the raw Image's Y axis + rotation can accidentally scale along the ship length.
            // This is what caused vertical ships to be truncated.)
            Item {
                id: shipLayer
                anchors.centerIn: parent
                width: len * root.cellPx
                height: root.cellPx

                transform: [
                    Rotation {
                        origin.x: shipLayer.width / 2
                        origin.y: shipLayer.height / 2
                        angle: horiz ? 0 : 90
                    },
                    Scale {
                        origin.x: shipLayer.width / 2
                        origin.y: shipLayer.height / 2
                        // After rotation, "thickness" axis swaps, so scale X for vertical ships.
                        xScale: horiz ? 1.0 : root.shipSpriteFatness(kind)
                        yScale: horiz ? root.shipSpriteFatness(kind) : 1.0
                    }
                ]

                Image {
                    id: shipImg
                    anchors.fill: parent
                    source: shipSprite(kind)
                    smooth: true
                    antialiasing: true
                    // Sunk ships should remain clearly visible when revealed.
                    opacity: sunk ? 1.0 : root.shipSpriteOpacity
                    fillMode: Image.PreserveAspectFit
                }

                ColorOverlay {
                    anchors.fill: shipImg
                    source: shipImg
                    color: root.tintColor
                    opacity: root.tintOpacity
                    visible: root.tintShips
                }
            }

        }
    }

    // Grid cells
    Repeater {
        model: gridSize * gridSize
        delegate: Rectangle {
            id: cellRect
            transformOrigin: Item.Center
            opacity: 1.0

            // For animations and logic
            property int cellValue: (root.cells && root.cells.length > index) ? root.cells[index] : 0
            property int shipKind: (root.shipKinds && root.shipKinds.length > index) ? root.shipKinds[index] : 0
            property int _lastCellValue: -999
        Component.onCompleted: { _lastCellValue = cellValue }
        function triggerPop() { markerPop.stop(); markerPop.start(); }
            onCellValueChanged: {
                if (_lastCellValue === -999) { _lastCellValue = cellValue; return; }
                if (cellValue !== _lastCellValue) {
                    _lastCellValue = cellValue;
                    triggerPop();
                }
            }

            SequentialAnimation {
                id: markerPop
                running: false
                ParallelAnimation {
                    NumberAnimation { target: cellRect; property: "scale"; from: 0.55; to: 1.35; duration: 140 }
                    NumberAnimation { target: cellRect; property: "opacity"; from: 0.55; to: 1.0; duration: 140 }
                }
                NumberAnimation { target: cellRect; property: "scale"; from: 1.35; to: 1.0; duration: 240; easing.type: Easing.OutBack }
            }

width: root.cellPx
            height: root.cellPx
            x: root.gutter + (index % root.gridSize) * root.cellPx
            y: root.gutter + Math.floor(index / root.gridSize) * root.cellPx
            color: root.cellColor(cellValue, shipKind)
            border.width: 1
            border.color: Theme.rgba(Theme.primaryColor, 0.20)
            radius: 2

            // Highlight last shot cell
            Rectangle {
                anchors.fill: parent
                visible: root.highlightX === (index % root.gridSize) && root.highlightY === Math.floor(index / root.gridSize)
                color: "transparent"
                border.width: 3
                border.color: Theme.highlightColor
                radius: 3
            }

            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.18
                height: width
                radius: width / 2
                visible: (root.cells && root.cells.length > index && root.cells[index] === 3)
                color: Theme.rgba(Theme.primaryColor, 0.35)
            }

            Rectangle {
                anchors.centerIn: parent
                width: parent.width * 0.45
                height: width
                radius: 4
                visible: (root.cells && root.cells.length > index && (root.cells[index] === 4 || root.cells[index] === 5))
                color: Theme.rgba(Theme.primaryColor, 0.10)
                border.width: 2
                border.color: Theme.highlightColor
            }

            MouseArea {
                anchors.fill: parent
                enabled: root.interactive

                // BoardGrid is embedded in a SilicaFlickable; prevent the flickable
                // from stealing taps as scroll gestures.
                preventStealing: true

                onPressed: mouse.accepted = true
                onClicked: {
                    var cx = index % root.gridSize
                    var cy = Math.floor(index / root.gridSize)
                    if (root.cellClicked) root.cellClicked(cx, cy)
                }
            }
        }
    }
}