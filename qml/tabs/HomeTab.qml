import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 448
    implicitHeight: 180

    Column {
        anchors.fill: parent
        spacing: Theme.gap // 8px gap

        // ─────────────────────────────────────────────────────
        // Row 1 (Height 116): [Now Playing 2/3 width] [Focus timer 1/3 width]
        // ─────────────────────────────────────────────────────
        Row {
            width: parent.width
            height: 116
            spacing: Theme.gap

            // 1. Now Playing Card (2/3 width)
            GlassCard {
                id: mediaCard
                width: Math.floor((parent.width - Theme.gap) * 2 / 3)
                height: 116
                cornerRadius: Theme.cardRadius // 20px

                readonly property bool hasMedia: (mediaManager && mediaManager.hasTrack && mediaManager.title !== "" && mediaManager.title !== "No media playing")

                // Media Active State
                Item {
                    anchors.fill: parent
                    visible: mediaCard.hasMedia

                    Row {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        // 64px rounded album art on the left
                        Rectangle {
                            width: 64
                            height: 64
                            radius: 12
                            color: Qt.rgba(Theme.accentPink.r, Theme.accentPink.g, Theme.accentPink.b, 0.20)
                            anchors.verticalCenter: parent.verticalCenter
                            clip: true

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/media.svg"
                                size: 28
                                color: Theme.accentPink
                            }
                        }

                        // Title, artist, progress bar & controls
                        Column {
                            width: parent.width - 76
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4

                            Text {
                                width: parent.width
                                text: mediaManager ? mediaManager.title : "Nothing playing"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontTitleSize // 15px
                                font.weight: Theme.fontTitleWeight // 600
                                elide: Text.ElideRight
                                maximumLineCount: 1
                            }

                            Text {
                                width: parent.width
                                text: mediaManager ? mediaManager.artist : ""
                                color: Theme.textSecondary // 13px secondary
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontBodySize
                                font.weight: Theme.fontBodyWeight
                                elide: Text.ElideRight
                                maximumLineCount: 1
                            }

                            // Thin progress bar
                            Rectangle {
                                width: parent.width
                                height: 3
                                radius: 1.5
                                color: Qt.rgba(1.0, 1.0, 1.0, 0.12)

                                Rectangle {
                                    width: parent.width * 0.45
                                    height: parent.height
                                    radius: parent.radius
                                    color: Theme.accentPink
                                }
                            }

                            // Previous / Play / Next circular 32px buttons
                            Row {
                                spacing: 8
                                anchors.horizontalCenter: parent.horizontalCenter

                                // Prev
                                Rectangle {
                                    width: 32
                                    height: 32
                                    radius: 16
                                    color: prevMouse.pressed ? Theme.cardPressed : Theme.card
                                    border.color: Theme.hairline
                                    border.width: Theme.hairlineWidth

                                    scale: prevMouse.pressed ? 0.94 : 1.0
                                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                                    IconImage {
                                        anchors.centerIn: parent
                                        source: "qrc:/resources/icons/prev.svg"
                                        size: 14
                                        color: "#FFFFFF"
                                    }
                                    MouseArea {
                                        id: prevMouse
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: if (mediaManager) mediaManager.skipPrevious()
                                    }
                                }

                                // Play / Pause
                                Rectangle {
                                    width: 32
                                    height: 32
                                    radius: 16
                                    color: playMouse.pressed ? Theme.cardPressed : Theme.card
                                    border.color: Theme.hairline
                                    border.width: Theme.hairlineWidth

                                    scale: playMouse.pressed ? 0.94 : 1.0
                                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                                    IconImage {
                                        anchors.centerIn: parent
                                        source: (mediaManager && mediaManager.isPlaying) ? "qrc:/resources/icons/pause.svg" : "qrc:/resources/icons/play.svg"
                                        size: 14
                                        color: Theme.accentPink
                                    }
                                    MouseArea {
                                        id: playMouse
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: if (mediaManager) mediaManager.togglePlayPause()
                                    }
                                }

                                // Next
                                Rectangle {
                                    width: 32
                                    height: 32
                                    radius: 16
                                    color: nextMouse.pressed ? Theme.cardPressed : Theme.card
                                    border.color: Theme.hairline
                                    border.width: Theme.hairlineWidth

                                    scale: nextMouse.pressed ? 0.94 : 1.0
                                    Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                                    IconImage {
                                        anchors.centerIn: parent
                                        source: "qrc:/resources/icons/next.svg"
                                        size: 14
                                        color: "#FFFFFF"
                                    }
                                    MouseArea {
                                        id: nextMouse
                                        anchors.fill: parent
                                        cursorShape: Qt.PointingHandCursor
                                        onClicked: if (mediaManager) mediaManager.skipNext()
                                    }
                                }
                            }
                        }
                    }
                }

                // Media Empty State: Personalized Greeting
                Item {
                    anchors.fill: parent
                    visible: !mediaCard.hasMedia

                    Column {
                        anchors.centerIn: parent
                        spacing: 6
                        width: parent.width - 24

                        IconImage {
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "qrc:/resources/icons/sparkles.svg"
                            size: 20
                            color: Theme.accent
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: (settings && settings.greeting) ? settings.greeting : "Welcome to Aura Island"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.weight: Theme.fontTitleWeight
                            elide: Text.ElideRight
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "No media playing"
                            color: Theme.textTertiary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
                            font.weight: Theme.fontBodyWeight
                        }
                    }
                }
            }

            // 2. Focus Timer Card (1/3 width)
            GlassCard {
                id: timerCard
                width: (parent.width - Theme.gap) - mediaCard.width
                height: 116
                cornerRadius: Theme.cardRadius // 20px

                readonly property bool isRunning: devManager && devManager.pomodoro ? devManager.pomodoro.isRunning : false
                readonly property string timeDisplay: devManager && devManager.pomodoro ? devManager.pomodoro.timeDisplay : "25:00"

                Column {
                    anchors.centerIn: parent
                    spacing: 4

                    // Circular Progress Ring with "24:51" inside
                    Rectangle {
                        width: 76
                        height: 76
                        radius: 38
                        color: "transparent"
                        border.color: timerCard.isRunning ? Theme.accentOrange : Qt.rgba(Theme.accentOrange.r, Theme.accentOrange.g, Theme.accentOrange.b, 0.35)
                        border.width: 3
                        anchors.horizontalCenter: parent.horizontalCenter

                        Column {
                            anchors.centerIn: parent
                            spacing: 0

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: timerCard.timeDisplay
                                color: Theme.textPrimary
                                font.family: Theme.fontMono
                                font.pixelSize: 18 // Big number inside ring
                                font.weight: Theme.fontBigNumberWeight
                            }

                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: "Focus"
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: Theme.fontCaptionSize
                                font.weight: Theme.fontCaptionWeight
                            }
                        }
                    }
                }

                onClicked: {
                    if (devManager && devManager.pomodoro) {
                        if (devManager.pomodoro.isRunning) {
                            devManager.pomodoro.pause()
                        } else {
                            devManager.pomodoro.start()
                        }
                    }
                }
            }
        }

        // ─────────────────────────────────────────────────────
        // Row 2 (Height 56): Four Equal Chips (Tray, Clips, Mail, Next)
        // ─────────────────────────────────────────────────────
        Row {
            width: parent.width
            height: 56
            spacing: Theme.gap

            readonly property int chipWidth: Math.floor((parent.width - Theme.gap * 3) / 4)

            // 1. Tray Chip
            Rectangle {
                width: parent.chipWidth
                height: 52
                radius: Theme.chipRadius // 14px
                color: trayMouse.pressed ? Theme.cardPressed : Theme.card
                border.color: Theme.hairline
                border.width: Theme.hairlineWidth
                antialiasing: true

                scale: trayMouse.pressed ? 0.94 : 1.0
                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                readonly property int count: (shelfManager ? shelfManager.itemCount : 0)

                Row {
                    anchors.centerIn: parent
                    spacing: 8
                    visible: parent.count > 0

                    IconImage {
                        source: "qrc:/resources/icons/shelf.svg"
                        size: 18
                        color: Theme.accentBlue
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 1

                        Text {
                            text: parent.parent.parent.count.toString()
                            color: Theme.textPrimary
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.fontTitleSize
                            font.weight: Theme.fontTitleWeight
                        }

                        Text {
                            text: "Tray"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
                            font.weight: Theme.fontCaptionWeight
                        }
                    }
                }

                // Dimmed icon when empty
                IconImage {
                    visible: parent.count === 0
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/shelf.svg"
                    size: 20
                    color: Theme.textTertiary
                }

                MouseArea {
                    id: trayMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (appState) appState.setCurrentTab("shelf")
                }
            }

            // 2. Clips Chip
            Rectangle {
                width: parent.chipWidth
                height: 52
                radius: Theme.chipRadius
                color: clipsMouse.pressed ? Theme.cardPressed : Theme.card
                border.color: Theme.hairline
                border.width: Theme.hairlineWidth
                antialiasing: true

                scale: clipsMouse.pressed ? 0.94 : 1.0
                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                readonly property int count: (clipboardManager ? clipboardManager.itemCount : 0)

                Row {
                    anchors.centerIn: parent
                    spacing: 8
                    visible: parent.count > 0

                    IconImage {
                        source: "qrc:/resources/icons/clipboard.svg"
                        size: 18
                        color: Theme.accentGreen
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 1

                        Text {
                            text: parent.parent.parent.count.toString()
                            color: Theme.textPrimary
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.fontTitleSize
                            font.weight: Theme.fontTitleWeight
                        }

                        Text {
                            text: "Clips"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
                            font.weight: Theme.fontCaptionWeight
                        }
                    }
                }

                IconImage {
                    visible: parent.count === 0
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/clipboard.svg"
                    size: 20
                    color: Theme.textTertiary
                }

                MouseArea {
                    id: clipsMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (appState) appState.setCurrentTab("clipboard")
                }
            }

            // 3. Mail Chip
            Rectangle {
                width: parent.chipWidth
                height: 52
                radius: Theme.chipRadius
                color: mailMouse.pressed ? Theme.cardPressed : Theme.card
                border.color: Theme.hairline
                border.width: Theme.hairlineWidth
                antialiasing: true

                scale: mailMouse.pressed ? 0.94 : 1.0
                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                readonly property int count: (appState ? appState.unreadMailCount : 0)

                Row {
                    anchors.centerIn: parent
                    spacing: 8
                    visible: parent.count > 0

                    IconImage {
                        source: "qrc:/resources/icons/mail.svg"
                        size: 18
                        color: Theme.accentBlue
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 1

                        Text {
                            text: parent.parent.parent.count.toString()
                            color: Theme.textPrimary
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.fontTitleSize
                            font.weight: Theme.fontTitleWeight
                        }

                        Text {
                            text: "Mail"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
                            font.weight: Theme.fontCaptionWeight
                        }
                    }
                }

                IconImage {
                    visible: parent.count === 0
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/mail.svg"
                    size: 20
                    color: Theme.textTertiary
                }

                MouseArea {
                    id: mailMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (appState) appState.setCurrentTab("mail")
                }
            }

            // 4. Next Chip (Next calendar event or Poya day)
            Rectangle {
                width: parent.chipWidth
                height: 52
                radius: Theme.chipRadius
                color: nextChipMouse.pressed ? Theme.cardPressed : Theme.card
                border.color: Theme.hairline
                border.width: Theme.hairlineWidth
                antialiasing: true

                scale: nextChipMouse.pressed ? 0.94 : 1.0
                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }

                readonly property string nextText: {
                    if (appState && appState.isPoyaToday) return "Poya"
                    if (appState && appState.isHolidayToday) return "Holiday"
                    return ""
                }

                Row {
                    anchors.centerIn: parent
                    spacing: 8
                    visible: parent.nextText !== ""

                    IconImage {
                        source: "qrc:/resources/icons/calendar.svg"
                        size: 18
                        color: Theme.accentRed
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 1

                        Text {
                            text: parent.parent.parent.nextText
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            font.weight: Theme.fontTitleWeight
                            elide: Text.ElideRight
                        }

                        Text {
                            text: "Next"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
                            font.weight: Theme.fontCaptionWeight
                        }
                    }
                }

                IconImage {
                    visible: parent.nextText === ""
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/calendar.svg"
                    size: 20
                    color: Theme.textTertiary
                }

                MouseArea {
                    id: nextChipMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: if (appState) appState.setCurrentTab("calendar")
                }
            }
        }
    }
}
