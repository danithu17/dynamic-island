import QtQuick
import QtQuick.Shapes
import ".."

Item {
    id: root

    property int cornerRadius: 17
    property int bottomRadius: cornerRadius
    readonly property bool isNotch: (typeof settings !== "undefined" && settings && settings.islandStyle === "floating") ? false : true

    // Pure black solid shell (#000000, NO transparency, NO blur, NO glow, NO colored edge)
    // 1. Notch mode: MacBook style with 10px concave flares at top corners flush with screen edge
    Shape {
        id: notchShape
        visible: root.isNotch
        anchors.fill: parent
        layer.enabled: true
        layer.samples: 4

        ShapePath {
            fillColor: Theme.shell
            strokeColor: "transparent"
            strokeWidth: 0
            startX: -10
            startY: 0

            // Top-left concave flare (MacBook ear)
            PathQuad {
                x: 0
                y: 10
                controlX: 0
                controlY: 0
            }

            // Left straight edge
            PathLine {
                x: 0
                y: Math.max(10, root.height - root.bottomRadius)
            }

            // Bottom-left rounded corner
            PathArc {
                x: root.bottomRadius
                y: root.height
                radiusX: root.bottomRadius
                radiusY: root.bottomRadius
                direction: PathArc.Counterclockwise
            }

            // Bottom straight edge
            PathLine {
                x: Math.max(root.bottomRadius, root.width - root.bottomRadius)
                y: root.height
            }

            // Bottom-right rounded corner
            PathArc {
                x: root.width
                y: Math.max(10, root.height - root.bottomRadius)
                radiusX: root.bottomRadius
                radiusY: root.bottomRadius
                direction: PathArc.Counterclockwise
            }

            // Right straight edge
            PathLine {
                x: root.width
                y: 10
            }

            // Top-right concave flare (MacBook ear)
            PathQuad {
                x: root.width + 10
                y: 0
                controlX: root.width
                controlY: 0
            }

            // Close across top edge
            PathLine {
                x: -10
                y: 0
            }
        }
    }

    // 2. Floating mode: iPhone style rounded pill (10px gap from top)
    Rectangle {
        id: floatingShape
        visible: !root.isNotch
        anchors.fill: parent
        radius: root.cornerRadius
        color: Theme.shell
        antialiasing: true
    }
}
