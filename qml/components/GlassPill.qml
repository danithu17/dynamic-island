import QtQuick
import ".."

Item {
    id: root
    property int cornerRadius: 16
    property color backgroundColor: Theme.glassBackground
    property color borderColor: Theme.glassBorder
    property bool showSpecular: true
    property bool activeGlow: false

    // Outer glow for drop zone or active state
    Rectangle {
        id: glowRect
        anchors.fill: parent
        anchors.margins: -3
        radius: root.cornerRadius + 3
        color: "transparent"
        border.color: Theme.accentGlow
        border.width: 2
        opacity: root.activeGlow ? 1.0 : 0.0
        visible: opacity > 0

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationNormal }
        }
    }

    // Main Liquid Glass Body
    Rectangle {
        id: pillBody
        anchors.fill: parent
        radius: root.cornerRadius
        color: root.backgroundColor
        border.color: root.borderColor
        border.width: 1
        antialiasing: true

        Behavior on radius {
            SmoothedAnimation { duration: Theme.animDurationNormal; easing.type: Easing.OutCubic }
        }

        // Top specular highlight gradient (Liquid Glass sheen)
        Rectangle {
            id: specularSheen
            visible: root.showSpecular
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 1
            anchors.leftMargin: 2
            anchors.rightMargin: 2
            height: Math.min(parent.height * 0.45, 30)
            radius: Math.max(0, root.cornerRadius - 1)
            antialiasing: true

            gradient: Gradient {
                GradientStop { position: 0.0; color: Theme.glassSpecular }
                GradientStop { position: 0.3; color: Qt.rgba(1.0, 1.0, 1.0, 0.04) }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }

        // Inner bottom ambient light reflection
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottomMargin: 1
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            height: 1
            color: Qt.rgba(1.0, 1.0, 1.0, 0.08)
        }
    }
}
