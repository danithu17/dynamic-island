import QtQuick
import ".."

Item {
    id: root
    property string text: ""
    property string iconSource: ""
    property color iconColor: Theme.textPrimary
    property bool isAccent: false
    property bool isDanger: false
    property int cornerRadius: height / 2 // Circular or pill by default
    property alias isPressed: mouseArea.pressed
    signal clicked()

    implicitWidth: text !== "" ? (contentRow.implicitWidth + 24) : 32
    implicitHeight: 32

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
        id: bg
        anchors.fill: parent
        radius: root.cornerRadius
        color: {
            if (mouseArea.pressed) return Theme.cardPressed
            if (root.isDanger) {
                return Qt.rgba(Theme.accentRed.r, Theme.accentRed.g, Theme.accentRed.b, 0.20)
            }
            if (root.isAccent) {
                return Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.25)
            }
            return Theme.card
        }
        border.color: {
            if (root.isDanger) return Theme.accentRed
            if (root.isAccent) return Theme.accent
            return Theme.hairline
        }
        border.width: Theme.hairlineWidth
        antialiasing: true

        Behavior on color {
            ColorAnimation { duration: 120 }
        }
    }

    Row {
        id: contentRow
        anchors.centerIn: parent
        spacing: 6

        IconImage {
            visible: root.iconSource !== ""
            source: root.iconSource
            size: 16
            color: root.isDanger ? Theme.accentRed : (root.isAccent ? Theme.accent : root.iconColor)
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            visible: root.text !== ""
            text: root.text
            color: root.isDanger ? Theme.accentRed : Theme.textPrimary
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontBodySize
            font.weight: Theme.fontBodyWeight
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
