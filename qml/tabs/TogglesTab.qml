import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 448
    implicitHeight: 220

    readonly property bool isWifiOn: appState ? appState.isWifiConnected : true
    readonly property bool isBtOn: appState ? appState.isBluetoothConnected : true
    readonly property bool isMuteOn: systemControlManager ? systemControlManager.isMuted : false
    readonly property bool isMicMuteOn: systemControlManager ? systemControlManager.isMicMuted : false
    readonly property bool isKeepAwakeOn: systemControlManager ? systemControlManager.isKeepAwake : false
    readonly property bool isFocusOn: appState ? appState.isFocusModeActive : false
    readonly property bool isDarkOn: systemControlManager ? systemControlManager.isDarkMode : true

    Column {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 12

        // ─────────────────────────────────────────────────────
        // 1. Grid of Round 56px Toggle Buttons with Captions
        // ─────────────────────────────────────────────────────
        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 16

            // Toggle 1: Wi-Fi
            Column {
                spacing: 4
                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: root.isWifiOn ? "#FFFFFF" : Theme.card
                    border.color: root.isWifiOn ? "transparent" : Theme.hairline
                    border.width: Theme.hairlineWidth
                    scale: wifiMouse.pressed ? 0.94 : 1.0
                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                    Behavior on color { ColorAnimation { duration: 150 } }

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/wifi.svg"
                        size: 24
                        color: root.isWifiOn ? "#000000" : "#FFFFFF"
                    }

                    MouseArea {
                        id: wifiMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (systemControlManager) systemControlManager.toggleWifi()
                            if (alertManager) alertManager.postAlert("toggle", "Wi-Fi", root.isWifiOn ? "Disconnected" : "Connected", "qrc:/resources/icons/wifi.svg", Theme.accentBlue, "", "", "", "", 1500)
                        }
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Wi-Fi"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontCaptionSize
                }
            }

            // Toggle 2: Bluetooth
            Column {
                spacing: 4
                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: root.isBtOn ? "#FFFFFF" : Theme.card
                    border.color: root.isBtOn ? "transparent" : Theme.hairline
                    border.width: Theme.hairlineWidth
                    scale: btMouse.pressed ? 0.94 : 1.0
                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                    Behavior on color { ColorAnimation { duration: 150 } }

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/bluetooth.svg"
                        size: 24
                        color: root.isBtOn ? "#000000" : "#FFFFFF"
                    }

                    MouseArea {
                        id: btMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (systemControlManager) systemControlManager.toggleBluetooth()
                            if (alertManager) alertManager.postAlert("toggle", "Bluetooth", root.isBtOn ? "Off" : "On", "qrc:/resources/icons/bluetooth.svg", Theme.accentBlue, "", "", "", "", 1500)
                        }
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Bluetooth"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontCaptionSize
                }
            }

            // Toggle 3: Mute
            Column {
                spacing: 4
                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: root.isMuteOn ? "#FFFFFF" : Theme.card
                    border.color: root.isMuteOn ? "transparent" : Theme.hairline
                    border.width: Theme.hairlineWidth
                    scale: muteMouse.pressed ? 0.94 : 1.0
                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                    Behavior on color { ColorAnimation { duration: 150 } }

                    IconImage {
                        anchors.centerIn: parent
                        source: root.isMuteOn ? "qrc:/resources/icons/volume-mute.svg" : "qrc:/resources/icons/volume.svg"
                        size: 24
                        color: root.isMuteOn ? "#000000" : "#FFFFFF"
                    }

                    MouseArea {
                        id: muteMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (systemControlManager) systemControlManager.toggleMute()
                        }
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Mute"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontCaptionSize
                }
            }

            // Toggle 4: Mic Mute
            Column {
                spacing: 4
                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: root.isMicMuteOn ? "#FFFFFF" : Theme.card
                    border.color: root.isMicMuteOn ? "transparent" : Theme.hairline
                    border.width: Theme.hairlineWidth
                    scale: micMouse.pressed ? 0.94 : 1.0
                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                    Behavior on color { ColorAnimation { duration: 150 } }

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/volume-mute.svg"
                        size: 24
                        color: root.isMicMuteOn ? "#000000" : "#FFFFFF"
                    }

                    MouseArea {
                        id: micMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (systemControlManager) systemControlManager.toggleMicMute()
                        }
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Mic Mute"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontCaptionSize
                }
            }

            // Toggle 5: Keep Awake
            Column {
                spacing: 4
                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: root.isKeepAwakeOn ? "#FFFFFF" : Theme.card
                    border.color: root.isKeepAwakeOn ? "transparent" : Theme.hairline
                    border.width: Theme.hairlineWidth
                    scale: awakeMouse.pressed ? 0.94 : 1.0
                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                    Behavior on color { ColorAnimation { duration: 150 } }

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/sun.svg"
                        size: 24
                        color: root.isKeepAwakeOn ? "#000000" : "#FFFFFF"
                    }

                    MouseArea {
                        id: awakeMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (systemControlManager) systemControlManager.toggleKeepAwake()
                        }
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Awake"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontCaptionSize
                }
            }

            // Toggle 6: Focus Mode
            Column {
                spacing: 4
                Rectangle {
                    width: 56
                    height: 56
                    radius: 28
                    color: root.isFocusOn ? "#FFFFFF" : Theme.card
                    border.color: root.isFocusOn ? "transparent" : Theme.hairline
                    border.width: Theme.hairlineWidth
                    scale: focusMouse.pressed ? 0.94 : 1.0
                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                    Behavior on color { ColorAnimation { duration: 150 } }

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/moon.svg"
                        size: 24
                        color: root.isFocusOn ? "#000000" : "#FFFFFF"
                    }

                    MouseArea {
                        id: focusMouse
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (appState) appState.toggleFocusMode()
                        }
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Focus"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontCaptionSize
                }
            }
        }

        // ─────────────────────────────────────────────────────
        // 2. Sliders (28px tall pill sliders with white fill and icon inside)
        // ─────────────────────────────────────────────────────
        Column {
            width: parent.width - 24
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            // Volume Slider
            Rectangle {
                width: parent.width
                height: 28
                radius: 14
                color: Theme.card
                border.color: Theme.hairline
                border.width: Theme.hairlineWidth
                clip: true

                readonly property int currentVol: systemControlManager ? systemControlManager.volume : 50

                // Fill
                Rectangle {
                    width: parent.width * (parent.currentVol / 100.0)
                    height: parent.height
                    radius: parent.radius
                    color: "#FFFFFF"
                }

                // Volume Icon inside
                IconImage {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/resources/icons/volume.svg"
                    size: 16
                    color: (parent.currentVol > 12) ? "#000000" : "#FFFFFF"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    preventStealing: true
                    onPositionChanged: function(mouse) {
                        var pct = Math.max(0, Math.min(100, Math.round(mouse.x / width * 100)))
                        if (systemControlManager) systemControlManager.setVolume(pct)
                    }
                    onClicked: function(mouse) {
                        var pct = Math.max(0, Math.min(100, Math.round(mouse.x / width * 100)))
                        if (systemControlManager) systemControlManager.setVolume(pct)
                    }
                }
            }

            // Brightness Slider (if supported)
            Rectangle {
                width: parent.width
                height: 28
                radius: 14
                color: Theme.card
                border.color: Theme.hairline
                border.width: Theme.hairlineWidth
                clip: true
                visible: systemControlManager ? systemControlManager.hasBrightness : true

                readonly property int currentBright: systemControlManager ? systemControlManager.brightness : 70

                Rectangle {
                    width: parent.width * (parent.currentBright / 100.0)
                    height: parent.height
                    radius: parent.radius
                    color: "#FFFFFF"
                }

                IconImage {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    source: "qrc:/resources/icons/sun.svg"
                    size: 16
                    color: (parent.currentBright > 12) ? "#000000" : "#FFFFFF"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    preventStealing: true
                    onPositionChanged: function(mouse) {
                        var pct = Math.max(0, Math.min(100, Math.round(mouse.x / width * 100)))
                        if (systemControlManager) systemControlManager.setBrightness(pct)
                    }
                    onClicked: function(mouse) {
                        var pct = Math.max(0, Math.min(100, Math.round(mouse.x / width * 100)))
                        if (systemControlManager) systemControlManager.setBrightness(pct)
                    }
                }
            }
        }
    }
}
