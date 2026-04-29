import QtQuick
import QtQuick.Effects
import Felgo

Item {
    id: root

    signal clicked()

    width: 46
    height: 46

    property bool pressed: mouseArea.pressed
    property bool hovered: mouseArea.containsMouse

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: width / 2
        color: pressed ? Qt.rgba(1, 1, 1, 0.5) : Qt.rgba(1, 1, 1, 0.58)

        Behavior on color {
            ColorAnimation { duration: 140; easing.type: Easing.OutCubic }
        }
    }

    MultiEffect {
        anchors.fill: bg
        source: bg
        shadowEnabled: true
        shadowColor: "#402A0F1F"
        shadowBlur: 0.6
        shadowOpacity: pressed ? 0.3 : hovered ? 0.7 : 0.5
        shadowVerticalOffset: pressed ? 2 : hovered ? 7 : 4

        Behavior on shadowOpacity {
            NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
        }
        Behavior on shadowVerticalOffset {
            NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
        }
    }

    Text {
        anchors.centerIn: parent
        anchors.horizontalCenterOffset: -1
        text: "‹"
        font.pixelSize: 26
        font.weight: Font.DemiBold
        color: "#401425"
    }

    scale: pressed ? 0.91 : hovered ? 1.08 : 1.0

    Behavior on scale {
        NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        // Emit the public signal instead of reaching into a global
        // context property here. Keeping the component navigation-
        // agnostic means it still loads cleanly inside the Felgo Live
        // App (where `appStateManager` does not exist) and lets the
        // parent screen wire it up to whatever back-navigation the app
        // is using.
        onClicked: root.clicked()
    }
}
