import QtQuick 2.7
import QtQuick.Controls 2.0

import RayCast 1.0

ApplicationWindow {
    id: window
    color: "white"
    visible: true
    width: 600
    height: 400
    title: "Grid Line Intersection"

    TileRayCast {
        id: tileRayCast
        startPos: Qt.point(40, 40)
        endPos: Qt.point(180, 210)
        sizeInTiles: Qt.size(Math.floor(window.width / tileSize), Math.floor(window.height / tileSize))

        MouseArea {
            anchors.fill: parent
            onPressed: tileRayCast.startPos = Qt.point(mouseX, mouseY)
            onPositionChanged: tileRayCast.endPos = Qt.point(mouseX, mouseY)
            onReleased: tileRayCast.endPos = Qt.point(mouseX, mouseY)
        }
    }
}
