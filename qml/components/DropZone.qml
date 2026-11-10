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
    property string completionMessage: ""

    // Drop Area covering entire panel
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
    // 1. Completion Pill State (Shows for 1.5s then collapses)
    // ─────────────────────────────────────────────────────────
    Item {
        id: completionView
        anchors.fill: parent
        visible: root.completionMessage !== ""

        Row {
            anchors.centerIn: parent
            spacing: 12

            Rectangle {
                width: 36
                height: 36
                radius: 18
                color: Qt.rgba(Theme.accentGreen.r, Theme.accentGreen.g, Theme.accentGreen.b, 0.20)
                anchors.verticalCenter: parent.verticalCenter

                IconImage {
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/check.svg"
                    size: 20
                    color: Theme.accentGreen
                }
            }

            Text {
                text: root.completionMessage
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: Theme.fontTitleSize
                font.weight: Theme.fontTitleWeight
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // 2. QR Code Display State (When "To your phone" is active)
    // ─────────────────────────────────────────────────────────
    Item {
        id: qrView
        anchors.fill: parent
        visible: (root.completionMessage === "") && (dropManager && dropManager.isQrActive)

        Rectangle {
            anchors.fill: parent
            radius: Theme.cardRadius
            color: Theme.card
            border.color: Theme.hairline
            border.width: Theme.hairlineWidth

            Row {
                anchors.centerIn: parent
                spacing: 20

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

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 270
                    spacing: 6

                    Row {
                        spacing: 8
                        IconImage {
                            source: "qrc:/resources/icons/phone.svg"
                            size: 18
                            color: Theme.accentOrange
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Text {
                            text: "Scan to download"
                            color: "#FFFFFF"
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontTitleSize
                            font.weight: Theme.fontTitleWeight
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }

                    Text {
                        text: dropManager ? dropManager.shareServer.sharedName : ""
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontBodySize
                        elide: Text.ElideMiddle
                        width: parent.width
                    }

                    Text {
                        text: "Phone must be on the same Wi-Fi."
                        color: Theme.accentOrange
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontCaptionSize
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
                            color: Theme.textTertiary
                            font.family: Theme.fontMono
                            font.pixelSize: Theme.fontCaptionSize
                            anchors.verticalCenter: parent.verticalCenter
                        }

                        GlassButton {
                            text: "Dismiss"
                            cornerRadius: 12
                            implicitHeight: 24
                            onClicked: {
                                if (dropManager) dropManager.dismissQr()
                                if (appState) appState.collapseToIdle()
                            }
                        }
                    }
                }
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // 3. Three-Card Drop Zone (148x136 each, in a row)
    // ─────────────────────────────────────────────────────────
    Item {
        id: normalDropView
        anchors.fill: parent
        visible: (root.completionMessage === "") && (!dropManager || !dropManager.isQrActive)

        Row {
            anchors.centerIn: parent
            spacing: 8 // (480 - 148*3) / 2 = 18 margins, 8px spacing

            // CARD 1: "Mail it" (Gmail draft with attachment)
            Item {
                id: card1
                width: 148
                height: 136
                scale: (root.hoveredCardIndex === 0) ? 1.04 : 1.0
                opacity: (root.hoveredCardIndex !== -1 && root.hoveredCardIndex !== 0) ? 0.40 : 1.0

                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                Behavior on opacity { NumberAnimation { duration: 150 } }

                Rectangle {
                    anchors.fill: parent
                    radius: Theme.cardRadius // 20px
                    color: Theme.card
                    border.width: (root.hoveredCardIndex === 0) ? 2 : Theme.hairlineWidth
                    border.color: (root.hoveredCardIndex === 0) ? "#FFFFFF" : Theme.hairline
                    clip: true

                    // Orange radial glow from the bottom
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 1.2
                        height: 60
                        radius: 30
                        opacity: (root.hoveredCardIndex === 0) ? 0.45 : 0.0
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: Theme.accentOrange }
                            GradientStop { position: 1.0; color: "transparent" }
                        }
                        Behavior on opacity { NumberAnimation { duration: 180 } }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 6
                        width: parent.width - 20

                        IconImage {
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "qrc:/resources/icons/mail.svg"
                            size: 26
                            color: (root.hoveredCardIndex === 0) ? Theme.accentOrange : "#FFFFFF"
                        }

                        Text {
                            text: "Mail it"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontTitleSize
                            font.weight: Theme.fontTitleWeight
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "Gmail draft"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
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

            // CARD 2: "Save" (To Tray)
            Item {
                id: card2
                width: 148
                height: 136
                scale: (root.hoveredCardIndex === 1) ? 1.04 : 1.0
                opacity: (root.hoveredCardIndex !== -1 && root.hoveredCardIndex !== 1) ? 0.40 : 1.0

                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                Behavior on opacity { NumberAnimation { duration: 150 } }

                Rectangle {
                    anchors.fill: parent
                    radius: Theme.cardRadius
                    color: Theme.card
                    border.width: (root.hoveredCardIndex === 1) ? 2 : Theme.hairlineWidth
                    border.color: (root.hoveredCardIndex === 1) ? "#FFFFFF" : Theme.hairline
                    clip: true

                    // Orange radial glow from the bottom
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 1.2
                        height: 60
                        radius: 30
                        opacity: (root.hoveredCardIndex === 1) ? 0.45 : 0.0
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: Theme.accentOrange }
                            GradientStop { position: 1.0; color: "transparent" }
                        }
                        Behavior on opacity { NumberAnimation { duration: 180 } }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 6
                        width: parent.width - 20

                        IconImage {
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "qrc:/resources/icons/shelf.svg"
                            size: 26
                            color: (root.hoveredCardIndex === 1) ? Theme.accentOrange : "#FFFFFF"
                        }

                        Text {
                            text: "Save"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontTitleSize
                            font.weight: Theme.fontTitleWeight
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "To Tray"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
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

            // CARD 3: "To your phone" (One-time LAN link QR)
            Item {
                id: card3
                width: 148
                height: 136
                scale: (root.hoveredCardIndex === 2) ? 1.04 : 1.0
                opacity: (root.hoveredCardIndex !== -1 && root.hoveredCardIndex !== 2) ? 0.40 : 1.0

                Behavior on scale { SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0 } }
                Behavior on opacity { NumberAnimation { duration: 150 } }

                Rectangle {
                    anchors.fill: parent
                    radius: Theme.cardRadius
                    color: Theme.card
                    border.width: (root.hoveredCardIndex === 2) ? 2 : Theme.hairlineWidth
                    border.color: (root.hoveredCardIndex === 2) ? "#FFFFFF" : Theme.hairline
                    clip: true

                    // Orange radial glow from the bottom
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: parent.width * 1.2
                        height: 60
                        radius: 30
                        opacity: (root.hoveredCardIndex === 2) ? 0.45 : 0.0
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: Theme.accentOrange }
                            GradientStop { position: 1.0; color: "transparent" }
                        }
                        Behavior on opacity { NumberAnimation { duration: 180 } }
                    }

                    Column {
                        anchors.centerIn: parent
                        spacing: 6
                        width: parent.width - 20

                        IconImage {
                            anchors.horizontalCenter: parent.horizontalCenter
                            source: "qrc:/resources/icons/phone.svg"
                            size: 26
                            color: (root.hoveredCardIndex === 2) ? Theme.accentOrange : "#FFFFFF"
                        }

                        Text {
                            text: "To your phone"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontTitleSize
                            font.weight: Theme.fontTitleWeight
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            text: "QR download"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: Theme.fontCaptionSize
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

    // Swallow icon shrinking 100% -> 20%
    Rectangle {
        id: swallowGhost
        width: 50
        height: 50
        radius: 12
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
            NumberAnimation { target: swallowGhost; property: "scale"; to: 0.20; duration: 250; easing.type: Easing.InBack }
            NumberAnimation { target: swallowGhost; property: "opacity"; to: 0.0; duration: 220 }
            onFinished: {
                root.isSwallowing = false
            }
        }
    }

    // Card bounce animation
    SequentialAnimation {
        id: cardBounceAnim
        property var targetCard: null

        NumberAnimation { target: cardBounceAnim.targetCard; property: "scale"; to: 1.08; duration: 90; easing.type: Easing.OutQuad }
        SpringAnimation { target: cardBounceAnim.targetCard; property: "scale"; to: 1.0; spring: 4.5; damping: 0.60; duration: 200 }
    }

    Timer {
        id: completionTimer
        interval: 1500 // 1.5s as specified
        repeat: false
        onTriggered: {
            root.completionMessage = ""
            if (appState) appState.collapseToIdle()
        }
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

        if (cardIndex === 0) {
            if (dropManager) dropManager.handleDropMail(dataStr, typeStr, nameStr)
            root.completionMessage = "Draft ready"
            completionTimer.start()
        } else if (cardIndex === 1) {
            if (dropManager) dropManager.handleDropSave(dataStr, typeStr, nameStr)
            root.completionMessage = "Saved"
            completionTimer.start()
        } else if (cardIndex === 2) {
            if (dropManager) dropManager.handleDropPhone(dataStr, typeStr, nameStr)
            // QR view handles itself
        }
    }
}
