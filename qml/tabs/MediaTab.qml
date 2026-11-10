import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    property string pendingPowerAction: ""

    Column {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        // ── 1. Header Row ──
        Item {
            width: parent.width
            height: 22

            Text {
                text: "Music & System Controls"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 13
                font.bold: true
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
            }

            EqualizerBars {
                isPlaying: mediaManager ? mediaManager.isPlaying : false
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
            }
        }

        // ── 2. Music Player & Volume Glass Card ──
        GlassCard {
            width: parent.width
            implicitHeight: 96

            Column {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 6

                // Top: Track Info & Playback Buttons
                Row {
                    width: parent.width
                    height: 40
                    spacing: 8

                    // Album / Disc Graphic
                    Rectangle {
                        width: 38
                        height: 38
                        radius: 8
                        color: Qt.rgba(0.0, 0.0, 0.0, 0.4)
                        border.color: Theme.glassBorder
                        border.width: 1

                        IconImage {
                            anchors.centerIn: parent
                            source: "qrc:/resources/icons/media.svg"
                            size: 18
                            color: (mediaManager && mediaManager.isPlaying) ? Theme.accent : Theme.textMuted
                        }
                    }

                    // Title & Artist
                    Column {
                        width: parent.width - 38 - 110 - 16
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Text {
                            text: mediaManager ? mediaManager.title : "No Media Playing"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.bold: true
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: mediaManager ? mediaManager.artist : "Ready"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    // Playback Controls (Prev, Play/Pause, Next)
                    Row {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4

                        GlassButton {
                            iconSource: "qrc:/resources/icons/prev.svg"
                            cornerRadius: 14
                            implicitWidth: 28
                            implicitHeight: 28
                            onClicked: if (mediaManager) mediaManager.skipPrevious()
                        }

                        GlassButton {
                            iconSource: (mediaManager && mediaManager.isPlaying) ? "qrc:/resources/icons/pause.svg" : "qrc:/resources/icons/play.svg"
                            isAccent: true
                            cornerRadius: 15
                            implicitWidth: 32
                            implicitHeight: 32
                            onClicked: if (mediaManager) mediaManager.togglePlayPause()
                        }

                        GlassButton {
                            iconSource: "qrc:/resources/icons/next.svg"
                            cornerRadius: 14
                            implicitWidth: 28
                            implicitHeight: 28
                            onClicked: if (mediaManager) mediaManager.skipNext()
                        }
                    }
                }

                // Bottom: Volume Slider & Mute Toggle
                Row {
                    width: parent.width
                    height: 26
                    spacing: 8

                    // Mute / Speaker Toggle
                    GlassButton {
                        iconSource: (systemControlManager && systemControlManager.isMuted)
                                    ? "qrc:/resources/icons/volume-mute.svg"
                                    : "qrc:/resources/icons/volume.svg"
                        cornerRadius: 13
                        implicitWidth: 26
                        implicitHeight: 26
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: if (systemControlManager) systemControlManager.toggleMute()
                    }

                    // Volume Slider
                    GlassSlider {
                        width: parent.width - 26 - 44 - 16
                        anchors.verticalCenter: parent.verticalCenter
                        from: 0
                        to: 100
                        value: systemControlManager ? systemControlManager.volume : 50
                        onValueModified: function(newVal) {
                            if (systemControlManager) systemControlManager.setVolume(newVal)
                        }
                    }

                    // Volume Text
                    Text {
                        text: (systemControlManager && systemControlManager.isMuted)
                              ? "Mute"
                              : ((systemControlManager ? systemControlManager.volume : 50) + "%")
                        color: Theme.textSecondary
                        font.family: Theme.fontMono
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        width: 36
                        horizontalAlignment: Text.AlignRight
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }
        }

        // ── 3. Brightness & Touchpad Row ──
        Row {
            width: parent.width
            height: 40
            spacing: 8

            // Brightness Card
            GlassCard {
                width: parent.width - 150 - 8
                height: 40

                Row {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 6

                    IconImage {
                        source: "qrc:/resources/icons/sun.svg"
                        size: 14
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    GlassSlider {
                        width: parent.width - 14 - 36 - 12
                        anchors.verticalCenter: parent.verticalCenter
                        from: 0
                        to: 100
                        value: systemControlManager ? systemControlManager.brightness : 75
                        onValueModified: function(newVal) {
                            if (systemControlManager) systemControlManager.setBrightness(newVal)
                        }
                    }

                    Text {
                        text: (systemControlManager ? systemControlManager.brightness : 75) + "%"
                        color: Theme.textSecondary
                        font.family: Theme.fontMono
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        width: 30
                        horizontalAlignment: Text.AlignRight
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            // Touchpad Toggle Button
            GlassButton {
                width: 150
                height: 40
                cornerRadius: 12
                iconSource: "qrc:/resources/icons/touchpad.svg"
                text: (systemControlManager && systemControlManager.isTouchpadEnabled) ? "Touchpad: ON" : "Touchpad: OFF"
                isAccent: (systemControlManager && systemControlManager.isTouchpadEnabled)
                onClicked: if (systemControlManager) systemControlManager.toggleTouchpad()
            }
        }

        // ── 4. System Power Controls Row ──
        Item {
            width: parent.width
            height: 38

            // Normal Power Buttons
            Row {
                anchors.fill: parent
                spacing: 6
                visible: root.pendingPowerAction === ""

                GlassButton {
                    width: (parent.width - 18) / 4
                    height: 36
                    cornerRadius: 10
                    iconSource: "qrc:/resources/icons/sleep.svg"
                    text: "Sleep"
                    onClicked: if (systemControlManager) systemControlManager.sleepPC()
                }

                GlassButton {
                    width: (parent.width - 18) / 4
                    height: 36
                    cornerRadius: 10
                    iconSource: "qrc:/resources/icons/lock.svg"
                    text: "Lock"
                    onClicked: if (systemControlManager) systemControlManager.lockScreen()
                }

                GlassButton {
                    width: (parent.width - 18) / 4
                    height: 36
                    cornerRadius: 10
                    iconSource: "qrc:/resources/icons/restart.svg"
                    text: "Restart"
                    onClicked: root.pendingPowerAction = "restart"
                }

                GlassButton {
                    width: (parent.width - 18) / 4
                    height: 36
                    cornerRadius: 10
                    iconSource: "qrc:/resources/icons/power.svg"
                    text: "Shut Down"
                    onClicked: root.pendingPowerAction = "shutdown"
                }
            }

            // Power Confirmation Banner (Prevents accidental clicks)
            Rectangle {
                anchors.fill: parent
                radius: 10
                color: Qt.rgba(0.2, 0.05, 0.05, 0.90)
                border.color: Theme.danger
                border.width: 1
                visible: root.pendingPowerAction !== ""

                Row {
                    anchors.centerIn: parent
                    spacing: 10

                    Text {
                        text: (root.pendingPowerAction === "restart") ? "Restart PC now?" : "Shut down PC now?"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    GlassButton {
                        height: 26
                        implicitWidth: 60
                        cornerRadius: 8
                        isAccent: true
                        text: "Yes"
                        onClicked: {
                            if (root.pendingPowerAction === "restart") {
                                if (systemControlManager) systemControlManager.restartPC()
                            } else if (root.pendingPowerAction === "shutdown") {
                                if (systemControlManager) systemControlManager.shutdownPC()
                            }
                            root.pendingPowerAction = ""
                        }
                    }

                    GlassButton {
                        height: 26
                        implicitWidth: 60
                        cornerRadius: 8
                        text: "Cancel"
                        onClicked: root.pendingPowerAction = ""
                    }
                }
            }
        }
    }
}
