import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        // Header
        Row {
            width: parent.width
            height: 28

            Text {
                text: "Gmail Hub"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 15
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }

            Text {
                visible: gmailService && gmailService.isAuthenticated && gmailService.userEmail !== ""
                text: " (" + (gmailService ? gmailService.userEmail : "") + ")"
                color: Theme.textSecondary
                font.family: Theme.fontFamily
                font.pixelSize: 11
                anchors.verticalCenter: parent.verticalCenter
            }


            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                GlassButton {
                    visible: gmailService && gmailService.isAuthenticated
                    text: "Refresh"
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: if (gmailService) gmailService.refresh()
                }

                GlassButton {
                    text: (gmailService && gmailService.isAuthenticated) ? "Disconnect" : "Connect Google Account"
                    isAccent: !gmailService || !gmailService.isAuthenticated
                    isDanger: gmailService && gmailService.isAuthenticated
                    cornerRadius: 6
                    implicitHeight: 26
                    onClicked: {
                        if (gmailService) {
                            if (gmailService.isAuthenticated) gmailService.disconnectAccount()
                            else gmailService.startAuth()
                        }
                    }
                }
            }
        }

        // Email List
        ListView {
            id: emailList
            width: parent.width
            height: parent.height - 40
            clip: true
            spacing: 8
            model: gmailService ? gmailService.model : null

            delegate: GlassCard {
                width: emailList.width
                implicitHeight: 52

                Row {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 10

                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: Theme.accent
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Column {
                        width: parent.width - 150
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 2

                        Row {
                            spacing: 8
                            Text {
                                text: model.sender
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                                font.bold: true
                                elide: Text.ElideRight
                                width: Math.min(implicitWidth, 180)
                            }
                            Text {
                                text: model.date
                                color: Theme.textMuted
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                            }
                        }

                        Text {
                            text: model.subject
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.weight: Font.Medium
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: model.snippet
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }

                    Row {
                        anchors.right: parent.right
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 6

                        // Open in browser
                        GlassButton {
                            iconSource: "qrc:/resources/icons/mail.svg"
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (gmailService) gmailService.openMessage(model.messageId)
                        }

                        // Mark Read
                        GlassButton {
                            iconSource: "qrc:/resources/icons/check.svg"
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (gmailService) gmailService.markAsRead(model.messageId)
                        }

                        // Archive
                        GlassButton {
                            iconSource: "qrc:/resources/icons/close.svg"
                            cornerRadius: 5
                            implicitWidth: 26
                            implicitHeight: 26
                            onClicked: if (gmailService) gmailService.archiveMessage(model.messageId)
                        }
                    }
                }
            }

            Column {
                visible: !gmailService || !gmailService.isAuthenticated
                anchors.centerIn: parent
                spacing: 8

                IconImage {
                    source: "qrc:/resources/icons/mail.svg"
                    size: 32
                    color: Theme.textMuted
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                Text {
                    text: "Connect Gmail to monitor unread emails in Liquid Glass"
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                GlassButton {
                    text: "Sign In with Google"
                    isAccent: true
                    anchors.horizontalCenter: parent.horizontalCenter
                    onClicked: if (gmailService) gmailService.startAuth()
                }
            }

            Text {
                visible: gmailService && gmailService.isAuthenticated && emailList.count === 0
                anchors.centerIn: parent
                text: "No unread emails in inbox. You're all caught up!"
                color: Theme.textMuted
                font.family: Theme.fontFamily
                font.pixelSize: 12
            }
        }
    }
}
