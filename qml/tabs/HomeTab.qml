import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 456
    implicitHeight: 180

    Grid {
        anchors.fill: parent
        columns: 3
        rows: 2
        spacing: 8

        // ─────────────────────────────────────────────────────
        // 1. JOBS TODAY / TASKS
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: 146
            height: 82
            radius: 16
            color: jobsMouse.containsMouse ? Theme.glassSurfaceHover : Theme.cardBackground
            border.color: jobsMouse.containsMouse ? Theme.glassBorderHighlight : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1
            scale: jobsMouse.pressed ? 0.98 : (jobsMouse.containsMouse ? 1.02 : 1.0)
            Behavior on scale { SpringAnimation { spring: 3.5; damping: 0.6 } }
            Behavior on color { ColorAnimation { duration: 120 } }

            Column {
                anchors.fill: parent
                anchors.margins: 9
                spacing: 4

                Row {
                    spacing: 6
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 6
                        color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.2)
                        IconImage {
                            anchors.centerIn: parent
                            source: "qrc:/resources/icons/check.svg"
                            size: 11
                            color: Theme.accent
                        }
                    }
                    Text {
                        text: "Jobs today"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    text: (notesManager && notesManager.model) ? (notesManager.model.rowCount() + " notes / items") : "3 tasks pending"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: "Tap to view list"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 9
                }
            }

            MouseArea {
                id: jobsMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: if (appState) appState.setCurrentTab("notes")
            }
        }

        // ─────────────────────────────────────────────────────
        // 2. NOW PLAYING / MEDIA
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: 146
            height: 82
            radius: 16
            color: mediaMouse.containsMouse ? Theme.glassSurfaceHover : Theme.cardBackground
            border.color: mediaMouse.containsMouse ? Theme.glassBorderHighlight : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1
            scale: mediaMouse.pressed ? 0.98 : (mediaMouse.containsMouse ? 1.02 : 1.0)
            Behavior on scale { SpringAnimation { spring: 3.5; damping: 0.6 } }
            Behavior on color { ColorAnimation { duration: 120 } }

            Column {
                anchors.fill: parent
                anchors.margins: 9
                spacing: 4

                Row {
                    width: parent.width
                    spacing: 6
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 6
                        color: Qt.rgba(0.75, 0.35, 0.95, 0.2)
                        IconImage {
                            anchors.centerIn: parent
                            source: "qrc:/resources/icons/media.svg"
                            size: 11
                            color: "#BF5AF2"
                        }
                    }
                    Text {
                        text: "Now playing"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    text: (appState && appState.nowPlayingTitle !== "") ? appState.nowPlayingTitle : "No media playing"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.bold: true
                    elide: Text.ElideRight
                    width: parent.width
                }

                Row {
                    spacing: 6
                    EqualizerBars {
                        barColor: "#BF5AF2"
                        barCount: 4
                        implicitWidth: 16
                        implicitHeight: 10
                        isPlaying: appState ? appState.isPlaying : false
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: (appState && appState.nowPlayingArtist !== "") ? appState.nowPlayingArtist : "System audio"
                        color: Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: 9
                        elide: Text.ElideRight
                        width: 100
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            MouseArea {
                id: mediaMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: if (appState) appState.setCurrentTab("media")
            }
        }

        // ─────────────────────────────────────────────────────
        // 3. FOCUS TIMER / POMODORO
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: 146
            height: 82
            radius: 16
            color: focusMouse.containsMouse ? Theme.glassSurfaceHover : Theme.cardBackground
            border.color: focusMouse.containsMouse ? Theme.glassBorderHighlight : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1
            scale: focusMouse.pressed ? 0.98 : (focusMouse.containsMouse ? 1.02 : 1.0)
            Behavior on scale { SpringAnimation { spring: 3.5; damping: 0.6 } }
            Behavior on color { ColorAnimation { duration: 120 } }

            Column {
                anchors.fill: parent
                anchors.margins: 9
                spacing: 4

                Row {
                    spacing: 6
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 6
                        color: Qt.rgba(1.0, 0.62, 0.04, 0.2)
                        IconImage {
                            anchors.centerIn: parent
                            source: "qrc:/resources/icons/timer.svg"
                            size: 11
                            color: "#FF9F0A"
                        }
                    }
                    Text {
                        text: "Focus timer"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    text: appState ? appState.pomodoroDisplay : "25:00"
                    color: "#FF9F0A"
                    font.family: Theme.fontMono
                    font.pixelSize: 14
                    font.bold: true
                }

                Text {
                    text: "Work session"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 9
                }
            }

            MouseArea {
                id: focusMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: if (appState) appState.setCurrentTab("dev")
            }
        }

        // ─────────────────────────────────────────────────────
        // 4. TRAY / SHELF
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: 146
            height: 82
            radius: 16
            color: trayMouse.containsMouse ? Theme.glassSurfaceHover : Theme.cardBackground
            border.color: trayMouse.containsMouse ? Theme.glassBorderHighlight : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1
            scale: trayMouse.pressed ? 0.98 : (trayMouse.containsMouse ? 1.02 : 1.0)
            Behavior on scale { SpringAnimation { spring: 3.5; damping: 0.6 } }
            Behavior on color { ColorAnimation { duration: 120 } }

            Column {
                anchors.fill: parent
                anchors.margins: 9
                spacing: 4

                Row {
                    spacing: 6
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 6
                        color: Qt.rgba(0.19, 0.82, 0.35, 0.2)
                        IconImage {
                            anchors.centerIn: parent
                            source: "qrc:/resources/icons/shelf.svg"
                            size: 11
                            color: "#30D158"
                        }
                    }
                    Text {
                        text: "Tray"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    text: (shelfManager && shelfManager.model) ? (shelfManager.model.rowCount() + " items saved") : "Quick storage"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: "%APPDATA%/tray"
                    color: Theme.textMuted
                    font.family: Theme.fontMono
                    font.pixelSize: 8.5
                    elide: Text.ElideMiddle
                    width: parent.width
                }
            }

            MouseArea {
                id: trayMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: if (appState) appState.setCurrentTab("shelf")
            }
        }

        // ─────────────────────────────────────────────────────
        // 5. CLIPS / CLIPBOARD HISTORY
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: 146
            height: 82
            radius: 16
            color: clipsMouse.containsMouse ? Theme.glassSurfaceHover : Theme.cardBackground
            border.color: clipsMouse.containsMouse ? Theme.glassBorderHighlight : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1
            scale: clipsMouse.pressed ? 0.98 : (clipsMouse.containsMouse ? 1.02 : 1.0)
            Behavior on scale { SpringAnimation { spring: 3.5; damping: 0.6 } }
            Behavior on color { ColorAnimation { duration: 120 } }

            Column {
                anchors.fill: parent
                anchors.margins: 9
                spacing: 4

                Row {
                    spacing: 6
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 6
                        color: Qt.rgba(0.04, 0.52, 1.0, 0.2)
                        IconImage {
                            anchors.centerIn: parent
                            source: "qrc:/resources/icons/clipboard.svg"
                            size: 11
                            color: "#0A84FF"
                        }
                    }
                    Text {
                        text: "Clips"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    text: (clipboardManager && clipboardManager.model) ? (clipboardManager.model.rowCount() + " clips in history") : "Clipboard log"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: "Secrets filtered & safe"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 9
                }
            }

            MouseArea {
                id: clipsMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: if (appState) appState.setCurrentTab("clipboard")
            }
        }

        // ─────────────────────────────────────────────────────
        // 6. CALENDAR & SRI LANKA POYA DAY
        // ─────────────────────────────────────────────────────
        Rectangle {
            width: 146
            height: 82
            radius: 16
            color: calMouse.containsMouse ? Theme.glassSurfaceHover : Theme.cardBackground
            border.color: calMouse.containsMouse ? Theme.glassBorderHighlight : ((appState && appState.isPoyaToday) ? "#FFD60A" : Qt.rgba(1.0, 1.0, 1.0, 0.08))
            border.width: 1
            scale: calMouse.pressed ? 0.98 : (calMouse.containsMouse ? 1.02 : 1.0)
            Behavior on scale { SpringAnimation { spring: 3.5; damping: 0.6 } }
            Behavior on color { ColorAnimation { duration: 120 } }

            Column {
                anchors.fill: parent
                anchors.margins: 9
                spacing: 4

                Row {
                    spacing: 6
                    Rectangle {
                        width: 20
                        height: 20
                        radius: 6
                        color: (appState && appState.isPoyaToday) ? Qt.rgba(1.0, 0.84, 0.04, 0.25) : Qt.rgba(1.0, 1.0, 1.0, 0.1)
                        IconImage {
                            anchors.centerIn: parent
                            source: "qrc:/resources/icons/moon.svg"
                            size: 11
                            color: (appState && appState.isPoyaToday) ? "#FFD60A" : "#FFFFFF"
                        }
                    }
                    Text {
                        text: "Calendar"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Text {
                    text: (appState && appState.isPoyaToday) ? "🌕 " + appState.poyaDayInfo : (appState ? appState.poyaDayInfo : "Il Full Moon")
                    color: (appState && appState.isPoyaToday) ? "#FFD60A" : Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 10.5
                    font.bold: appState ? appState.isPoyaToday : false
                    elide: Text.ElideRight
                    width: parent.width
                }

                Text {
                    text: (appState && appState.isHolidayToday) ? ("🇱🇰 " + appState.holidayInfo) : "Sri Lanka (LK) Calendar"
                    color: (appState && appState.isHolidayToday) ? "#FF9F0A" : Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 9
                    elide: Text.ElideRight
                    width: parent.width
                }
            }

            MouseArea {
                id: calMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: if (appState) appState.setCurrentTab("dev")
            }
        }
    }
}
