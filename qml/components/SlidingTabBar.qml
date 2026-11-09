import QtQuick
import ".."

Item {
    id: root
    property string activeTab: appState ? appState.currentTab : "shelf"
    signal tabSelected(string tabId)

    implicitHeight: 38
    implicitWidth: 480

    readonly property var tabs: [
        { id: "home", label: "Home", icon: "qrc:/resources/icons/home.svg" },
        { id: "shelf", label: "Tray", icon: "qrc:/resources/icons/shelf.svg" },
        { id: "clipboard", label: "Clips", icon: "qrc:/resources/icons/clipboard.svg" },
        { id: "notes", label: "Notes", icon: "qrc:/resources/icons/notes.svg" },
        { id: "media", label: "Media", icon: "qrc:/resources/icons/media.svg" },
        { id: "mail", label: "Mail", icon: "qrc:/resources/icons/mail.svg", badge: (appState ? appState.unreadMailCount : 0) },
        { id: "dev", label: "Dev", icon: "qrc:/resources/icons/dev.svg", badge: (appState ? (appState.agentStatus === "needs_approval" ? "!" : "") : "") },
        { id: "settings", label: "", icon: "qrc:/resources/icons/settings.svg" }
    ]

    function getActiveIndex() {
        for (var i = 0; i < tabs.length; i++) {
            if (tabs[i].id === activeTab) return i;
        }
        return 0;
    }

    Rectangle {
        id: barBackground
        anchors.fill: parent
        radius: 19
        color: Qt.rgba(0.0, 0.0, 0.0, 0.35)
        border.color: Theme.glassBorder
        border.width: 1

        // Sliding Glass Indicator
        Rectangle {
            id: indicator
            property int targetIdx: root.getActiveIndex()
            property Item targetTab: tabRepeater.itemAt(targetIdx)

            x: targetTab ? targetTab.x + 3 : 3
            y: 3
            width: targetTab ? targetTab.width - 6 : 40
            height: parent.height - 6
            radius: 16

            gradient: Gradient {
                GradientStop { position: 0.0; color: (Theme && Theme.accent) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.35) : "#330A84FF" }
                GradientStop { position: 1.0; color: (Theme && Theme.accent) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.18) : "#180A84FF" }
            }
            border.color: (Theme && Theme.accent) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.6) : "#990A84FF"
            border.width: 1

            Behavior on x {
                SpringAnimation {
                    spring:  Theme.springFast
                    damping: Theme.dampingFirm
                    epsilon: 0.5
                }
            }
            Behavior on width {
                SpringAnimation {
                    spring:  Theme.springFast
                    damping: Theme.dampingFirm
                    epsilon: 0.5
                }
            }
        }

        Row {
            id: tabRow
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            spacing: 2

            Repeater {
                id: tabRepeater
                model: root.tabs

                Item {
                    id: tabItem
                    width: (tabRow.width - ((root.tabs.length - 1) * tabRow.spacing)) / root.tabs.length
                    height: tabRow.height

                    readonly property bool isActive: root.activeTab === modelData.id

                    Row {
                        anchors.centerIn: parent
                        spacing: 5

                        IconImage {
                            source: modelData.icon
                            size: 15
                            color: tabItem.isActive ? Theme.textPrimary : Theme.textSecondary
                            anchors.verticalCenter: parent.verticalCenter

                            Behavior on color {
                                ColorAnimation { duration: Theme.animDurationFast }
                            }
                        }

                        Text {
                            visible: tabItem.isActive && modelData.label !== ""
                            text: modelData.label
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            font.weight: Font.DemiBold
                            anchors.verticalCenter: parent.verticalCenter

                            Behavior on opacity {
                                NumberAnimation { duration: 140 }
                            }
                        }

                        // Notification Badge
                        Rectangle {
                            visible: !!modelData.badge && modelData.badge !== 0 && modelData.badge !== ""
                            width: Math.max(badgeText.implicitWidth + 6, 14)
                            height: 14
                            radius: 7
                            color: modelData.badge === "!" ? Theme.warning : Theme.danger
                            anchors.verticalCenter: parent.verticalCenter

                            Text {
                                id: badgeText
                                anchors.centerIn: parent
                                text: modelData.badge
                                color: "#FFFFFF"
                                font.family: Theme.fontFamily
                                font.pixelSize: 9
                                font.bold: true
                            }
                        }
                    }

                    // Active Tab White Dot
                    Rectangle {
                        visible: tabItem.isActive
                        width: 3
                        height: 3
                        radius: 1.5
                        color: "#FFFFFF"
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 2
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: {
                            root.tabSelected(modelData.id)
                            if (appState) appState.setCurrentTab(modelData.id)
                        }
                    }
                }
            }
        }
    }
}
