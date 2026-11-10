import QtQuick
import ".."

Item {
    id: root

    implicitWidth: 46
    implicitHeight: 22

    property string mood: appState ? appState.mascotMood : "normal"
    property real pupilOffsetX: appState ? appState.mascotPupilX : 0.0
    property real pupilOffsetY: appState ? appState.mascotPupilY : 0.0
    property bool isWide: (appState && appState.islandState === "drop") || mood === "wide"
    property bool isHappySquint: mood === "squint"

    // Periodic random blinking
    Timer {
        id: blinkTimer
        interval: 3500 + Math.random() * 2500
        running: true
        repeat: true
        onTriggered: {
            if (!root.isHappySquint && !root.isWide) {
                blinkAnim.restart()
            }
            interval = 3200 + Math.random() * 2600
        }
    }

    SequentialAnimation {
        id: blinkAnim
        ParallelAnimation {
            NumberAnimation { target: leftEyeContainer; property: "height"; to: 2; duration: 75; easing.type: Easing.OutQuad }
            NumberAnimation { target: rightEyeContainer; property: "height"; to: 2; duration: 75; easing.type: Easing.OutQuad }
        }
        ParallelAnimation {
            NumberAnimation { target: leftEyeContainer; property: "height"; to: root.isWide ? 19 : 15; duration: 90; easing.type: Easing.InQuad }
            NumberAnimation { target: rightEyeContainer; property: "height"; to: root.isWide ? 19 : 15; duration: 90; easing.type: Easing.InQuad }
        }
    }

    Row {
        anchors.centerIn: parent
        spacing: 7

        // Left Eye
        Item {
            id: leftEyeContainer
            width: root.isWide ? 15 : 13
            height: root.isWide ? 19 : 15
            anchors.verticalCenter: parent.verticalCenter

            Behavior on width { NumberAnimation { duration: 150 } }
            Behavior on height { NumberAnimation { duration: 150 } }

            // Happy squint curve ( ^ )
            Text {
                anchors.centerIn: parent
                text: "^"
                font.family: Theme.fontFamily
                font.pixelSize: 17
                font.bold: true
                color: "#FFFFFF"
                visible: root.isHappySquint
                scale: 1.1
            }

            // Normal / Wide Eye Capsule
            Rectangle {
                id: leftEye
                anchors.fill: parent
                radius: width / 2
                color: "#FFFFFF"
                visible: !root.isHappySquint
                antialiasing: true

                // Pupil / Iris
                Rectangle {
                    id: leftPupil
                    width: root.isWide ? 6.5 : 5.5
                    height: root.isWide ? 7.5 : 6.0
                    radius: width / 2
                    color: "#0A0A0C"
                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: root.pupilOffsetX
                    anchors.verticalCenterOffset: root.pupilOffsetY

                    Behavior on anchors.horizontalCenterOffset {
                        SpringAnimation { spring: 3.5; damping: 0.6; duration: 120 }
                    }
                    Behavior on anchors.verticalCenterOffset {
                        SpringAnimation { spring: 3.5; damping: 0.6; duration: 120 }
                    }

                    // Eye light glint (catchlight)
                    Rectangle {
                        width: 2
                        height: 2
                        radius: 1
                        color: "#FFFFFF"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.topMargin: 1
                        anchors.leftMargin: 1
                    }
                }
            }
        }

        // Right Eye
        Item {
            id: rightEyeContainer
            width: root.isWide ? 15 : 13
            height: root.isWide ? 19 : 15
            anchors.verticalCenter: parent.verticalCenter

            Behavior on width { NumberAnimation { duration: 150 } }
            Behavior on height { NumberAnimation { duration: 150 } }

            // Happy squint curve ( ^ )
            Text {
                anchors.centerIn: parent
                text: "^"
                font.family: Theme.fontFamily
                font.pixelSize: 17
                font.bold: true
                color: "#FFFFFF"
                visible: root.isHappySquint
                scale: 1.1
            }

            // Normal / Wide Eye Capsule
            Rectangle {
                id: rightEye
                anchors.fill: parent
                radius: width / 2
                color: "#FFFFFF"
                visible: !root.isHappySquint
                antialiasing: true

                // Pupil / Iris
                Rectangle {
                    id: rightPupil
                    width: root.isWide ? 6.5 : 5.5
                    height: root.isWide ? 7.5 : 6.0
                    radius: width / 2
                    color: "#0A0A0C"
                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: root.pupilOffsetX
                    anchors.verticalCenterOffset: root.pupilOffsetY

                    Behavior on anchors.horizontalCenterOffset {
                        SpringAnimation { spring: 3.5; damping: 0.6; duration: 120 }
                    }
                    Behavior on anchors.verticalCenterOffset {
                        SpringAnimation { spring: 3.5; damping: 0.6; duration: 120 }
                    }

                    // Eye light glint (catchlight)
                    Rectangle {
                        width: 2
                        height: 2
                        radius: 1
                        color: "#FFFFFF"
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.topMargin: 1
                        anchors.leftMargin: 1
                    }
                }
            }
        }
    }
}
