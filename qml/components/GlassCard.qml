import QtQuick
import ".."

Item {
    id: root
    property int cornerRadius: Theme.cardRadius // 20px
    property bool isHoverable: true
    property bool isSelected: false
    property alias isPressed: mouseArea.pressed
    property alias containsMouse: mouseArea.containsMouse
    property alias contentItem: container.children
    default property alias data: container.data

    signal clicked()

    implicitWidth: 200
    implicitHeight: 60

    scale: mouseArea.pressed ? 0.96 : 1.0

    Behavior on scale {
        SpringAnimation {
            spring: 4.5
            damping: 0.65
            mass: 1.0
            epsilon: 0.001
        }
    }

    Rectangle {
        id: cardBg
        anchors.fill: parent
        radius: root.cornerRadius
        color: {
            if (mouseArea.pressed) return Theme.cardPressed
            if (root.isSelected) return Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.20)
            return Theme.card
        }
        border.color: root.isSelected ? Theme.accent : Theme.hairline
        border.width: Theme.hairlineWidth
        antialiasing: true

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
        Behavior on border.color {
            ColorAnimation { duration: 120 }
        }
    }

    Item {
        id: container
        anchors.fill: parent
        anchors.margins: Theme.gap
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: root.isHoverable
        cursorShape: root.isHoverable ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.clicked()
    }
}
