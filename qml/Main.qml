import QtQuick
import Aura 1.0

IslandWindow {
    id: mainWindow
    title: "Aura Island"
    visible: true

    Island {
        id: islandItem
        anchors.fill: parent
    }
}
