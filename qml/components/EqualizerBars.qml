import QtQuick
import ".."

Item {
    id: root
    property bool isPlaying: appState ? appState.isPlaying : false
    property color barColor: Theme.accent
    property int barCount: 4

    width: barCount * 3 + (barCount - 1) * 2
    height: 14

    Row {
        anchors.bottom: parent.bottom
        spacing: 2

        Repeater {
            model: root.barCount

            Rectangle {
                id: bar
                width: 2.5
                radius: 1.2
                color: root.barColor
                anchors.bottom: parent.bottom

                property int baseHeight: (index % 2 === 0) ? 6 : 10
                height: root.isPlaying ? baseHeight : 2

                SequentialAnimation on height {
                    running: root.isPlaying
                    loops: Animation.Infinite

                    NumberAnimation {
                        to: 12 - (index * 2)
                        duration: 220 + (index * 90)
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        to: 3 + (index * 2)
                        duration: 180 + (index * 70)
                        easing.type: Easing.InOutQuad
                    }
                    NumberAnimation {
                        to: 14
                        duration: 260 + (index * 80)
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
    }
}
