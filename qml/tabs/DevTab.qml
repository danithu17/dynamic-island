import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import "../components"
import ".."

Item {
    id: root

    FolderDialog {
        id: folderDialog
        title: "Select Git Repository Directory"
        onAccepted: {
            var path = folderDialog.selectedFolder.toString()
            if (path.startsWith("file:///")) {
                path = path.substring(8)
            }
            if (devManager) devManager.selectRepoPath(path)
        }
    }

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        // Header
        Row {
            width: parent.width
            height: 28

            Text {
                text: "Developer Hub"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 15
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Section 1: Git Repository Status
        GlassCard {
            width: parent.width
            implicitHeight: 64

            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 12

                IconImage {
                    source: "qrc:/resources/icons/git-branch.svg"
                    size: 22
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }

                Column {
                    width: parent.width - 150
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 3

                    Row {
                        spacing: 8
                        Text {
                            text: (devManager && devManager.git && devManager.git.branch !== "-") ? devManager.git.branch : "No Git Repo Selected"
                            color: Theme.textPrimary
                            font.family: Theme.fontMono
                            font.pixelSize: 12
                            font.bold: true
                        }

                        Rectangle {
                            visible: devManager && devManager.git && devManager.git.branch !== "-" && devManager.git.branch !== "Not a git repo"
                            width: uncommitText.implicitWidth + 8
                            height: 16
                            radius: 8
                            color: (devManager.git.uncommittedCount > 0) ? Qt.rgba(1.0, 0.6, 0.0, 0.25) : Qt.rgba(0.2, 0.8, 0.3, 0.25)

                            Text {
                                id: uncommitText
                                anchors.centerIn: parent
                                text: devManager.git.uncommittedCount + " uncommitted"
                                color: (devManager.git.uncommittedCount > 0) ? Theme.warning : Theme.success
                                font.family: Theme.fontFamily
                                font.pixelSize: 9
                                font.bold: true
                            }
                        }
                    }

                    Text {
                        text: (devManager && devManager.git && devManager.git.repoPath !== "") ? devManager.git.repoPath : "Select a local project folder to monitor git status"
                        color: Theme.textMuted
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        elide: Text.ElideMiddle
                        width: parent.width
                    }
                }

                GlassButton {
                    text: "Pick Folder"
                    cornerRadius: 6
                    implicitHeight: 28
                    anchors.verticalCenter: parent.verticalCenter
                    onClicked: folderDialog.open()
                }
            }
        }

        // Section 2: Pomodoro Timer (25 / 5)
        GlassCard {
            width: parent.width
            implicitHeight: 64

            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 12

                IconImage {
                    source: "qrc:/resources/icons/timer.svg"
                    size: 22
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }

                Column {
                    width: 140
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2

                    Text {
                        text: devManager ? devManager.pomodoro.timeDisplay : "25:00"
                        color: Theme.textPrimary
                        font.family: Theme.fontMono
                        font.pixelSize: 18
                        font.bold: true
                    }

                    Text {
                        text: {
                            if (!devManager) return "Pomodoro (Work)"
                            var m = devManager.pomodoro.mode
                            if (m === "work") return "Focus Session (" + devManager.pomodoro.completedCycles + " completed)"
                            if (m === "shortBreak") return "Short Break (5 min)"
                            return "Long Rest Break (15 min)"
                        }
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                    }
                }


                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 6

                    GlassButton {
                        text: (devManager && devManager.pomodoro.isRunning) ? "Pause" : "Start"
                        isAccent: true
                        cornerRadius: 6
                        implicitHeight: 28
                        onClicked: {
                            if (devManager) {
                                if (devManager.pomodoro.isRunning) devManager.pomodoro.pause()
                                else devManager.pomodoro.start()
                            }
                        }
                    }

                    GlassButton {
                        text: "Skip"
                        cornerRadius: 6
                        implicitHeight: 28
                        onClicked: if (devManager) devManager.pomodoro.skipToNext()
                    }

                    GlassButton {
                        text: "Reset"
                        cornerRadius: 6
                        implicitHeight: 28
                        onClicked: if (devManager) devManager.pomodoro.reset()
                    }
                }
            }
        }

        // Section 3: Claude Code / Codex Agent Hook Status
        GlassCard {
            width: parent.width
            implicitHeight: 88

            Column {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 6

                Row {
                    width: parent.width
                    spacing: 8

                    Rectangle {
                        width: 8
                        height: 8
                        radius: 4
                        color: {
                            if (!devManager || !devManager.agentHook) return Theme.textMuted
                            var s = devManager.agentHook.currentStatus
                            if (s === "working") return Theme.accent
                            if (s === "needs_approval") return Theme.warning
                            if (s === "done") return Theme.success
                            return Theme.textMuted
                        }
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Text {
                        text: "AI Agent Status: " + (devManager ? devManager.agentHook.currentAgent : "Claude Code")
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        font.bold: true
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Rectangle {
                        width: statusBadge.implicitWidth + 8
                        height: 16
                        radius: 8
                        color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.2)
                        anchors.verticalCenter: parent.verticalCenter

                        Text {
                            id: statusBadge
                            anchors.centerIn: parent
                            text: devManager ? devManager.agentHook.currentStatus.toUpperCase() : "IDLE"
                            color: Theme.accent
                            font.family: Theme.fontMono
                            font.pixelSize: 9
                            font.bold: true
                        }
                    }
                }

                Text {
                    text: (devManager && devManager.agentHook.currentMessage !== "") ? devManager.agentHook.currentMessage : "Agent hook server listening on 127.0.0.1:" + (devManager ? devManager.agentHook.port : "19827")
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    elide: Text.ElideRight
                    width: parent.width
                }

                // If approval pending, show Allow / Deny buttons
                Row {
                    visible: devManager && devManager.agentHook && devManager.agentHook.isApprovalPending
                    spacing: 8

                    Text {
                        text: "Action: " + (devManager ? devManager.agentHook.pendingAction : "")
                        color: Theme.warning
                        font.family: Theme.fontMono
                        font.pixelSize: 10
                        width: parent.parent.width - 150
                        elide: Text.ElideRight
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    GlassButton {
                        text: "Allow"
                        isAccent: true
                        cornerRadius: 5
                        implicitHeight: 24
                        onClicked: if (devManager) devManager.agentHook.allowAction()
                    }

                    GlassButton {
                        text: "Deny"
                        isDanger: true
                        cornerRadius: 5
                        implicitHeight: 24
                        onClicked: if (devManager) devManager.agentHook.denyAction()
                    }
                }
            }
        }
    }
}
