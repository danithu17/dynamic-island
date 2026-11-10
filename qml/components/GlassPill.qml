import QtQuick
import ".."

Item {
    id: root
    property int cornerRadius: 17
    property int bottomRadius: cornerRadius
    property color backgroundColor: Theme.shell
    property color borderColor: "transparent"
    property bool showSpecular: false
    property bool activeGlow: false

    ShellBackground {
        anchors.fill: parent
        cornerRadius: root.cornerRadius
        bottomRadius: root.bottomRadius
    }
}
