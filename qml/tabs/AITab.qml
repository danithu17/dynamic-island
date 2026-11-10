import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 456
    implicitHeight: 204

    property string aiResponse: "Hello! I am Aura AI, your native desktop copilot. Ask me for code solutions, shell commands, regex, or explanations."
    property string displayedResponse: ""
    property int typeIndex: 0
    property bool isGenerating: false
    property bool copiedSuccess: false

    Component.onCompleted: {
        displayedResponse = aiResponse
    }

    Timer {
        id: typewriterTimer
        interval: 16
        repeat: true
        running: root.isGenerating
        onTriggered: {
            if (root.typeIndex < root.aiResponse.length) {
                root.displayedResponse = root.aiResponse.substring(0, root.typeIndex + 1)
                root.typeIndex++
            } else {
                root.isGenerating = false
            }
        }
    }

    function askAI(prompt) {
        if (!prompt || prompt.trim() === "") return
        root.isGenerating = true
        root.typeIndex = 0
        root.displayedResponse = ""
        root.copiedSuccess = false

        var query = prompt.toLowerCase().trim()
        var reply = ""

        if (query.indexOf("git") !== -1 || query.indexOf("commit") !== -1) {
            reply = "Here is the recommended Git sequence:\n`git add .`\n`git commit -m \"feat: update features\"`\n`git push origin main`"
        } else if (query.indexOf("powershell") !== -1 || query.indexOf("ps") !== -1 || query.indexOf("cmd") !== -1) {
            reply = "PowerShell Quick Command:\n`Get-Process | Sort-Object CPU -Descending | Select-Object -First 5`\nReturns the top 5 CPU-intensive processes."
        } else if (query.indexOf("regex") !== -1 || query.indexOf("email") !== -1) {
            reply = "Email Regex Pattern:\n`^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$`\nMatches standard RFC-compliant email addresses."
        } else if (query.indexOf("code") !== -1 || query.indexOf("explain") !== -1) {
            reply = "Clean Architecture Tip:\nDecouple UI from core state using Signals/Slots and singletons. Keep native HWND operations asynchronous."
        } else {
            reply = "Query analyzed: \"" + prompt + "\"\nSolution ready. Use shortcut Ctrl+Alt+Space anytime to invoke Aura Island."
        }

        root.aiResponse = reply
    }

    Column {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 6

        // 1. Top Header Row with Status & Chips
        Row {
            width: parent.width
            height: 22
            spacing: 6

            Row {
                spacing: 5
                anchors.verticalCenter: parent.verticalCenter
                IconImage {
                    source: "qrc:/resources/icons/sparkles.svg"
                    size: 14
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "Aura AI"
                    color: "#FFFFFF"
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Rectangle {
                width: 6; height: 6; radius: 3
                color: Theme.success
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                text: "Claude / Local Engine"
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 9
                anchors.verticalCenter: parent.verticalCenter
            }

            // Preset Quick Chips
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                Repeater {
                    model: ["Explain", "PowerShell", "Regex"]

                    Rectangle {
                        width: chipText.implicitWidth + 10
                        height: 18
                        radius: 9
                        color: chipMouse.containsMouse ? Theme.glassSurfaceHover : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                        border.color: Qt.rgba(1.0, 1.0, 1.0, 0.12)
                        border.width: 1

                        Text {
                            id: chipText
                            anchors.centerIn: parent
                            text: modelData
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 9
                        }

                        MouseArea {
                            id: chipMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (modelData === "Explain") aiInput.text = "Explain this code snippet:"
                                else if (modelData === "PowerShell") aiInput.text = "Give PowerShell command to check memory usage"
                                else if (modelData === "Regex") aiInput.text = "Regex for email validation"
                                aiInput.forceActiveFocus()
                            }
                        }
                    }
                }
            }
        }

        // 2. Response Area Card
        Rectangle {
            width: parent.width
            height: 94
            radius: 12
            color: Theme.cardBackground || "#161618"
            border.color: root.isGenerating ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1

            Behavior on border.color { ColorAnimation { duration: 180 } }

            Flickable {
                id: responseFlick
                anchors.fill: parent
                anchors.margins: 8
                anchors.rightMargin: 40
                contentWidth: width
                contentHeight: responseText.implicitHeight
                clip: true

                Text {
                    id: responseText
                    width: parent.width
                    text: root.displayedResponse
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    wrapMode: Text.Wrap
                }
            }

            // Copy Response Button
            Rectangle {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.margins: 6
                width: 24
                height: 24
                radius: 6
                color: copyMouse.containsMouse ? Qt.rgba(1.0, 1.0, 1.0, 0.18) : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                scale: copyMouse.pressed ? 0.94 : 1.0

                IconImage {
                    anchors.centerIn: parent
                    source: root.copiedSuccess ? "qrc:/resources/icons/check.svg" : "qrc:/resources/icons/copy.svg"
                    size: 11
                    color: root.copiedSuccess ? Theme.success : Theme.textPrimary
                }

                MouseArea {
                    id: copyMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (clipboardManager && clipboardManager.copyToClipboard) {
                            clipboardManager.copyToClipboard(root.displayedResponse)
                        }
                        root.copiedSuccess = true
                        copyResetTimer.restart()
                        if (appState) appState.showToast("Response Copied to Clipboard", "success")
                    }
                }
            }

            Timer {
                id: copyResetTimer
                interval: 2000
                onTriggered: root.copiedSuccess = false
            }
        }

        // 3. Prompt Input Field with Send Button
        Rectangle {
            width: parent.width
            height: 32
            radius: 10
            color: Qt.rgba(1.0, 1.0, 1.0, 0.07)
            border.color: aiInput.activeFocus ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.12)
            border.width: 1

            TextInput {
                id: aiInput
                anchors.left: parent.left
                anchors.right: sendBtn.left
                anchors.leftMargin: 10
                anchors.rightMargin: 6
                anchors.verticalCenter: parent.verticalCenter
                color: "#FFFFFF"
                font.family: Theme.fontFamily
                font.pixelSize: 11
                selectByMouse: true

                Text {
                    text: "Ask AI prompt or type question..."
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    visible: !aiInput.text && !aiInput.activeFocus
                    anchors.verticalCenter: parent.verticalCenter
                }

                onActiveFocusChanged: {
                    if (activeFocus && typeof mainWindow !== "undefined" && mainWindow && mainWindow.requestWindowFocus) {
                        mainWindow.requestWindowFocus()
                    }
                }

                Keys.onReturnPressed: {
                    root.askAI(text)
                    text = ""
                }
            }

            Rectangle {
                id: sendBtn
                anchors.right: parent.right
                anchors.rightMargin: 3
                anchors.verticalCenter: parent.verticalCenter
                width: 26
                height: 26
                radius: 8
                color: Theme.accent
                scale: sendMouse.pressed ? 0.92 : (sendMouse.containsMouse ? 1.05 : 1.0)
                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.6 } }

                IconImage {
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/sparkles.svg"
                    size: 12
                    color: "#FFFFFF"
                }

                MouseArea {
                    id: sendMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.askAI(aiInput.text)
                        aiInput.text = ""
                    }
                }
            }
        }

        // 4. Claude Code / Codex Agent Permission Card
        Rectangle {
            width: parent.width
            height: 34
            radius: 10
            color: Qt.rgba(1.0, 1.0, 1.0, 0.05)
            border.color: Qt.rgba(1.0, 1.0, 1.0, 0.08)
            border.width: 1

            Row {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 8
                spacing: 8

                IconImage {
                    source: "qrc:/resources/icons/dev.svg"
                    size: 13
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 150
                    spacing: 1

                    Text {
                        text: "Agent Tool Approval"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        font.bold: true
                    }
                    Text {
                        text: (appState && appState.agentMessage !== "") ? appState.agentMessage : "Claude Code: exec 'git status --short'"
                        color: Theme.textSecondary
                        font.family: Theme.fontMono
                        font.pixelSize: 8
                        elide: Text.ElideRight
                        width: parent.width
                    }
                }

                // Allow / Deny Buttons
                Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 4

                    Rectangle {
                        width: 48
                        height: 20
                        radius: 6
                        color: Theme.success
                        scale: allowMouse.pressed ? 0.94 : 1.0

                        Text {
                            anchors.centerIn: parent
                            text: "Allow"
                            color: "#FFFFFF"
                            font.family: Theme.fontFamily
                            font.pixelSize: 9
                            font.bold: true
                        }

                        MouseArea {
                            id: allowMouse
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (appState && appState.approveAgentAction) {
                                    appState.approveAgentAction()
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: 44
                        height: 20
                        radius: 6
                        color: Qt.rgba(1.0, 0.2, 0.2, 0.25)
                        border.color: Theme.danger
                        border.width: 1
                        scale: denyMouse.pressed ? 0.94 : 1.0

                        Text {
                            anchors.centerIn: parent
                            text: "Deny"
                            color: Theme.danger
                            font.family: Theme.fontFamily
                            font.pixelSize: 9
                            font.bold: true
                        }

                        MouseArea {
                            id: denyMouse
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (appState && appState.rejectAgentAction) {
                                    appState.rejectAgentAction()
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
