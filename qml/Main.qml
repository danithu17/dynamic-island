import QtQuick
import Aura 1.0

IslandWindow {
    id: mainWindow
    width: 540
    height: 360
    title: "Aura Island"
    visible: true
    color: "transparent"

    Island {
        id: islandItem
        anchors.top: parent.top
        anchors.topMargin: 8
        anchors.horizontalCenter: parent.horizontalCenter

        onXChanged: mainWindow.updatePillBounds(x, y, width, height)
        onYChanged: mainWindow.updatePillBounds(x, y, width, height)
        onWidthChanged: mainWindow.updatePillBounds(x, y, width, height)
        onHeightChanged: mainWindow.updatePillBounds(x, y, width, height)
        Component.onCompleted: mainWindow.updatePillBounds(x, y, width, height)
    }
}
