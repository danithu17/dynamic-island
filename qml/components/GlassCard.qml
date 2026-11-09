import QtQuick
import ".."

Item {
    id: root
    property int cornerRadius: 10
    property bool isHoverable: true
    property bool isSelected: false
    property alias contentItem: container.children
    default property alias data: container.data

    implicitWidth: 200
    implicitHeight: 60

    Rectangle {
        id: cardBg
        anchors.fill: parent
        radius: root.cornerRadius
        color: {
            if (root.isSelected) return Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.25)
            return (mouseArea.containsMouse && root.isHoverable) ? Theme.glassSurfaceHover : Theme.glassSurface
        }
        border.color: {
            if (root.isSelected) return Theme.accent
            return (mouseArea.containsMouse && root.isHoverable) ? Theme.glassBorderHighlight : Theme.glassBorder
        }
        border.width: 1

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }

        // Inner specular highlight
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Qt.rgba(1.0, 1.0, 1.0, 0.12)
        }
    }

    Item {
        id: container
        anchors.fill: parent
        anchors.margins: 10
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.isHoverable
        cursorShape: root.isHoverable ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}
