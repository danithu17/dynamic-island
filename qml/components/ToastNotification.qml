import QtQuick
import ".."

Item {
    id: root
    property string message: appState ? appState.toastMessage : ""
    property string type: appState ? appState.toastType : "info"
    property bool visibleState: appState ? appState.hasToast : false

    anchors.bottom: parent.bottom
    anchors.bottomMargin: 8
    anchors.horizontalCenter: parent.horizontalCenter
    width: Math.min(contentRow.implicitWidth + 24, parent.width - 24)
    height: 30
    opacity: visibleState ? 1.0 : 0.0
    scale: visibleState ? 1.0 : 0.9

    Behavior on opacity {
        NumberAnimation { duration: Theme.animDurationFast }
    }
    Behavior on scale {
        NumberAnimation { duration: Theme.animDurationFast; easing.type: Theme.animEasingSpring }
    }

    Rectangle {
        anchors.fill: parent
        radius: 15
        color: Qt.rgba(0.1, 0.11, 0.15, 0.95)
        border.color: {
            if (root.type === "error" || root.type === "danger") return Theme.danger
            if (root.type === "success") return Theme.success
            if (root.type === "warning") return Theme.warning
            return Theme.accent
        }
        border.width: 1

        Row {
            id: contentRow
            anchors.centerIn: parent
            spacing: 8

            Rectangle {
                width: 6
                height: 6
                radius: 3
                color: {
                    if (root.type === "error") return Theme.danger
                    if (root.type === "success") return Theme.success
                    if (root.type === "warning") return Theme.warning
                    return Theme.accent
                }
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: root.message
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 11
                font.weight: Font.Medium
                elide: Text.ElideRight
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
