import QtQuick
import QtQuick.Controls
import "components"
import "tabs"
import "."

Item {
    id: root

    property string stateName: appState ? appState.islandState : "idle"
    property bool isHovered: false

    // ─────────────────────────────────────────────────────────
    // Target dimensions per state
    // ─────────────────────────────────────────────────────────
    readonly property int targetWidth: {
        if (stateName === "idle")     return (root.isHovered ? 134 : 120)
        if (stateName === "compact")  return (appState && appState.isMusicMorphActive) ? 280 : 260
        if (stateName === "expanded") return 480
        if (stateName === "drop")     return 480
        return 120
    }

    readonly property int targetHeight: {
        if (stateName === "idle")     return 32
        if (stateName === "compact")  return 36
        if (stateName === "expanded") return (appState && appState.currentTab === "home") ? 270 : 300
        if (stateName === "drop")     return 200
        return 32
    }

    readonly property int targetRadius: {
        if (stateName === "idle")     return 16
        if (stateName === "compact")  return 18
        if (stateName === "expanded") return 36
        if (stateName === "drop")     return 24
        return 16
    }

    // ─────────────────────────────────────────────────────────
    // Elastic Spring Dimensions based on Island State
    // ─────────────────────────────────────────────────────────
    width:  targetWidth
    height: targetHeight

    Behavior on width {
        NumberAnimation {
            duration: (root.stateName === "idle") ? 270 : 340
            easing.type: (root.stateName === "idle") ? Easing.OutCubic : Easing.OutBack
            easing.overshoot: 1.22
        }
    }
    Behavior on height {
        NumberAnimation {
            duration: (root.stateName === "idle") ? 250 : 320
            easing.type: (root.stateName === "idle") ? Easing.OutCubic : Easing.OutBack
            easing.overshoot: 1.15
        }
    }

    // ─────────────────────────────────────────────────────────
    // Glass Pill Background
    // ─────────────────────────────────────────────────────────
    GlassPill {
        id: pillBg
        anchors.fill: parent
        cornerRadius: root.targetRadius
        activeGlow: root.stateName === "drop"

        Behavior on cornerRadius {
            NumberAnimation {
                duration: (root.stateName === "idle") ? 250 : 320
                easing.type: Easing.OutCubic
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // Hover-to-expand and Leave-to-collapse Controllers
    // ─────────────────────────────────────────────────────────
    MouseArea {
        id: islandHoverSensor
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton

        onEntered: {
            collapseGraceTimer.stop()
            if (root.stateName === "idle") {
                hoverExpandTimer.start()
            }
        }

        onExited: {
            hoverExpandTimer.stop()
            if (root.stateName === "expanded" && (!appState || !appState.isPinned)) {
                collapseGraceTimer.start()
            }
        }
    }

    Timer {
        id: hoverExpandTimer
        interval: 180
        repeat: false
        onTriggered: {
            if (root.stateName === "idle") {
                if (appState) appState.setIslandState("expanded")
            }
        }
    }

    Timer {
        id: collapseGraceTimer
        interval: 400
        repeat: false
        onTriggered: {
            if (root.stateName === "expanded" && (!appState || !appState.isPinned)) {
                if (appState) appState.collapseToIdle()
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // Ambient breathing ring for Focus mode
    // ─────────────────────────────────────────────────────────
    Rectangle {
        id: focusBreathingHalo
        anchors.fill: parent
        anchors.margins: -4
        radius: root.targetRadius + 4
        color: "transparent"
        border.color: (Theme && Theme.accent) ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.45) : "#660A84FF"
        border.width: 1.5
        visible: appState && appState.isFocusModeActive

        SequentialAnimation on opacity {
            running: appState && appState.isFocusModeActive
            loops: Animation.Infinite
            NumberAnimation { to: 0.15; duration: 1600; easing.type: Easing.InOutSine }
            NumberAnimation { to: 0.85; duration: 1600; easing.type: Easing.InOutSine }
        }
    }

    // ─────────────────────────────────────────────────────────
    // Pulsing outer ring for drop zone
    // ─────────────────────────────────────────────────────────
    Rectangle {
        id: dropPulseRing
        anchors.fill: parent
        anchors.margins: -6
        radius: root.targetRadius + 6
        color: "transparent"
        border.color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.5)
        border.width: 1.5
        visible: root.stateName === "drop"

        SequentialAnimation on opacity {
            running: root.stateName === "drop"
            loops:   Animation.Infinite
            NumberAnimation { to: 0.15; duration: 750 }
            NumberAnimation { to: 1.0;  duration: 750 }
        }
    }

    // ─────────────────────────────────────────────────────────
    // 1. IDLE STATE (~120x32) — NEVER BLANK!
    // ─────────────────────────────────────────────────────────
    Item {
        id: idleView
        anchors.fill: parent
        opacity: (root.stateName === "idle") ? 1.0 : 0.0
        visible: opacity > 0.01
        scale:   (root.stateName === "idle") ? 1.0 : 0.85

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.0; damping: 0.60; epsilon: 0.001 }
        }

        Row {
            anchors.centerIn: parent
            spacing: 6

            // Dynamic Widget Icon
            IconImage {
                anchors.verticalCenter: parent.verticalCenter
                size: 13
                color: Theme.accent
                source: {
                    var w = settings ? settings.idleWidget : "clipboard"
                    if (w === "media") return "qrc:/resources/icons/media.svg"
                    if (w === "git") return "qrc:/resources/icons/git-branch.svg"
                    if (w === "pomodoro") return "qrc:/resources/icons/timer.svg"
                    if (w === "mail") return "qrc:/resources/icons/mail.svg"
                    if (w === "agent") return "qrc:/resources/icons/dev.svg"
                    return "qrc:/resources/icons/clipboard.svg"
                }
            }

            // Dynamic Widget Text / Value
            Text {
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.textPrimary
                font.family: (settings && (settings.idleWidget === "pomodoro" || settings.idleWidget === "clipboard")) ? Theme.fontMono : Theme.fontFamily
                font.pixelSize: 11
                font.weight: Font.DemiBold
                elide: Text.ElideRight
                maximumLineCount: 1
                text: {
                    var w = settings ? settings.idleWidget : "clipboard"
                    if (w === "media") {
                        return (appState && appState.nowPlayingTitle !== "") ? appState.nowPlayingTitle : "Music"
                    }
                    if (w === "git") {
                        return (appState && appState.gitBranch !== "") ? appState.gitBranch : "main"
                    }
                    if (w === "pomodoro") {
                        return appState ? appState.pomodoroDisplay : "25:00"
                    }
                    if (w === "mail") {
                        return (appState && appState.unreadMailCount > 0) ? (appState.unreadMailCount + " unread") : "0 unread"
                    }
                    if (w === "agent") {
                        return appState ? appState.agentStatus : "idle"
                    }
                    // Default: clipboard count
                    var count = clipboardManager ? clipboardManager.itemCount : 0
                    return count + (count === 1 ? " clip" : " clips")
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onEntered:      root.isHovered = true
            onExited:       root.isHovered = false
            onClicked:      if (appState) appState.toggleExpanded()
            onDoubleClicked: if (appState) appState.toggleExpanded()
        }
    }

    // ─────────────────────────────────────────────────────────
    // 2. COMPACT STATE (~260x36)
    // ─────────────────────────────────────────────────────────
    Item {
        id: compactView
        anchors.fill: parent
        anchors.margins: 4
        opacity: (root.stateName === "compact") ? 1.0 : 0.0
        visible: opacity > 0.01
        scale:   (root.stateName === "compact") ? 1.0 : 0.88

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.5; damping: 0.65; epsilon: 0.001 }
        }

        // Mode A: Quick Note Capture Input
        Row {
            visible: appState && appState.isQuickCaptureActive
            anchors.fill: parent
            anchors.leftMargin: 6
            anchors.rightMargin: 6
            spacing: 6

            IconImage {
                source: "qrc:/resources/icons/notes.svg"
                size: 14
                color: Theme.accent
                anchors.verticalCenter: parent.verticalCenter
            }

            TextInput {
                id: quickNoteInput
                width: parent.width - 24
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 11
                selectByMouse: true
                focus: appState && appState.isQuickCaptureActive

                Text {
                    text: "Quick note... (Enter saves, Esc exits)"
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    visible: !quickNoteInput.text && !quickNoteInput.activeFocus
                    anchors.verticalCenter: parent.verticalCenter
                }

                Keys.onReturnPressed: {
                    if (notesManager && text.trim() !== "") {
                        notesManager.quickCapture(text)
                        text = ""
                    }
                    if (appState) appState.closeQuickCapture()
                }

                Keys.onEscapePressed: {
                    text = ""
                    if (appState) appState.closeQuickCapture()
                }
            }
        }

        // Mode B1: Sleek Music Morph Notification (When any track starts / changes)
        Row {
            visible: appState && !appState.isQuickCaptureActive && appState.isMusicMorphActive
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 8

            Rectangle {
                width: 22
                height: 22
                radius: 11
                color: Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.20)
                anchors.verticalCenter: parent.verticalCenter

                IconImage {
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/media.svg"
                    size: 12
                    color: Theme.accent
                }
            }

            Column {
                width: parent.width - 66
                anchors.verticalCenter: parent.verticalCenter
                spacing: 1

                Text {
                    width: parent.width
                    text: appState ? appState.nowPlayingTitle : "Music"
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Text {
                    width: parent.width
                    text: appState ? appState.nowPlayingArtist : ""
                    color: Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 9
                    elide: Text.ElideRight
                }
            }

            EqualizerBars {
                isPlaying: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Mode B2: System Event Message (Volume, Brightness, Touchpad, Power, Battery)
        Row {
            visible: appState && !appState.isQuickCaptureActive && !appState.isMusicMorphActive && appState.compactEventMessage !== ""
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 8

            Rectangle {
                width: 20
                height: 20
                radius: 10
                color: Qt.rgba(1.0, 1.0, 1.0, 0.12)
                anchors.verticalCenter: parent.verticalCenter

                IconImage {
                    anchors.centerIn: parent
                    size: 11
                    color: Theme.accent
                    source: {
                        var msg = appState ? appState.compactEventMessage : ""
                        if (msg.indexOf("Volume") !== -1) return "qrc:/resources/icons/volume.svg"
                        if (msg.indexOf("Mute") !== -1) return "qrc:/resources/icons/volume-mute.svg"
                        if (msg.indexOf("Brightness") !== -1) return "qrc:/resources/icons/sun.svg"
                        if (msg.indexOf("Touchpad") !== -1) return "qrc:/resources/icons/touchpad.svg"
                        if (msg.indexOf("Battery") !== -1 || msg.indexOf("Charging") !== -1) return "qrc:/resources/icons/battery.svg"
                        if (msg.indexOf("Lock") !== -1) return "qrc:/resources/icons/lock.svg"
                        if (msg.indexOf("Sleep") !== -1) return "qrc:/resources/icons/sleep.svg"
                        if (msg.indexOf("Restart") !== -1) return "qrc:/resources/icons/restart.svg"
                        if (msg.indexOf("Shut") !== -1) return "qrc:/resources/icons/power.svg"
                        return "qrc:/resources/icons/check.svg"
                    }
                }
            }

            Text {
                width: parent.width - 32
                anchors.verticalCenter: parent.verticalCenter
                text: appState ? appState.compactEventMessage : ""
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 11
                font.weight: Font.Medium
                elide: Text.ElideRight
            }
        }

        // Mode C: Standard Compact Status & Media / Focus
        Row {
            visible: appState && !appState.isQuickCaptureActive && appState.compactEventMessage === ""
            anchors.fill: parent
            spacing: 8

            // Left: icon + badge
            Row {
                anchors.verticalCenter: parent.verticalCenter
                spacing: 4

                IconImage {
                    source: (appState && appState.isPlaying)
                             ? "qrc:/resources/icons/media.svg"
                             : "qrc:/resources/icons/mail.svg"
                    size: 14
                    color: Theme.accent
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    visible: appState && appState.unreadMailCount > 0
                    text: appState ? appState.unreadMailCount : ""
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 10
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            // Centre: now-playing / focus timer / name
            Item {
                width: parent.width - 90
                height: parent.height

                Text {
                    id: compactLabel
                    anchors.centerIn: parent
                    text: {
                        if (appState && appState.isPlaying && appState.nowPlayingTitle !== "")
                            return appState.nowPlayingTitle
                        if (appState && appState.pomodoroDisplay !== "")
                            return "Focus: " + appState.pomodoroDisplay
                        return "Aura Island"
                    }
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    width: parent.width
                    horizontalAlignment: Text.AlignHCenter
                }
            }

            // Right: equalizer bars + expand button
            Row {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                EqualizerBars {
                    visible: appState && appState.isPlaying
                    isPlaying: appState ? appState.isPlaying : false
                    anchors.verticalCenter: parent.verticalCenter
                }

                Rectangle {
                    width: 18; height: 18; radius: 9
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.10)
                    anchors.verticalCenter: parent.verticalCenter

                    IconImage {
                        anchors.centerIn: parent
                        source: "qrc:/resources/icons/dev.svg"
                        size: 9
                        color: Theme.textPrimary
                    }
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            enabled: appState && !appState.isQuickCaptureActive
            onClicked: if (appState) appState.toggleExpanded()
        }
    }

    // ─────────────────────────────────────────────────────────
    // 3. EXPANDED STATE (~560x340)
    // ─────────────────────────────────────────────────────────
    Item {
        id: expandedView
        anchors.fill: parent
        anchors.margins: 10
        opacity: (root.stateName === "expanded") ? 1.0 : 0.0
        visible: opacity > 0.01

        property real contentScale: (root.stateName === "expanded") ? 1.0 : 0.93

        Behavior on opacity {
            NumberAnimation {
                duration: Theme.animDurationNormal
                easing.type: Easing.OutCubic
            }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            onPositionChanged: function(mouse) {
                if (appState) {
                    appState.updateMascotTracking((mouse.x - width / 2) / 35, (mouse.y - height / 2) / 25)
                }
            }
        }

        Item {
            id: expandedContent
            anchors.fill: parent
            scale: expandedView.contentScale
            transformOrigin: Item.Top

            Behavior on scale {
                SpringAnimation { spring: 3.5; damping: 0.60; epsilon: 0.001 }
            }

            // 1. Top Status Row (Fixed at Top)
            StatusRow {
                id: statusRow
                anchors.top: parent.top
                anchors.topMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 6
                anchors.right: parent.right
                anchors.rightMargin: 6
                height: 24
            }

            // 2. Bottom Dock (Fixed at Bottom)
            SlidingTabBar {
                id: bottomDock
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 2
                anchors.left: parent.left
                anchors.leftMargin: 4
                anchors.right: parent.right
                anchors.rightMargin: 4
                height: 36
                activeTab: appState ? appState.currentTab : "home"
                onTabSelected: function(tabId) {
                    if (appState) appState.setCurrentTab(tabId)
                }
            }

            // 3. Tab Content Area (Stretches perfectly between StatusRow and Dock!)
            Rectangle {
                id: contentArea
                anchors.top: statusRow.bottom
                anchors.topMargin: 6
                anchors.bottom: bottomDock.top
                anchors.bottomMargin: 6
                anchors.left: parent.left
                anchors.right: parent.right
                radius: 16
                color: Qt.rgba(0.04, 0.04, 0.06, 0.55)
                border.color: Qt.rgba(1.0, 1.0, 1.0, 0.08)
                clip: true

                HomeTab      { anchors.fill: parent; anchors.margins: 4; visible: !appState || appState.currentTab === "home" }
                ShelfTab     { anchors.fill: parent; visible: appState && appState.currentTab === "shelf" }
                MailTab      { anchors.fill: parent; visible: appState && appState.currentTab === "mail" }
                ClipboardTab { anchors.fill: parent; visible: appState && appState.currentTab === "clipboard" }
                NotesTab     { anchors.fill: parent; visible: appState && appState.currentTab === "notes" }
                DevTab       { anchors.fill: parent; visible: appState && appState.currentTab === "dev" }
                MediaTab     { anchors.fill: parent; visible: appState && appState.currentTab === "media" }
                AITab        { anchors.fill: parent; visible: appState && appState.currentTab === "ai" }
                GamesTab     { anchors.fill: parent; visible: appState && appState.currentTab === "games" }
                FocusTab     { anchors.fill: parent; visible: appState && appState.currentTab === "focus" }
                SettingsTab  { anchors.fill: parent; visible: appState && appState.currentTab === "settings" }

                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 1
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.07)
                }
            }
        }

        // Toast notifications
        ToastNotification {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 8
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }

    // ─────────────────────────────────────────────────────────
    // 4. DROP STATE (~400x140)
    // ─────────────────────────────────────────────────────────
    Item {
        id: dropView
        anchors.fill: parent
        opacity: (root.stateName === "drop") ? 1.0 : 0.0
        visible: opacity > 0.01
        scale:   (root.stateName === "drop") ? 1.0 : 0.90

        Behavior on opacity {
            NumberAnimation { duration: Theme.animDurationFast; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.0; damping: 0.65; epsilon: 0.001 }
        }

        DropZone {
            anchors.fill: parent
        }
    }
}
