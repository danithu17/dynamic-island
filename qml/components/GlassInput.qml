import QtQuick
import ".."

Item {
    id: root
    property alias text: input.text
    property string placeholderText: "Search..."
    property int cornerRadius: 8
    signal textChangedByUser(string newText)

    implicitWidth: 200
    implicitHeight: 32

    Rectangle {
        anchors.fill: parent
        radius: root.cornerRadius
        color: input.activeFocus ? Theme.glassSurfaceActive : Theme.glassSurface
        border.color: input.activeFocus ? Theme.accent : Theme.glassBorder
        border.width: 1

        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }

        Row {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 8

            IconImage {
                source: "qrc:/resources/icons/search.svg"
                size: 14
                color: Theme.textMuted
                anchors.verticalCenter: parent.verticalCenter
            }

            TextInput {
                id: input
                width: parent.width - 48
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.textPrimary
                font.family: Theme.fontFamily
                font.pixelSize: 12
                selectByMouse: true
                selectionColor: Theme.accent

                Text {
                    text: root.placeholderText
                    color: Theme.textMuted
                    font.family: Theme.fontFamily
                    font.pixelSize: 12
                    visible: !input.text && !input.activeFocus
                    anchors.verticalCenter: parent.verticalCenter
                }

                onTextChanged: root.textChangedByUser(text)
            }

            IconImage {
                visible: input.text.length > 0
                source: "qrc:/resources/icons/close.svg"
                size: 12
                color: Theme.textMuted
                anchors.verticalCenter: parent.verticalCenter

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        input.text = ""
                        root.textChangedByUser("")
                    }
                }
            }
        }
    }
}
