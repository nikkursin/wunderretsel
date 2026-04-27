import QtQuick
import QtQuick.Effects

Item {
    id: card

    signal clicked()

    property bool interactive: true
    property bool hoverEnabled: true
    property real normalScale: 1.0
    property real hoverScale: 1.015
    property real pressedScale: 0.975
    property real radius: 28
    property alias color: bg.color
    property alias border: bg.border

    implicitHeight: 120

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: card.radius
        color: Qt.rgba(1, 1, 1, 0.82)
        border { color: Qt.rgba(0.45, 0.15, 0.3, 0.08); width: 1 }
    }

    scale: {
        if (!interactive) return normalScale
        if (mouseArea.pressed) return pressedScale
        if (mouseArea.containsMouse) return hoverScale
        return normalScale
    }

    Behavior on scale {
        NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
    }

    layer.enabled: true
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: Qt.rgba(0.38, 0.06, 0.2, 1)
        shadowOpacity: mouseArea.containsMouse ? 0.18 : 0.12
        shadowBlur: 0.8
        shadowVerticalOffset: mouseArea.containsMouse ? 8 : 5

        Behavior on shadowOpacity {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }
        Behavior on shadowVerticalOffset {
            NumberAnimation { duration: 160; easing.type: Easing.OutCubic }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: card.interactive
        hoverEnabled: card.hoverEnabled
        cursorShape: Qt.PointingHandCursor
        onClicked: card.clicked()
    }
}
