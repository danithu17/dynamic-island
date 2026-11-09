import QtQuick
import QtQuick.Effects
import ".."

Item {
    id: root
    property string source: ""
    property color color: (Theme && Theme.textPrimary) ? Theme.textPrimary : "#FFFFFF"
    property int size: 18

    width: size
    height: size

    Image {
        id: img
        anchors.fill: parent
        source: root.source
        sourceSize.width: root.size * 2
        sourceSize.height: root.size * 2
        fillMode: Image.PreserveAspectFit
        smooth: true
        visible: false
    }

    MultiEffect {
        id: effect
        anchors.fill: img
        source: img
        colorization: 1.0
        colorizationColor: root.color
    }
}
