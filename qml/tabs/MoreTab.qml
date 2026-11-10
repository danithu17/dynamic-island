import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 448
    implicitHeight: 160

    readonly property var moreTiles: [
        { id: "toggles",  label: "Toggles",  icon: "qrc:/resources/icons/sun.svg",      accent: Theme.accentBlue },
        { id: "ai",       label: "AI",       icon: "qrc:/resources/icons/sparkles.svg", accent: Theme.accentPurple },
        { id: "focus",    label: "Timer",    icon: "qrc:/resources/icons/timer.svg",    accent: Theme.accentOrange },
        { id: "mail",     label: "Mail",     icon: "qrc:/resources/icons/mail.svg",     accent: Theme.accentBlue },
        { id: "calendar", label: "Calendar", icon: "qrc:/resources/icons/calendar.svg", accent: Theme.accentRed },
        { id: "games",    label: "Games",    icon: "qrc:/resources/icons/gamepad.svg",  accent: Theme.accentGreen },
        { id: "launcher", label: "Launcher", icon: "qrc:/resources/icons/plus.svg",     accent: Theme.accentOrange },
        { id: "settings", label: "Settings", icon: "qrc:/resources/icons/settings.svg", accent: Theme.accentBlue }
    ]

    // 4x2 Grid of 64px rounded tiles
    Grid {
        anchors.centerIn: parent
        columns: 4
        rows: 2
        spacing: 12

        Repeater {
            model: root.moreTiles

            Item {
                width: 64
                height: 64

                scale: tileMouse.pressed ? 0.94 : 1.0
                Behavior on scale {
                    SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0; epsilon: 0.001 }
                }

                Rectangle {
                    anchors.fill: parent
                    radius: 18
                    color: tileMouse.pressed ? Theme.cardPressed : Theme.card
                    border.color: Theme.hairline
                    border.width: Theme.hairlineWidth
                    antialiasing: true

                    Column {
                        anchors.centerIn: parent
                        spacing: 4

                        IconImage {
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: modelData.icon
                            size: 22
                            color: modelData.accent
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.label
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
                            font.weight: Theme.fontCaptionWeight
                        }
                    }
                }

                MouseArea {
                    id: tileMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (appState) appState.setCurrentTab(modelData.id)
                    }
                }
            }
        }
    }
}
