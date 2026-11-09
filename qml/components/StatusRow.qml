import QtQuick
import QtQuick.Controls
import ".."

Item {
    id: root

    implicitHeight: 28
    implicitWidth: 456

    Row {
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        IconImage {
            source: "qrc:/resources/icons/wifi.svg"
            size: 13
            color: (appState && appState.isWifiConnected) ? "#FFFFFF" : Theme.textMuted
            anchors.verticalCenter: parent.verticalCenter
        }

        IconImage {
            source: "qrc:/resources/icons/bluetooth.svg"
            size: 13
            color: (appState && appState.isBluetoothConnected) ? "#FFFFFF" : Theme.textMuted
            opacity: 0.8
            anchors.verticalCenter: parent.verticalCenter
        }

        Text {
            text: appState ? appState.currentDateString : "MON 09 NOV"
            color: Theme.textSecondary
            font.family: Theme.fontMono
            font.pixelSize: 10
            font.weight: Font.DemiBold
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    // Center Mascot Eyes
    MascotEyes {
        anchors.centerIn: parent
    }

    // Right: Battery pill, Time, Pin & Close buttons
    Row {
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        spacing: 8

        // Battery pill (percentage & status, turns red below 20%)
        Rectangle {
            id: batteryPill
            height: 18
            radius: 9
            width: batteryRow.implicitWidth + 12
            color: (appState && appState.isBatteryLow) ? Qt.rgba(1.0, 0.27, 0.23, 0.25) : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.color: (appState && appState.isBatteryLow) ? Theme.danger : Qt.rgba(1.0, 1.0, 1.0, 0.15)
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter

            Row {
                id: batteryRow
                anchors.centerIn: parent
                spacing: 4

                IconImage {
                    source: "qrc:/resources/icons/battery.svg"
                    size: 11
                    color: (appState && appState.isBatteryLow) ? Theme.danger : ((appState && appState.isBatteryCharging) ? Theme.success : "#FFFFFF")
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: (appState ? appState.batteryPercent : 100) + "%"
                    color: (appState && appState.isBatteryLow) ? Theme.danger : "#FFFFFF"
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.weight: Font.Medium
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Time display
        Text {
            text: appState ? appState.currentTimeString : "6:18 PM"
            color: "#FFFFFF"
            font.family: Theme.fontFamily
            font.pixelSize: 12
            font.weight: Font.Bold
            anchors.verticalCenter: parent.verticalCenter
        }

        // Pin Toggle Button
        Rectangle {
            width: 22
            height: 22
            radius: 11
            color: (appState && appState.isPinned) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.4) : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.color: (appState && appState.isPinned) ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.12)
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter

            IconImage {
                anchors.centerIn: parent
                source: "qrc:/resources/icons/pin.svg"
                size: 11
                color: (appState && appState.isPinned) ? Theme.accent : Theme.textSecondary
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (appState) appState.setIsPinned(!appState.isPinned)
                }
            }
        }

        // Close 'X' Button
        Rectangle {
            width: 22
            height: 22
            radius: 11
            color: Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.color: Qt.rgba(1.0, 1.0, 1.0, 0.12)
            border.width: 1
            anchors.verticalCenter: parent.verticalCenter

            IconImage {
                anchors.centerIn: parent
                source: "qrc:/resources/icons/close.svg"
                size: 10
                color: "#FFFFFF"
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    if (appState) appState.collapseToIdle()
                }
            }
        }
    }
}
