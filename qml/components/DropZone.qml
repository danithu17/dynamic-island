import QtQuick
import ".."

Item {
    id: root
    signal droppedFiles(var urls)
    signal droppedText(string text)

    anchors.fill: parent

    DropArea {
        id: dropArea
        anchors.fill: parent

        onEntered: function(drag) {
            if (drag.hasUrls || drag.hasText) {
                drag.acceptProposedAction()
                if (appState && appState.islandState !== "drop") {
                    appState.setIslandState("drop")
                }
            }
        }

        onDropped: function(drop) {
            if (drop.hasUrls) {
                if (shelfManager) {
                    shelfManager.addDropUrls(drop.urls)
                }
                root.droppedFiles(drop.urls)
                drop.acceptProposedAction()
            } else if (drop.hasText) {
                if (shelfManager) {
                    shelfManager.addText(drop.text)
                }
                root.droppedText(drop.text)
                drop.acceptProposedAction()
            }

            if (appState) {
                appState.expandTab("shelf")
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: 24
        color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.12)
        border.color: Theme.accent
        border.width: 2

        // Animated dashed/pulsing glow
        SequentialAnimation on border.color {
            loops: Animation.Infinite
            ColorAnimation { to: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.4); duration: 600 }
            ColorAnimation { to: Theme.accent; duration: 600 }
        }

        Column {
            anchors.centerIn: parent
            spacing: 8

            IconImage {
                source: "qrc:/resources/icons/drop.svg"
                size: 28
                color: Theme.accent
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: "Drop files, links, or text into Shelf"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 13
                font.weight: Font.DemiBold
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Text {
                text: "Release to store safely in your Liquid Glass Hub"
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 11
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
