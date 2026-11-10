import QtQuick
import QtQuick.Controls
import ".."

Item {
    id: root

    implicitWidth: 360
    implicitHeight: 76

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onEntered: if (alertManager) alertManager.pauseTimer()
        onExited: if (alertManager) alertManager.resumeTimer()
        onClicked: {
            if (alertManager && alertManager.targetTab !== "") {
                if (appState) appState.expandTab(alertManager.targetTab)
            }
        }
    }

    Row {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // Tinted Icon circle
        Rectangle {
            width: 40
            height: 40
            radius: 20
            color: {
                var c = (alertManager && alertManager.currentIconColor !== "") ? alertManager.currentIconColor : Theme.accentBlue
                return Qt.rgba(c.r, c.g, c.b, 0.20)
            }
            anchors.verticalCenter: parent.verticalCenter

            IconImage {
                anchors.centerIn: parent
                source: alertManager ? alertManager.currentIcon : ""
                size: 20
                color: (alertManager && alertManager.currentIconColor !== "") ? alertManager.currentIconColor : Theme.accentBlue
            }
        }

        // Title and Subtitle
        Column {
            width: parent.width - 52 - actionsRow.implicitWidth - 8
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Text {
                width: parent.width
                text: alertManager ? alertManager.currentTitle : ""
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontTitleSize // 15px
                font.weight: Theme.fontTitleWeight // 600
                elide: Text.ElideRight
                maximumLineCount: 1
            }

            Text {
                width: parent.width
                text: alertManager ? alertManager.currentSubtitle : ""
                color: Theme.textSecondary // 13px secondary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontBodySize
                font.weight: Theme.fontBodyWeight
                elide: Text.ElideRight
                maximumLineCount: 1
            }
        }

        // Action Buttons (Pill, 28px)
        Row {
            id: actionsRow
            anchors.verticalCenter: parent.verticalCenter
            spacing: 6

            // Action 1
            Rectangle {
                visible: (alertManager && alertManager.action1Text !== "")
                height: 28
                width: a1Text.implicitWidth + 18
                radius: 14
                color: a1Mouse.pressed ? Theme.cardPressed : Qt.rgba(1.0, 1.0, 1.0, 0.12)
                border.color: Theme.hairline
                border.width: Theme.hairlineWidth

                scale: a1Mouse.pressed ? 0.94 : 1.0
                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                Text {
                    id: a1Text
                    anchors.centerIn: parent
                    text: alertManager ? alertManager.action1Text : ""
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    font.weight: Font.Medium
                }

                MouseArea {
                    id: a1Mouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (alertManager) alertManager.triggerAction(alertManager.action1Id)
                    }
                }
            }

            // Action 2
            Rectangle {
                visible: (alertManager && alertManager.action2Text !== "")
                height: 28
                width: a2Text.implicitWidth + 18
                radius: 14
                color: a2Mouse.pressed ? Theme.cardPressed : Qt.rgba(Theme.accentBlue.r, Theme.accentBlue.g, Theme.accentBlue.b, 0.25)
                border.color: Theme.accentBlue
                border.width: Theme.hairlineWidth

                scale: a2Mouse.pressed ? 0.94 : 1.0
                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                Text {
                    id: a2Text
                    anchors.centerIn: parent
                    text: alertManager ? alertManager.action2Text : ""
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    font.weight: Font.Medium
                }

                MouseArea {
                    id: a2Mouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (alertManager) alertManager.triggerAction(alertManager.action2Id)
                    }
                }
            }
        }
    }
}
