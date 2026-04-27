import QtQuick

Rectangle {
    id: card

    signal clicked()

    width: parent ? parent.width : 300
    height: 120

    radius: 28
    color: Qt.rgba(1, 1, 1, 0.62)

    border.color: Qt.rgba(0.45, 0.15, 0.3, 0.08)
    border.width: 1

    scale: mouseArea.pressed ? 0.98 : 1.0
    opacity: mouseArea.containsMouse ? 0.96 : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: 120
            easing.type: Easing.OutQuad
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 120
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 10
        radius: card.radius
        color: Qt.rgba(0.38, 0.06, 0.2, 0.08)
        z: -1
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: card.clicked()
    }
}
