import QtQuick
import QtQuick.Controls
import "components"
import "tabs"
import "."

Item {
    id: root

    property string stateName: appState ? appState.islandState : "idle"
    property bool isExpanding: (stateName !== "idle")
    readonly property bool reduceAnim: (typeof settings !== "undefined" && settings && settings.reduceAnimations)

    // ─────────────────────────────────────────────────────────
    // Target dimensions per state (Spec Section 2)
    // - Idle: 148x34 (radius 17)
    // - Live: 220-280x34 (radius 17)
    // - Alert: 360x76 (radius 28)
    // - Home: 480x252 (radius 34)
    // - Toggles: 480x252 (radius 34)
    // - Tray/Clipboard/Notes: 480x300 (radius 34)
    // - Drop: 480x200 (radius 34)
    // ─────────────────────────────────────────────────────────
    readonly property int targetWidth: {
        if (stateName === "idle")     return 148
        if (stateName === "live" || stateName === "compact") return (appState && appState.isMusicMorphActive) ? 280 : 260
        if (stateName === "alert")    return 360
        if (stateName === "drop")     return 480
        if (stateName === "onboarding") return 480
        if (stateName === "expanded") return 480
        return 148
    }

    readonly property int targetHeight: {
        if (stateName === "idle")     return 34
        if (stateName === "live" || stateName === "compact") return 34
        if (stateName === "alert")    return 76
        if (stateName === "drop")     return 200
        if (stateName === "onboarding") return 252
        if (stateName === "expanded") {
            var tab = appState ? appState.currentTab : "home"
            if (tab === "home" || tab === "toggles") return 252
            return 300 // Tray, Clipboard, Notes, etc.
        }
        return 34
    }

    readonly property int targetRadius: {
        if (stateName === "idle")     return 17
        if (stateName === "live" || stateName === "compact") return 17
        if (stateName === "alert")    return 28
        return 34 // Home, Toggles, Tray, Clipboard, Notes, Drop
    }

    // ─────────────────────────────────────────────────────────
    // Section 3: Motion - Unified Spring
    // Stiffness 250, Damping 24, Mass 1 on expand
    // Stiffness 320, Damping 30, Mass 1 on collapse
    // ─────────────────────────────────────────────────────────
    width: targetWidth
    height: targetHeight

    property int currentRadius: targetRadius

    Behavior on width {
        enabled: !root.reduceAnim
        SpringAnimation {
            spring: root.isExpanding ? 5.0 : 6.4
            damping: root.isExpanding ? 0.60 : 0.68
            mass: 1.0
            epsilon: 0.25
        }
    }

    Behavior on height {
        enabled: !root.reduceAnim
        SpringAnimation {
            spring: root.isExpanding ? 5.0 : 6.4
            damping: root.isExpanding ? 0.60 : 0.68
            mass: 1.0
            epsilon: 0.25
        }
    }

    Behavior on currentRadius {
        enabled: !root.reduceAnim
        SpringAnimation {
            spring: root.isExpanding ? 5.0 : 6.4
            damping: root.isExpanding ? 0.60 : 0.68
            mass: 1.0
            epsilon: 0.25
        }
    }

    // ─────────────────────────────────────────────────────────
    // Pure Black Shell Background (NO glow, NO transparency, NO borders)
    // ─────────────────────────────────────────────────────────
    ShellBackground {
        id: shellBg
        anchors.fill: parent
        cornerRadius: root.currentRadius
        bottomRadius: root.currentRadius
    }

    // ─────────────────────────────────────────────────────────
    // Expand / Collapse Content Lifecycle
    // Content starts 80ms after shape begins growing.
    // Content fades out in 60ms on collapse.
    // ─────────────────────────────────────────────────────────
    property bool contentReady: false

    Timer {
        id: expandContentTimer
        interval: Theme.contentExpandDelay // 80ms
        repeat: false
        onTriggered: {
            root.contentReady = true
        }
    }

    onStateNameChanged: {
        if (stateName !== "idle") {
            root.contentReady = false
            expandContentTimer.start()
        } else {
            expandContentTimer.stop()
            root.contentReady = false
        }
    }

    // ─────────────────────────────────────────────────────────
    // 1. IDLE STATE (148x34) — Pure black, calm, clean, empty!
    // ─────────────────────────────────────────────────────────
    Item {
        id: idleView
        anchors.fill: parent
        visible: (root.stateName === "idle")
        // Entirely empty as specified in acceptance checklist:
        // "Idle looks like a notch: solid black, flush with the top, no glow, no border, empty."
    }

    // ─────────────────────────────────────────────────────────
    // 2. LIVE STATE (220-280x34, Live activities / compact ongoing)
    // ─────────────────────────────────────────────────────────
    Item {
        id: liveView
        anchors.fill: parent
        anchors.margins: 4
        visible: (root.stateName === "live" || root.stateName === "compact")
        opacity: (visible && root.contentReady) ? 1.0 : 0.0
        scale: (visible && root.contentReady) ? 1.0 : 0.92

        Behavior on opacity {
            NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0; epsilon: 0.001 }
        }

        // Live Mode A: Music Morph Notification
        Row {
            visible: appState && appState.isMusicMorphActive
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 8

            Rectangle {
                width: 22
                height: 22
                radius: 11
                color: Qt.rgba(Theme.accentPink.r, Theme.accentPink.g, Theme.accentPink.b, 0.20)
                anchors.verticalCenter: parent.verticalCenter

                IconImage {
                    anchors.centerIn: parent
                    source: "qrc:/resources/icons/media.svg"
                    size: 12
                    color: Theme.accentPink
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
                    font.pixelSize: Theme.fontCaptionSize
                    font.weight: Theme.fontTitleWeight
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

        // Live Mode B: System Event Message (Volume, Battery, Brightness)
        Row {
            visible: appState && !appState.isMusicMorphActive && appState.compactEventMessage !== ""
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
                    size: 12
                    color: Theme.accentBlue
                    source: {
                        var msg = appState ? appState.compactEventMessage : ""
                        if (msg.indexOf("Volume") !== -1) return "qrc:/resources/icons/volume.svg"
                        if (msg.indexOf("Mute") !== -1) return "qrc:/resources/icons/volume-mute.svg"
                        if (msg.indexOf("Battery") !== -1 || msg.indexOf("Charging") !== -1) return "qrc:/resources/icons/battery.svg"
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
                font.pixelSize: Theme.fontCaptionSize
                font.weight: Theme.fontBodyWeight
                elide: Text.ElideRight
            }
        }

        // Live Mode C: Ongoing Status (Music playing or Pomodoro running)
        Item {
            visible: appState && !appState.isMusicMorphActive && appState.compactEventMessage === ""
            anchors.fill: parent

            Row {
                anchors.left: parent.left
                anchors.leftMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                IconImage {
                    source: (appState && appState.isPlaying) ? "qrc:/resources/icons/media.svg" : "qrc:/resources/icons/timer.svg"
                    size: 14
                    color: (appState && appState.isPlaying) ? Theme.accentPink : Theme.accentOrange
                    anchors.verticalCenter: parent.verticalCenter
                }

                Text {
                    text: {
                        if (appState && appState.isPlaying && appState.nowPlayingTitle !== "")
                            return appState.nowPlayingTitle
                        if (appState && appState.pomodoroDisplay !== "")
                            return appState.pomodoroDisplay
                        return "Aura Island"
                    }
                    color: Theme.textPrimary
                    font.family: Theme.fontFamily
                    font.pixelSize: Theme.fontCaptionSize
                    font.weight: Theme.fontTitleWeight
                    elide: Text.ElideRight
                    maximumLineCount: 1
                    width: parent.parent.width - 70
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Row {
                anchors.right: parent.right
                anchors.rightMargin: 8
                anchors.verticalCenter: parent.verticalCenter
                spacing: 6

                EqualizerBars {
                    visible: appState && appState.isPlaying
                    isPlaying: appState ? appState.isPlaying : false
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.openedByClick = true
                if (appState) {
                    if (appState.isMusicMorphActive) {
                        appState.setCurrentTab("media")
                    }
                    appState.toggleExpanded()
                }
            }
        }
    }

    // ─────────────────────────────────────────────────────────
    // 2.5. ALERT STATE (360x76, Radius 28)
    // ─────────────────────────────────────────────────────────
    Item {
        id: alertContainer
        anchors.fill: parent
        visible: (root.stateName === "alert")
        opacity: (visible && root.contentReady) ? 1.0 : 0.0
        scale: (visible && root.contentReady) ? 1.0 : 0.92

        Behavior on opacity {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0; epsilon: 0.001 }
        }

        AlertView {
            anchors.fill: parent
        }
    }

    // ─────────────────────────────────────────────────────────
    // 3. EXPANDED STATE (480x252 Home/Toggles or 480x300 Tabs)
    // ─────────────────────────────────────────────────────────
    Item {
        id: expandedView
        anchors.fill: parent
        anchors.margins: 10
        visible: (root.stateName === "expanded")
        opacity: (visible && root.contentReady) ? 1.0 : 0.0
        scale: (visible && root.contentReady) ? 1.0 : 0.92

        Behavior on opacity {
            NumberAnimation {
                duration: (root.stateName === "idle") ? Theme.contentCollapseDuration : 180
                easing.type: Easing.OutCubic
            }
        }

        Behavior on scale {
            SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0; epsilon: 0.001 }
        }

        Item {
            id: expandedContent
            anchors.fill: parent

            // 1. Row 0 Status (Height 28) - No bell, no close 'x'
            StatusRow {
                id: statusRow
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 28
            }

            // 2. Dock (Height 44)
            SlidingTabBar {
                id: bottomDock
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: 44
                activeTab: appState ? appState.currentTab : "home"
                onTabSelected: function(tabId) {
                    if (appState) appState.setCurrentTab(tabId)
                }
            }

            // 3. Tab Content Area
            Item {
                id: contentArea
                anchors.top: statusRow.bottom
                anchors.topMargin: Theme.gap
                anchors.bottom: bottomDock.top
                anchors.bottomMargin: Theme.gap
                anchors.left: parent.left
                anchors.right: parent.right
                clip: true

                HomeTab      { anchors.fill: parent; visible: !appState || appState.currentTab === "home" }
                TogglesTab   { anchors.fill: parent; visible: appState && appState.currentTab === "toggles" }
                MoreTab      { anchors.fill: parent; visible: appState && appState.currentTab === "more" }
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
    // 3.5. ONBOARDING STATE (480x252, Radius 34)
    // ─────────────────────────────────────────────────────────
    Item {
        id: onboardingContainer
        anchors.fill: parent
        visible: (root.stateName === "onboarding")
        opacity: (visible && root.contentReady) ? 1.0 : 0.0
        scale: (visible && root.contentReady) ? 1.0 : 0.92

        Behavior on opacity {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0; epsilon: 0.001 }
        }

        OnboardingView {
            anchors.fill: parent
        }
    }

    // ─────────────────────────────────────────────────────────
    // 4. DROP STATE (480x200)
    // ─────────────────────────────────────────────────────────
    Item {
        id: dropView
        anchors.fill: parent
        visible: (root.stateName === "drop")
        opacity: (visible && root.contentReady) ? 1.0 : 0.0
        scale: (visible && root.contentReady) ? 1.0 : 0.92

        Behavior on opacity {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }
        Behavior on scale {
            SpringAnimation { spring: 4.5; damping: 0.65; mass: 1.0; epsilon: 0.001 }
        }

        DropZone {
            anchors.fill: parent
        }
    }
}
