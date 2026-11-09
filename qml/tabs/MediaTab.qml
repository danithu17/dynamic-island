import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 14

        // Header
        Row {
            width: parent.width
            height: 28

            Text {
                text: "Now Playing"
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 15
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
            }


            EqualizerBars {
                isPlaying: mediaManager ? mediaManager.isPlaying : false
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        // Media Card
        GlassCard {
            width: parent.width
            implicitHeight: 180

            Column {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Row {
                    width: parent.width
                    spacing: 16

                    // Album Art Placeholder / Vinyl Graphic
                    Rectangle {
                        width: 72
                        height: 72
                        radius: 12
                        color: Qt.rgba(0.0, 0.0, 0.0, 0.5)
                        border.color: Theme.glassBorder
                        border.width: 1

                        IconImage {
                            anchors.centerIn: parent
                            source: "qrc:/resources/icons/media.svg"
                            size: 32
                            color: Theme.accent
                        }
                    }

                    Column {
                        width: parent.width - 100
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 4

                        Text {
                            text: mediaManager ? mediaManager.title : "No Media Playing"
                            color: Theme.textPrimary
                            font.family: Theme.fontFamily
                            font.pixelSize: 15
                            font.bold: true
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: mediaManager ? mediaManager.artist : "Windows Media Transport"
                            color: Theme.textSecondary
                            font.family: Theme.fontFamily
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            width: parent.width
                        }

                        Text {
                            text: (mediaManager && mediaManager.album !== "") ? mediaManager.album : "Ready"
                            color: Theme.textMuted
                            font.family: Theme.fontFamily
                            font.pixelSize: 10
                            elide: Text.ElideRight
                            width: parent.width
                        }
                    }
                }

                // Playback Controls
                Row {
                    anchors.horizontalCenter: parent.horizontalCenter
                    spacing: 16

                    // Previous
                    GlassButton {
                        iconSource: "qrc:/resources/icons/prev.svg"
                        cornerRadius: 20
                        implicitWidth: 40
                        implicitHeight: 40
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: if (mediaManager) mediaManager.skipPrevious()
                    }

                    // Play / Pause
                    GlassButton {
                        iconSource: (mediaManager && mediaManager.isPlaying) ? "qrc:/resources/icons/pause.svg" : "qrc:/resources/icons/play.svg"
                        isAccent: true
                        cornerRadius: 24
                        implicitWidth: 48
                        implicitHeight: 48
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: if (mediaManager) mediaManager.togglePlayPause()
                    }

                    // Next
                    GlassButton {
                        iconSource: "qrc:/resources/icons/next.svg"
                        cornerRadius: 20
                        implicitWidth: 40
                        implicitHeight: 40
                        anchors.verticalCenter: parent.verticalCenter
                        onClicked: if (mediaManager) mediaManager.skipNext()
                    }
                }
            }
        }
    }
}
