import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 456
    implicitHeight: 204

    property int totalSeconds: 25 * 60
    property int remainingSeconds: totalSeconds
    property bool isTimerRunning: false
    property string timerMode: "work" // "work", "short_break", "long_break"

    function formatTime(sec) {
        var m = Math.floor(sec / 60)
        var s = sec % 60
        return (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
    }

    Timer {
        id: pomodoroCountdown
        interval: 1000
        repeat: true
        running: root.isTimerRunning
        onTriggered: {
            if (root.remainingSeconds > 0) {
                root.remainingSeconds--
                if (appState) appState.setPomodoroDisplay(root.formatTime(root.remainingSeconds))
            } else {
                root.isTimerRunning = false
                if (appState) {
                    appState.showToast("Pomodoro Session Complete! Take a break.", "success")
                    appState.setPomodoroDisplay("00:00")
                }
            }
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 8

        // 1. Master Focus Mode Card
        Rectangle {
            width: parent.width
            height: 48
            radius: 14
            color: (appState && appState.isFocusModeActive) ? Qt.rgba(0.04, 0.45, 1.0, 0.15) : (Theme.cardBackground || "#161618")
            border.color: (appState && appState.isFocusModeActive) ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1

            Behavior on color { ColorAnimation { duration: 200 } }
            Behavior on border.color { ColorAnimation { duration: 200 } }

            Row {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 10

                Rectangle {
                    width: 28
                    height: 28
                    radius: 14
                    color: (appState && appState.isFocusModeActive) ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.1)
                    anchors.verticalCenter: parent.verticalCenter

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/moon.svg"
                        size: 15
                        color: (appState && appState.isFocusModeActive) ? "#FFFFFF" : Theme.textSecondary
                    }
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 100
                    spacing: 2

                    Text {
                        text: "Focus Mode (Do Not Disturb)"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.bold: true
                    }
                    Text {
                        text: (appState && appState.isFocusModeActive) ? "Island dimmed • Chimes muted • Distractions blocked" : "Dim island & silence non-essential activity"
                        color: (appState && appState.isFocusModeActive) ? Theme.accent : Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 9
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }

                // Focus Switch
                Rectangle {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    width: 44
                    height: 24
                    radius: 12
                    color: (appState && appState.isFocusModeActive) ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.15)
                    Behavior on color { ColorAnimation { duration: 150 } }

                    Rectangle {
                        width: 18
                        height: 18
                        radius: 9
                        color: "#FFFFFF"
                        anchors.verticalCenter: parent.verticalCenter
                        x: (appState && appState.isFocusModeActive) ? parent.width - 21 : 3
                        Behavior on x { SpringAnimation { spring: 4.5; damping: 0.6 } }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if (appState) appState.toggleFocusMode()
                        }
                    }
                }
            }
        }

        // 2. Pomodoro Timer Area
        Rectangle {
            width: parent.width
            height: 104
            radius: 14
            color: Theme.cardBackground || "#161618"
            border.color: Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1

            Column {
                anchors.centerIn: parent
                spacing: 6

                // Digital Timer
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: root.formatTime(root.remainingSeconds)
                    color: "#FFFFFF"
                    font.family: Theme.fontMono
                    font.pixelSize: 34
                    font.bold: true
                }

                // Mode presets
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 8

                    Repeater {
                        model: [
                            { id: "work", label: "Work (25m)", sec: 25 * 60 },
                            { id: "short_break", label: "Short Break (5m)", sec: 5 * 60 },
                            { id: "long_break", label: "Long Break (15m)", sec: 15 * 60 }
                        ]

                        Rectangle {
                            width: modeTxt.implicitWidth + 12
                            height: 20
                            radius: 10
                            color: root.timerMode === modelData.id ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.08)

                            Text {
                                id: modeTxt
                                anchors.centerIn: parent
                                text: modelData.label
                                color: root.timerMode === modelData.id ? "#FFFFFF" : Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 9
                                font.bold: root.timerMode === modelData.id
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    root.timerMode = modelData.id
                                    root.totalSeconds = modelData.sec
                                    root.remainingSeconds = modelData.sec
                                    root.isTimerRunning = false
                                    if (appState) appState.setPomodoroDisplay(root.formatTime(modelData.sec))
                                }
                            }
                        }
                    }
                }

                // Control Buttons (Start / Pause / Reset)
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 10

                    Rectangle {
                        width: 74
                        height: 22
                        radius: 11
                        color: root.isTimerRunning ? Theme.warning : Theme.success
                        scale: playMouse.pressed ? 0.94 : 1.0

                        Row {
                            anchors.centerIn: parent
                            spacing: 4
                            IconImage {
                                source: root.isTimerRunning ? "qrc:/resources/icons/pause.svg" : "qrc:/resources/icons/play.svg"
                                size: 10
                                color: "#FFFFFF"
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            Text {
                                text: root.isTimerRunning ? "Pause" : "Start"
                                color: "#FFFFFF"
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                font.bold: true
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }

                        MouseArea {
                            id: playMouse
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.isTimerRunning = !root.isTimerRunning
                            }
                        }
                    }

                    Rectangle {
                        width: 60
                        height: 22
                        radius: 11
                        color: Qt.rgba(1.0, 1.0, 1.0, 0.10)
                        scale: resetMouse.pressed ? 0.94 : 1.0

                        Text {
                            anchors.centerIn: parent
                            text: "Reset"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                        }

                        MouseArea {
                            id: resetMouse
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                root.isTimerRunning = false
                                root.remainingSeconds = root.totalSeconds
                                if (appState) appState.setPomodoroDisplay(root.formatTime(root.totalSeconds))
                            }
                        }
                    }
                }
            }
        }

        // 3. Motivational Zen Quote
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: "“Do one thing with complete dedication. Flow is found in focus.”"
            color: Theme.textMuted
            font.family: Theme.fontFamily
            font.pixelSize: 9
            font.italic: true
        }
    }
}
