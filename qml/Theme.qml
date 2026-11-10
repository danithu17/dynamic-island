pragma Singleton
import QtQuick

QtObject {
    id: theme

    // ─────────────────────────────────────────────────────────
    // Section 1: Colors & Global Tokens
    // ─────────────────────────────────────────────────────────
    // Shell: solid #000000 (solid, opaque, NO transparency, NO blur, NO glow, NO colored edge)
    readonly property color shell: "#000000"
    readonly property color glassBackground: "#000000" // Backwards compatibility alias

    // Cards & Hairline
    readonly property color card: "#1C1C1E"
    readonly property color cardBackground: "#1C1C1E" // Backwards compatibility alias
    readonly property color cardPressed: "#2C2C2E"
    readonly property color hairline: Qt.rgba(1.0, 1.0, 1.0, 0.08)
    readonly property color glassBorder: hairline

    // Typography Colors
    readonly property color textPrimary: "#FFFFFF"
    readonly property color textSecondary: Qt.rgba(235 / 255.0, 235 / 255.0, 245 / 255.0, 0.60)
    readonly property color textTertiary:  Qt.rgba(235 / 255.0, 235 / 255.0, 245 / 255.0, 0.30)
    readonly property color textMuted: textTertiary

    // Module Accents (each module owns ONE accent)
    readonly property color accentBlue:   "#0A84FF" // Tray / Shell
    readonly property color accentGreen:  "#30D158" // Clipboard / Success
    readonly property color accentOrange: "#FF9F0A" // Focus / Timer
    readonly property color accentRed:    "#FF453A" // Low battery / Danger / Errors
    readonly property color accentPurple: "#BF5AF2" // Notes / Dev
    readonly property color accentPink:   "#FF375F" // Music / Media

    // Dynamic fallback accent
    readonly property color accent: (typeof settings !== "undefined" && settings && settings.accentColor) ? settings.accentColor : accentBlue
    readonly property color danger: accentRed
    readonly property color success: accentGreen
    readonly property color warning: accentOrange

    // ─────────────────────────────────────────────────────────
    // Typography
    // ─────────────────────────────────────────────────────────
    readonly property string fontFamily: "Inter, 'SF Pro Display', -apple-system, 'Segoe UI Variable Display', 'Segoe UI', sans-serif"
    readonly property string fontMono: "'SF Mono', 'Cascadia Code', Consolas, monospace"

    readonly property int fontTitleSize: 15
    readonly property int fontTitleWeight: Font.DemiBold // 600

    readonly property int fontBodySize: 13
    readonly property int fontBodyWeight: Font.Normal // 400

    readonly property int fontCaptionSize: 11
    readonly property int fontCaptionWeight: Font.Normal // 400

    readonly property int fontBigNumberSize: 28
    readonly property int fontBigNumberWeight: Font.Bold // 700

    // ─────────────────────────────────────────────────────────
    // Spacing & Shapes (8px grid)
    // ─────────────────────────────────────────────────────────
    readonly property int gridUnit: 8
    readonly property int panelPadding: 16
    readonly property int gap: 8
    readonly property int cardRadius: 20
    readonly property int chipRadius: 14
    readonly property int hairlineWidth: 1

    // ─────────────────────────────────────────────────────────
    // Section 3: Motion Specifications
    // ─────────────────────────────────────────────────────────
    // Unified spring parameters:
    // Expand: stiffness 250, damping 24, mass 1
    // Collapse: stiffness 320, damping 30, mass 1
    readonly property real springExpandStiffness: 250.0
    readonly property real springExpandDamping:   24.0
    readonly property real springCollapseStiffness: 320.0
    readonly property real springCollapseDamping:   30.0
    readonly property real springMass: 1.0

    // Content timings
    readonly property int contentExpandDelay: 80 // ms
    readonly property int contentCollapseDuration: 60 // ms
    readonly property int contentStaggerDelay: 30 // ms
}
