import QtQuick
import ".."

Item {
    id: root
    property string text: ""
    property string iconSource: ""
    property color iconColor: Theme.textPrimary
    property bool isAccent: false
    property bool isDanger: false
    property int cornerRadius: 8
    signal clicked()

    implicitWidth: contentRow.implicitWidth + 24
    implicitHeight: 32

    scale: mouseArea.pressed ? 0.96 : (mouseArea.containsMouse ? 1.02 : 1.0)
    Behavior on scale {
        NumberAnimation { duration: 120; easing.type: Easing.OutCubic }
    }

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: root.cornerRadius
        color: {
            if (root.isDanger) {
                return mouseArea.containsMouse ? Qt.rgba(1.0, 0.27, 0.23, 0.35) : Qt.rgba(1.0, 0.27, 0.23, 0.2)
            }
            if (root.isAccent) {
                return mouseArea.containsMouse ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.4) : Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.25)
            }
            return mouseArea.containsMouse ? Theme.glassSurfaceHover : Theme.glassSurface
        }
        border.color: {
            if (root.isDanger) return Qt.rgba(1.0, 0.27, 0.23, 0.5)
            if (root.isAccent) return Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.6)
            return mouseArea.containsMouse ? Theme.glassBorderHighlight : Theme.glassBorder
        }
        border.width: 1

        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    Row {
        id: contentRow
        anchors.centerIn: parent
        spacing: 6

        IconImage {
            visible: root.iconSource !== ""
            source: root.iconSource
            size: 14
            color: root.isDanger ? Theme.danger : (root.isAccent ? Theme.accent : root.iconColor)
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            visible: root.text !== ""
            text: root.text
            color: root.isDanger ? Theme.danger : (root.isAccent ? Theme.textPrimary : Theme.textPrimary)
            font.family: Theme.fontFamily
            font.pixelSize: 12
            font.weight: Font.Medium
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
