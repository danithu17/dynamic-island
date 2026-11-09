pragma Singleton
import QtQuick

QtObject {
    id: theme

    // Palette: Apple iOS 26 Liquid Glass
    readonly property color glassBackground: Qt.rgba(0.08, 0.09, 0.12, 0.88)
    readonly property color glassSurface: Qt.rgba(0.15, 0.16, 0.22, 0.65)
    readonly property color glassSurfaceHover: Qt.rgba(0.22, 0.24, 0.32, 0.75)
    readonly property color glassSurfaceActive: Qt.rgba(0.28, 0.30, 0.40, 0.85)
    readonly property color glassBorder: Qt.rgba(1.0, 1.0, 1.0, 0.14)
    readonly property color glassBorderHighlight: Qt.rgba(1.0, 1.0, 1.0, 0.28)
    readonly property color glassSpecular: Qt.rgba(1.0, 1.0, 1.0, 0.18)

    // Typography & Accents
    readonly property color textPrimary: "#FFFFFF"
    readonly property color textSecondary: "#9E9EA8"
    readonly property color textMuted: "#6B6C77"
    
    // Dynamic Accent color bound to Settings
    readonly property color accent: settings ? settings.accentColor : "#0A84FF"
    readonly property color accentGlow: Qt.rgba(accent.r, accent.g, accent.b, 0.30)
    readonly property color danger: "#FF453A"
    readonly property color success: "#30D158"
    readonly property color warning: "#FF9F0A"

    // Fonts
    readonly property string fontFamily: "Segoe UI Variable Display, Segoe UI, Inter, -apple-system, sans-serif"
    readonly property string fontMono: "Cascadia Code, Consolas, Courier New, monospace"

    // Animation constants
    readonly property int animDurationFast:   160
    readonly property int animDurationNormal: 300
    readonly property int animDurationSlow:   400

    // Easing
    readonly property var animEasing:       Easing.OutCubic
    readonly property var animEasingSpring: Easing.OutBack

    // Spring presets (use directly in SpringAnimation properties)
    readonly property real springMedium:  2.8
    readonly property real springFast:    4.5
    readonly property real dampingMedium: 0.52
    readonly property real dampingFirm:   0.65
}
