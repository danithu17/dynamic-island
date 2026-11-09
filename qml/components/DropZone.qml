import QtQuick
import QtQuick.Controls
import ".."

Item {
    id: root

    implicitWidth: 480
    implicitHeight: 200

    property int hoveredCardIndex: -1 // 0: mail, 1: save, 2: phone
    property bool isSwallowing: false
    property int swallowedIndex: -1

    // Overall Drop Area to catch incoming drag events and open drop panel
    DropArea {
        id: generalDropArea
        anchors.fill: parent
        onEntered: function(drag) {
            if (drag.hasUrls || drag.hasText) {
                drag.acceptProposedAction()
                if (appState && appState.islandState !== "drop") {
                    appState.setIslandState("drop")
                }
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // A. QR Code Display State (When "To your phone" is active)
    // ─────────────────────────────────────────────────────────
    Item {
        id: qrView
        anchors.fill: parent
        visible: dropManager && dropManager.isQrActive

        Rectangle {
            anchors.fill: parent
            radius: 24
            color: "#1C1C1E"
            border.color: Theme.accent
            border.width: 1.5

            Row {
                anchors.centerIn: parent
                spacing: 20

                // QR Code Image container
                Rectangle {
                    width: 130
                    height: 130
                    radius: 14
                    color: "#FFFFFF"
                    clip: true
                    anchors.verticalCenter: parent.verticalCenter

                    Image {
                        id: qrImage
                        anchors.fill: parent
                        anchors.margins: 6
                        source: (dropManager && dropManager.shareServer.qrSvgPath !== "") ? ("file:///" + dropManager.shareServer.qrSvgPath) : ""
                        fillMode: Image.PreserveAspectFit
                        cache: false
                    }
                }

                // QR Info Column
                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 270
                    spacing: 6

                    Row {
                        spacing: 8
                        IconImage {
                            source: "qrc:/resources/icons/phone.svg"
                            size: 18
                            color: Theme.accent
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: "Scan to download"
                            color: "#FFFFFF"
                            font.family: Theme.fontFamily
                            font.pixelSize: 15
                            font.bold: true
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Text {
                        text: dropManager ? dropManager.shareServer.sharedName : ""
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        elide: Text.ElideMiddle
                        width: parent.width
                    }

                    Text {
                        text: "Phone must be on the same Wi-Fi."
                        color: "#FF9F0A" // warm amber note
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        font.weight: Font.Medium
                    }

                    Row {
                        spacing: 8
                        Text {
                            text: {
                                if (!dropManager) return ""
                                var s = dropManager.shareServer.remainingSeconds
                                var m = Math.floor(s / 60)
                                var sec = s % 60
                                return "Expires in " + m + ":" + (sec < 10 ? "0" : "") + sec
                            }
                            color: Theme.textMuted
                            font.family: Theme.fontMono
                            font.pixelSize: 11
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        GlassButton {
                            text: "Dismiss"
                            cornerRadius: 6
                            implicitHeight: 24
                            onClicked: {
                                if (dropManager) dropManager.dismissQr()
                            }
                        }
                    }
                }
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // B. Normal 3-Card Drop Zone
    // ─────────────────────────────────────────────────────────
    Item {
        id: normalDropView
        anchors.fill: parent
        visible: !dropManager || !dropManager.isQrActive

        // Row of 3 equal dark cards (#1c1c1e, radius 22)
        Row {
            anchors.centerIn: parent
            spacing: 12

            // CARD 1: "Mail it" / "With your Gmail"
            Item {
                id: card1
                width: 138
                height: 164
                scale: (root.hoveredCardIndex === 0) ? 1.04 : 1.0
                opacity: (root.hoveredCardIndex !== -1 && root.hoveredCardIndex !== 0) ? 0.50 : 1.0

                Behavior on scale {
                    SpringAnimation { spring: 3.5; damping: 0.55; duration: 200 }
                }
                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }

                Rectangle {
                    id: card1Bg
                    anchors.fill: parent
                    radius: 22
                    color: "#1C1C1E"
                    border.width: (root.hoveredCardIndex === 0) ? 2 : 1
                    border.color: (root.hoveredCardIndex === 0) ? "#FFFFFF" : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                    clip: true

                    // Orange radial glow from the bottom
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 1.2
                        height: 70
                        radius: 35
                        opacity: (root.hoveredCardIndex === 0) ? 0.45 : 0.0
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#FF9F0A" }
                            GradientStop { position: 1.0; color: "transparent" }
                        }
                        Behavior on opacity {
                            NumberAnimation { duration: 180 }
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 8
                        width: parent.width - 20

                        Rectangle {
                            width: 44
                            height: 44
                            radius: 12
                            color: Qt.rgba(1.0, 1.0, 1.0, 0.06)
                            anchors.horizontalCenter: parent.horizontalCenter

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/mail.svg"
                                size: 22
                                color: (root.hoveredCardIndex === 0) ? "#FF9F0A" : "#FFFFFF"
                            }
                        }

                        Text {
                            text: "Mail it"
                            color: "#FFFFFF"
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "With your Gmail"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                DropArea {
                    anchors.fill: parent
                    onEntered: function(drag) {
                        if (drag.hasUrls || drag.hasText) {
                            root.hoveredCardIndex = 0
                            drag.acceptProposedAction()
                        }
                    }
                    onExited: {
                        if (root.hoveredCardIndex === 0) root.hoveredCardIndex = -1
                    }
                    onDropped: function(drop) {
                        triggerSwallow(0, drop)
                    }
                }
            }

            // CARD 2: "Save" / "In your tray"
            Item {
                id: card2
                width: 138
                height: 164
                scale: (root.hoveredCardIndex === 1) ? 1.04 : 1.0
                opacity: (root.hoveredCardIndex !== -1 && root.hoveredCardIndex !== 1) ? 0.50 : 1.0

                Behavior on scale {
                    SpringAnimation { spring: 3.5; damping: 0.55; duration: 200 }
                }
                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }

                Rectangle {
                    id: card2Bg
                    anchors.fill: parent
                    radius: 22
                    color: "#1C1C1E"
                    border.width: (root.hoveredCardIndex === 1) ? 2 : 1
                    border.color: (root.hoveredCardIndex === 1) ? "#FFFFFF" : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                    clip: true

                    // Orange radial glow from the bottom
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 1.2
                        height: 70
                        radius: 35
                        opacity: (root.hoveredCardIndex === 1) ? 0.45 : 0.0
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#FF9F0A" }
                            GradientStop { position: 1.0; color: "transparent" }
                        }
                        Behavior on opacity {
                            NumberAnimation { duration: 180 }
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 8
                        width: parent.width - 20

                        Rectangle {
                            width: 44
                            height: 44
                            radius: 12
                            color: Qt.rgba(1.0, 1.0, 1.0, 0.06)
                            anchors.horizontalCenter: parent.horizontalCenter

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/shelf.svg"
                                size: 22
                                color: (root.hoveredCardIndex === 1) ? "#FF9F0A" : "#FFFFFF"
                            }
                        }

                        Text {
                            text: "Save"
                            color: "#FFFFFF"
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "In your tray"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                DropArea {
                    anchors.fill: parent
                    onEntered: function(drag) {
                        if (drag.hasUrls || drag.hasText) {
                            root.hoveredCardIndex = 1
                            drag.acceptProposedAction()
                        }
                    }
                    onExited: {
                        if (root.hoveredCardIndex === 1) root.hoveredCardIndex = -1
                    }
                    onDropped: function(drop) {
                        triggerSwallow(1, drop)
                    }
                }
            }

            // CARD 3: "To your phone" / "With a QR code"
            Item {
                id: card3
                width: 138
                height: 164
                scale: (root.hoveredCardIndex === 2) ? 1.04 : 1.0
                opacity: (root.hoveredCardIndex !== -1 && root.hoveredCardIndex !== 2) ? 0.50 : 1.0

                Behavior on scale {
                    SpringAnimation { spring: 3.5; damping: 0.55; duration: 200 }
                }
                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }

                Rectangle {
                    id: card3Bg
                    anchors.fill: parent
                    radius: 22
                    color: "#1C1C1E"
                    border.width: (root.hoveredCardIndex === 2) ? 2 : 1
                    border.color: (root.hoveredCardIndex === 2) ? "#FFFFFF" : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                    clip: true

                    // Orange radial glow from the bottom
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 1.2
                        height: 70
                        radius: 35
                        opacity: (root.hoveredCardIndex === 2) ? 0.45 : 0.0
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#FF9F0A" }
                            GradientStop { position: 1.0; color: "transparent" }
                        }
                        Behavior on opacity {
                            NumberAnimation { duration: 180 }
                        }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 8
                        width: parent.width - 20

                        Rectangle {
                            width: 44
                            height: 44
                            radius: 12
                            color: Qt.rgba(1.0, 1.0, 1.0, 0.06)
                            anchors.horizontalCenter: parent.horizontalCenter

                            IconImage {
                                anchors.centerIn: parent
                                source: "qrc:/resources/icons/phone.svg"
                                size: 22
                                color: (root.hoveredCardIndex === 2) ? "#FF9F0A" : "#FFFFFF"
                            }
                        }

                        Text {
                            text: "To your phone"
                            color: "#FFFFFF"
                            font.family: Theme.fontFamily
                            font.pixelSize: 14
                            font.bold: true
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "With a QR code"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }

                DropArea {
                    anchors.fill: parent
                    onEntered: function(drag) {
                        if (drag.hasUrls || drag.hasText) {
                            root.hoveredCardIndex = 2
                            drag.acceptProposedAction()
                        }
                    }
                    onExited: {
                        if (root.hoveredCardIndex === 2) root.hoveredCardIndex = -1
                    }
                    onDropped: function(drop) {
                        triggerSwallow(2, drop)
                    }
                }
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // Swallow animation ghost item
    // ─────────────────────────────────────────────────────────
    Rectangle {
        id: swallowGhost
        width: 60
        height: 60
        radius: 14
        color: Qt.rgba(1.0, 1.0, 1.0, 0.9)
        visible: root.isSwallowing
        scale: 1.0
        opacity: 1.0
        anchors.centerIn: {
            if (root.swallowedIndex === 0) return card1
            if (root.swallowedIndex === 1) return card2
            return card3
        }

        ParallelAnimation {
            id: swallowAnim
            NumberAnimation { target: swallowGhost; property: "scale"; to: 0.15; duration: 260; easing.type: Easing.InBack }
            NumberAnimation { target: swallowGhost; property: "opacity"; to: 0.0; duration: 240 }
            onFinished: {
                root.isSwallowing = false
            }
        }
    }

    // Card bounce animation
    SequentialAnimation {
        id: cardBounceAnim
        property var targetCard: null

        NumberAnimation { target: cardBounceAnim.targetCard; property: "scale"; to: 1.09; duration: 100; easing.type: Easing.OutQuad }
        SpringAnimation { target: cardBounceAnim.targetCard; property: "scale"; to: 1.0; spring: 4.2; damping: 0.52; duration: 250 }
    }

    function triggerSwallow(cardIndex, drop) {
        root.swallowedIndex = cardIndex
        root.isSwallowing = true
        swallowGhost.scale = 1.0
        swallowGhost.opacity = 1.0
        swallowAnim.restart()

        var card = (cardIndex === 0) ? card1 : (cardIndex === 1 ? card2 : card3)
        cardBounceAnim.targetCard = card
        cardBounceAnim.restart()

        var dataStr = ""
        var typeStr = "text"
        var nameStr = ""

        if (drop.hasUrls && drop.urls.length > 0) {
            dataStr = drop.urls[0]
            // Strip file:/// if present
            if (dataStr.indexOf("file:///") === 0) {
                dataStr = dataStr.substring(8)
            }
            typeStr = "file"
            var parts = dataStr.split(/[/\\]/)
            nameStr = parts[parts.length - 1]
        } else if (drop.hasText) {
            dataStr = drop.text
            typeStr = (dataStr.indexOf("http://") === 0 || dataStr.indexOf("https://") === 0) ? "url" : "text"
            nameStr = (typeStr === "url") ? "Shared Link" : "Note"
        }

        drop.acceptProposedAction()
        root.hoveredCardIndex = -1

        if (dropManager) {
            if (cardIndex === 0) {
                dropManager.handleDropMail(dataStr, typeStr, nameStr)
            } else if (cardIndex === 1) {
                dropManager.handleDropSave(dataStr, typeStr, nameStr)
            } else if (cardIndex === 2) {
                dropManager.handleDropPhone(dataStr, typeStr, nameStr)
            }
        }
    }
}
