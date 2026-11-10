import QtQuick
import ".."

Item {
    id: root

    implicitHeight: 28
    implicitWidth: 448

    // Left: date "TUE 10 NOV" (caption, monospace, secondary color). NO wifi/bluetooth icons here.
    Row {
        anchors.left: parent.left
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        spacing: 6

        Text {
            text: (appState && appState.currentDateString !== "") ? appState.currentDateString : "TUE 10 NOV"
            color: Theme.textSecondary
            font.family: Theme.fontMono
            font.pixelSize: Theme.fontCaptionSize
            font.weight: Theme.fontCaptionWeight
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    // Center: Mascot Eyes (eyes blink, follow cursor slightly)
    MascotEyes {
        id: eyesMascot
        anchors.centerIn: parent
    }

    // Right: battery "48%" with tiny battery icon (red < 20%, green when charging) and time "11:10 AM"
    Row {
        anchors.right: parent.right
        anchors.rightMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        spacing: 10

        // Battery: tiny icon + percentage
        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            IconImage {
                source: "qrc:/resources/icons/battery.svg"
                size: 13
                color: {
                    if (appState && appState.isBatteryCharging) return Theme.accentGreen
                    if (appState && appState.isBatteryLow) return Theme.accentRed
                    return Theme.textSecondary
                }
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: (appState ? appState.batteryPercent : 100) + "%"
                color: {
                    if (appState && appState.isBatteryCharging) return Theme.accentGreen
                    if (appState && appState.isBatteryLow) return Theme.accentRed
                    return Theme.textSecondary
                }
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontCaptionSize
                font.weight: Theme.fontCaptionWeight
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Time display (tabular figures)
        Text {
            text: appState ? appState.currentTimeString : "11:10 AM"
            color: Theme.textPrimary
            font.family: Theme.fontFamily
            font.pixelSize: Theme.fontBodySize
            font.weight: Theme.fontTitleWeight
            anchors.verticalCenter: parent.verticalCenter
        }
    }
}
