import QtQuick
import ".."

Item {
    id: root

    property real from: 0
    property real to: 100
    property real value: 50
    property color activeColor: Theme.accent
    signal valueModified(real newValue)

    implicitHeight: 20
    implicitWidth: 160

    Rectangle {
        id: track
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        height: 5
        radius: 2.5
        color: Qt.rgba(1.0, 1.0, 1.0, 0.14)

        Rectangle {
            id: progress
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: Math.max(0, Math.min(track.width, ((root.value - root.from) / Math.max(1, root.to - root.from)) * track.width))
            radius: 2.5
            color: root.activeColor
        }
    }

    Rectangle {
        id: thumb
        x: Math.max(0, Math.min(root.width - width, progress.width - width / 2))
        anchors.verticalCenter: parent.verticalCenter
        width: 13
        height: 13
        radius: 6.5
        color: "#FFFFFF"
        border.color: Qt.rgba(0.0, 0.0, 0.0, 0.25)
        border.width: 1

        Behavior on x {
            enabled: !dragArea.pressed
            NumberAnimation { duration: 60 }
        }
    }

    MouseArea {
        id: dragArea
        anchors.fill: parent
        anchors.topMargin: -5
        anchors.bottomMargin: -5
        cursorShape: Qt.PointingHandCursor

        function updateFromMouse(mouseX) {
            var ratio = Math.max(0.0, Math.min(1.0, mouseX / root.width))
            var newVal = Math.round(root.from + ratio * (root.to - root.from))
            root.valueModified(newVal)
        }

        onPositionChanged: function(mouse) {
            if (pressed) updateFromMouse(mouse.x)
        }
        onPressed: function(mouse) {
            updateFromMouse(mouse.x)
        }
    }
}
