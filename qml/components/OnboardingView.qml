import QtQuick
import QtQuick.Controls
import ".."

Item {
    id: root

    property int currentStep: 1
    readonly property int totalSteps: 7

    function saveProgress() {
        if (typeof settings !== "undefined" && settings) {
            settings.save()
        }
    }

    function goNext() {
        saveProgress()
        if (currentStep < totalSteps) {
            currentStep++
        } else {
            finishOnboarding()
        }
    }

    function goBack() {
        saveProgress()
        if (currentStep > 1) {
            currentStep--
        }
    }

    function finishOnboarding() {
        if (typeof settings !== "undefined" && settings) {
            settings.onboardingCompleted = true
            settings.save()
        }
        if (typeof appState !== "undefined" && appState) {
            appState.completeOnboarding()
        }
    }

    // Capture Tab / Enter / Escape navigation
    Keys.onReturnPressed: goNext()
    Keys.onEnterPressed: goNext()
    Keys.onEscapePressed: if (currentStep > 1) goBack()

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 10

        // Header: Step Indicator & Title
        Item {
            width: parent.width
            height: 32

            Row {
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                spacing: 8

                // Tiny pill step indicator
                Rectangle {
                    width: 38
                    height: 18
                    radius: 9
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.10)
                    anchors.verticalCenter: parent.verticalCenter

                    Text {
                        anchors.centerIn: parent
                        text: currentStep + "/" + totalSteps
                        color: Theme.textSecondary
                        font.family: Theme.fontMono
                        font.pixelSize: 10
                        font.bold: true
                    }
                }

                Text {
                    id: headerTitle
                    text: {
                        switch (currentStep) {
                        case 1: return "What should I call you?"
                        case 2: return "Pick your style"
                        case 3: return "Choose your alerts"
                        case 4: return "Your Home"
                        case 5: return "Make it yours"
                        case 6: return "Connect Google"
                        case 7: return "Try it"
                        default: return "Setup"
                        }
                    }
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontTitleSize
                    font.weight: Theme.fontTitleWeight
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        // Center Content Area (Height 142)
        Item {
            width: parent.width
            height: 142
            clip: true

            // ─────────────────────────────────────────────────
            // STEP 1: Name
            // ─────────────────────────────────────────────────
            Item {
                id: step1View
                anchors.fill: parent
                visible: root.currentStep === 1

                Column {
                    anchors.centerIn: parent
                    width: parent.width - 40
                    spacing: 12

                    Text {
                        text: "All greetings and daily summaries will use this name."
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: Theme.fontCaptionSize
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Rectangle {
                        width: parent.width
                        height: 42
                        radius: 12
                        color: Qt.rgba(1.0, 1.0, 1.0, 0.08)
                        border.color: nameInput.activeFocus ? Theme.accent : Theme.hairline
                        border.width: nameInput.activeFocus ? 2 : Theme.hairlineWidth

                        TextInput {
                            id: nameInput
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            verticalAlignment: TextInput.AlignVCenter
                            font.family: Theme.fontFamily
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                            color: Theme.textPrimary
                            focus: root.currentStep === 1
                            text: (settings && settings.displayName) ? settings.displayName : ""
                            onTextChanged: {
                                if (settings) {
                                    settings.displayName = text
                                }
                            }
                            onAccepted: root.goNext()

                            Text {
                                text: "Your Name"
                                visible: !nameInput.text && !nameInput.activeFocus
                                color: Theme.textTertiary
                                font.family: Theme.fontFamily
                                font.pixelSize: 15
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // STEP 2: Pick your style (MacBook Notch vs iPhone Floating)
            // ─────────────────────────────────────────────────
            Item {
                id: step2View
                anchors.fill: parent
                visible: root.currentStep === 2

                Row {
                    anchors.centerIn: parent
                    spacing: 16

                    // Style A: Notch
                    Rectangle {
                        width: 190
                        height: 120
                        radius: 18
                        color: (settings && settings.islandStyle === "notch")
                               ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.20)
                               : Qt.rgba(1.0, 1.0, 1.0, 0.06)
                        border.color: (settings && settings.islandStyle === "notch") ? Theme.accent : Theme.hairline
                        border.width: (settings && settings.islandStyle === "notch") ? 2 : 1

                        Column {
                            anchors.centerIn: parent
                            spacing: 8

                            // Notch preview mockup
                            Rectangle {
                                width: 70
                                height: 16
                                radius: 8
                                color: "#000000"
                                border.color: Theme.hairline
                                border.width: 1
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: "Notch (MacBook)"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 13
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: "Flush with top screen edge"
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (settings) {
                                    settings.islandStyle = "notch"
                                    root.saveProgress()
                                }
                            }
                        }
                    }

                    // Style B: Floating
                    Rectangle {
                        width: 190
                        height: 120
                        radius: 18
                        color: (settings && settings.islandStyle === "floating")
                               ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.20)
                               : Qt.rgba(1.0, 1.0, 1.0, 0.06)
                        border.color: (settings && settings.islandStyle === "floating") ? Theme.accent : Theme.hairline
                        border.width: (settings && settings.islandStyle === "floating") ? 2 : 1

                        Column {
                            anchors.centerIn: parent
                            spacing: 8

                            // Floating pill preview mockup
                            Rectangle {
                                width: 70
                                height: 16
                                radius: 8
                                color: "#000000"
                                border.color: Theme.hairline
                                border.width: 1
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: "Floating (iPhone)"
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 13
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }

                            Text {
                                text: "Floats below bezel with gap"
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                if (settings) {
                                    settings.islandStyle = "floating"
                                    root.saveProgress()
                                }
                            }
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // STEP 3: Alerts toggles
            // ─────────────────────────────────────────────────
            Item {
                id: step3View
                anchors.fill: parent
                visible: root.currentStep === 3

                Grid {
                    anchors.centerIn: parent
                    columns: 2
                    columnSpacing: 12
                    rowSpacing: 6

                    readonly property var alertItems: [
                        { id: "battery", label: "Charging & Battery", icon: "qrc:/resources/icons/battery.svg" },
                        { id: "screenshots", label: "Screenshots", icon: "qrc:/resources/icons/media.svg" },
                        { id: "downloads", label: "Downloads", icon: "qrc:/resources/icons/shelf.svg" },
                        { id: "timer", label: "Timer & Focus", icon: "qrc:/resources/icons/timer.svg" },
                        { id: "gmail", label: "Gmail", icon: "qrc:/resources/icons/mail.svg" },
                        { id: "agent", label: "AI Agent", icon: "qrc:/resources/icons/sparkles.svg" },
                        { id: "usb", label: "USB & Devices", icon: "qrc:/resources/icons/phone.svg" },
                        { id: "volume", label: "Volume HUD", icon: "qrc:/resources/icons/volume.svg" }
                    ]

                    Repeater {
                        model: parent.alertItems

                        Rectangle {
                            width: 195
                            height: 28
                            radius: 8
                            color: isEnabled(modelData.id) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16) : Qt.rgba(1.0, 1.0, 1.0, 0.05)
                            border.color: isEnabled(modelData.id) ? Theme.accent : Theme.hairline
                            border.width: 1

                            function isEnabled(id) {
                                if (!settings) return true
                                switch(id) {
                                case "battery": return settings.alertBattery
                                case "screenshots": return settings.alertScreenshots
                                case "downloads": return settings.alertDownloads
                                case "timer": return settings.alertTimer
                                case "gmail": return settings.alertGmail
                                case "agent": return settings.alertAgent
                                case "usb": return settings.alertUsb
                                case "volume": return settings.alertVolume
                                default: return true
                                }
                            }

                            function toggle(id) {
                                if (!settings) return
                                switch(id) {
                                case "battery": settings.alertBattery = !settings.alertBattery; break
                                case "screenshots": settings.alertScreenshots = !settings.alertScreenshots; break
                                case "downloads": settings.alertDownloads = !settings.alertDownloads; break
                                case "timer": settings.alertTimer = !settings.alertTimer; break
                                case "gmail": settings.alertGmail = !settings.alertGmail; break
                                case "agent": settings.alertAgent = !settings.alertAgent; break
                                case "usb": settings.alertUsb = !settings.alertUsb; break
                                case "volume": settings.alertVolume = !settings.alertVolume; break
                                }
                                root.saveProgress()
                            }

                            Row {
                                anchors.fill: parent
                                anchors.leftMargin: 8
                                anchors.rightMargin: 8
                                spacing: 8

                                IconImage {
                                    source: modelData.icon
                                    size: 13
                                    color: parent.parent.isEnabled(modelData.id) ? Theme.accent : Theme.textTertiary
                                    anchors.verticalCenter: parent.verticalCenter
                                }

                                Text {
                                    text: modelData.label
                                    color: Theme.textPrimary
                                    font.family: Theme.fontFamily
                                    font.pixelSize: 11
                                    font.weight: Font.Medium
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: 120
                                    elide: Text.ElideRight
                                }

                                Text {
                                    text: parent.parent.isEnabled(modelData.id) ? "ON" : "OFF"
                                    color: parent.parent.isEnabled(modelData.id) ? Theme.accentGreen : Theme.textTertiary
                                    font.family: Theme.fontMono
                                    font.pixelSize: 10
                                    font.bold: true
                                    anchors.verticalCenter: parent.verticalCenter
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: parent.toggle(modelData.id)
                            }
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // STEP 4: Your Home Chips (Tray, Clips, Mail, Next, Timer, Media - max 4)
            // ─────────────────────────────────────────────────
            Item {
                id: step4View
                anchors.fill: parent
                visible: root.currentStep === 4

                Column {
                    anchors.centerIn: parent
                    spacing: 8

                    Text {
                        text: "Choose up to 4 quick chips for your Home tab:"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Grid {
                        columns: 3
                        columnSpacing: 10
                        rowSpacing: 8
                        anchors.horizontalCenter: parent.horizontalCenter

                        readonly property var chipsModel: [
                            { id: "tray", label: "Tray", icon: "qrc:/resources/icons/shelf.svg" },
                            { id: "clips", label: "Clips", icon: "qrc:/resources/icons/clipboard.svg" },
                            { id: "mail", label: "Mail", icon: "qrc:/resources/icons/mail.svg" },
                            { id: "next", label: "Next event", icon: "qrc:/resources/icons/calendar.svg" },
                            { id: "pomodoro", label: "Focus timer", icon: "qrc:/resources/icons/timer.svg" },
                            { id: "media", label: "Now playing", icon: "qrc:/resources/icons/media.svg" }
                        ]

                        Repeater {
                            model: parent.chipsModel

                            Rectangle {
                                width: 120
                                height: 38
                                radius: 10
                                color: (settings && settings.isHomeChipActive(modelData.id))
                                       ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.20)
                                       : Qt.rgba(1.0, 1.0, 1.0, 0.06)
                                border.color: (settings && settings.isHomeChipActive(modelData.id)) ? Theme.accent : Theme.hairline
                                border.width: 1

                                Row {
                                    anchors.centerIn: parent
                                    spacing: 6

                                    IconImage {
                                        source: modelData.icon
                                        size: 14
                                        color: (settings && settings.isHomeChipActive(modelData.id)) ? Theme.accent : Theme.textSecondary
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Text {
                                        text: modelData.label
                                        color: Theme.textPrimary
                                        font.family: Theme.fontFamily
                                        font.pixelSize: 11
                                        font.bold: true
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (settings) {
                                            settings.toggleHomeChip(modelData.id)
                                            root.saveProgress()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // STEP 5: Make it yours (Accent, Mascot, Hotkey, Windows Start, Clipboard)
            // ─────────────────────────────────────────────────
            Item {
                id: step5View
                anchors.fill: parent
                visible: root.currentStep === 5

                Column {
                    anchors.fill: parent
                    spacing: 8

                    // Accent color dots
                    Row {
                        spacing: 10
                        anchors.horizontalCenter: parent.horizontalCenter

                        Repeater {
                            model: ["#0A84FF", "#BF5AF2", "#30D158", "#FF9F0A", "#FF375F", "#64D2FF"]

                            Rectangle {
                                width: 22
                                height: 22
                                radius: 11
                                color: modelData
                                border.color: (settings && settings.accentColor === modelData) ? "#FFFFFF" : "transparent"
                                border.width: 2

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (settings) {
                                            settings.accentColor = modelData
                                            root.saveProgress()
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Toggles row: Mascot & Start with Windows & Clipboard
                    Row {
                        spacing: 8
                        anchors.horizontalCenter: parent.horizontalCenter

                        // Mascot toggle
                        Rectangle {
                            width: 125
                            height: 32
                            radius: 8
                            color: (settings && settings.mascotEyesEnabled) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16) : Qt.rgba(1.0, 1.0, 1.0, 0.06)
                            border.color: (settings && settings.mascotEyesEnabled) ? Theme.accent : Theme.hairline
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "Eyes Mascot: " + ((settings && settings.mascotEyesEnabled) ? "ON" : "OFF")
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (settings) {
                                        settings.mascotEyesEnabled = !settings.mascotEyesEnabled
                                        root.saveProgress()
                                    }
                                }
                            }
                        }

                        // Start with Windows toggle
                        Rectangle {
                            width: 135
                            height: 32
                            radius: 8
                            color: (settings && settings.autoStart) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.16) : Qt.rgba(1.0, 1.0, 1.0, 0.06)
                            border.color: (settings && settings.autoStart) ? Theme.accent : Theme.hairline
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "Auto-Start: " + ((settings && settings.autoStart) ? "ON" : "OFF")
                                color: Theme.textPrimary
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    if (settings) {
                                        settings.autoStart = !settings.autoStart
                                        root.saveProgress()
                                    }
                                }
                            }
                        }

                        // Hotkey pill
                        Rectangle {
                            width: 135
                            height: 32
                            radius: 8
                            color: Qt.rgba(1.0, 1.0, 1.0, 0.06)
                            border.color: Theme.hairline
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: (settings && settings.hotkey) ? settings.hotkey : "Ctrl+Alt+Space"
                                color: Theme.textPrimary
                                font.family: Theme.fontMono
                                font.pixelSize: 11
                            }
                        }
                    }

                    // Clipboard privacy notice
                    Rectangle {
                        width: parent.width - 20
                        height: 48
                        radius: 8
                        color: Qt.rgba(1.0, 1.0, 1.0, 0.04)
                        border.color: Theme.hairline
                        border.width: 1
                        anchors.horizontalCenter: parent.horizontalCenter

                        Column {
                            anchors.centerIn: parent
                            spacing: 2

                            Text {
                                text: "Clipboard History: ON"
                                color: Theme.accentGreen
                                font.family: Theme.fontFamily
                                font.pixelSize: 11
                                font.bold: true
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            Text {
                                text: "Passwords & secrets are skipped. Everything stays 100% on this PC."
                                color: Theme.textSecondary
                                font.family: Theme.fontFamily
                                font.pixelSize: 10
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // STEP 6: Connect Google (optional)
            // ─────────────────────────────────────────────────
            Item {
                id: step6View
                anchors.fill: parent
                visible: root.currentStep === 6

                Column {
                    anchors.centerIn: parent
                    spacing: 12
                    width: parent.width - 40

                    Text {
                        text: "Connect Google for live unread summaries and calendar countdowns."
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: "Read-only access. Local OAuth tokens encrypted with Windows DPAPI."
                        color: Theme.textTertiary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Row {
                        spacing: 14
                        anchors.horizontalCenter: parent.horizontalCenter

                        GlassButton {
                            text: "Connect Google"
                            isAccent: true
                            implicitHeight: 34
                            implicitWidth: 140
                            cornerRadius: 17
                            onClicked: {
                                if (typeof gmailService !== "undefined" && gmailService) {
                                    gmailService.startAuth()
                                }
                                root.goNext()
                            }
                        }

                        GlassButton {
                            text: "Skip for now"
                            isAccent: false
                            implicitHeight: 34
                            implicitWidth: 120
                            cornerRadius: 17
                            onClicked: root.goNext()
                        }
                    }
                }
            }

            // ─────────────────────────────────────────────────
            // STEP 7: Try it (Drag a file to top of screen)
            // ─────────────────────────────────────────────────
            Item {
                id: step7View
                anchors.fill: parent
                visible: root.currentStep === 7

                property bool testSwallowed: false

                Column {
                    anchors.centerIn: parent
                    spacing: 10
                    width: parent.width - 40

                    IconImage {
                        source: step7View.testSwallowed ? "qrc:/resources/icons/check.svg" : "qrc:/resources/icons/drop.svg"
                        size: 32
                        color: step7View.testSwallowed ? Theme.accentGreen : Theme.accentBlue
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: step7View.testSwallowed
                              ? "You're all set!"
                              : "Drag a file to the top of your screen"
                        color: Theme.textPrimary
                        font.family: Theme.fontFamily
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Text {
                        text: step7View.testSwallowed
                              ? "Aura Island is ready. Hover near the top to expand anytime."
                              : "The island will swallow the file into your Shelf, Share, or Notes."
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignHCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    GlassButton {
                        text: step7View.testSwallowed ? "Finish" : "Complete Setup"
                        isAccent: true
                        implicitHeight: 34
                        implicitWidth: 150
                        cornerRadius: 17
                        anchors.horizontalCenter: parent.horizontalCenter
                        onClicked: {
                            step7View.testSwallowed = true
                            root.finishOnboarding()
                        }
                    }
                }
            }
        }

        // ─────────────────────────────────────────────────────
        // Bottom Navigation Bar (Back & Continue)
        // ─────────────────────────────────────────────────────
        Item {
            width: parent.width
            height: 36

            GlassButton {
                text: "Back"
                visible: root.currentStep > 1 && root.currentStep < 7
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                implicitHeight: 30
                implicitWidth: 70
                cornerRadius: 15
                onClicked: root.goBack()
            }

            GlassButton {
                text: "Continue"
                visible: root.currentStep < 7
                isAccent: true
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                implicitHeight: 32
                implicitWidth: 110
                cornerRadius: 16
                onClicked: root.goNext()
            }
        }
    }
}
