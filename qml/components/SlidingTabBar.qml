import QtQuick
import ".."

Item {
    id: root

    property string activeTab: appState ? appState.currentTab : "home"
    signal tabSelected(string tabId)

    implicitHeight: 44
    implicitWidth: dockRow.implicitWidth + 32

    readonly property var dockItems: [
        { id: "home",      icon: "qrc:/resources/icons/home.svg" },
        { id: "shelf",     icon: "qrc:/resources/icons/shelf.svg" },
        { id: "clipboard", icon: "qrc:/resources/icons/clipboard.svg" },
        { id: "notes",     icon: "qrc:/resources/icons/notes.svg" },
        { id: "media",     icon: "qrc:/resources/icons/media.svg" },
        { id: "more",      icon: "more" }
    ]

    function getActiveIndex() {
        for (var i = 0; i < dockItems.length; i++) {
            if (dockItems[i].id === activeTab) return i
        }
        return -1
    }

    readonly property int activeIdx: getActiveIndex()

    // ─────────────────────────────────────────────────────────
    // Centered Dock Row
    // ─────────────────────────────────────────────────────────
    Row {
        id: dockRow
        anchors.centerIn: parent
        spacing: 20 // 20px spacing as specified

        Repeater {
            model: root.dockItems

            Item {
                id: itemDelegate
                width: 44
                height: 32
                anchors.verticalCenter: parent.verticalCenter

                readonly property bool isActive: (root.activeTab === modelData.id)

                // Sliding translucent pill background behind active item
                Rectangle {
                    anchors.centerIn: parent
                    width: 44
                    height: 32
                    radius: 16
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.12)
                    visible: itemDelegate.isActive
                    antialiasing: true

                    scale: itemMouse.pressed ? 0.94 : 1.0
                    Behavior on scale {
                        SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 }
                    }
                }

                // Regular Icon (22px, monoline)
                IconImage {
                    visible: (modelData.icon !== "more")
                    anchors.centerIn: parent
                    source: (modelData.icon !== "more") ? modelData.icon : ""
                    size: 22
                    color: "#FFFFFF"
                    opacity: itemDelegate.isActive ? 1.0 : 0.55

                    Behavior on opacity {
                        NumberAnimation { duration: 150 }
                    }
                }

                // Three Dots "More" Icon (monoline 22px width)
                Row {
                    visible: (modelData.icon === "more")
                    anchors.centerIn: parent
                    spacing: 4
                    opacity: itemDelegate.isActive ? 1.0 : 0.55

                    Behavior on opacity {
                        NumberAnimation { duration: 150 }
                    }

                    Rectangle { width: 4; height: 4; radius: 2; color: "#FFFFFF"; antialiasing: true }
                    Rectangle { width: 4; height: 4; radius: 2; color: "#FFFFFF"; antialiasing: true }
                    Rectangle { width: 4; height: 4; radius: 2; color: "#FFFFFF"; antialiasing: true }
                }

                MouseArea {
                    id: itemMouse
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: {
                        root.tabSelected(modelData.id)
                    }
                }
            }
        }
    }
}
