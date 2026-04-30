import QtQuick

Rectangle {
    id: root

    signal clicked()

    property string text: ""
    property bool selected: false

    width: parent ? parent.width : 120
    height: 64

    radius: 18

    border.width: 1
    border.color: root.selected
                  ? (appStateManager ? appStateManager.themeControlBorder : Qt.rgba(0.93, 0.30, 0.56, 0.5))
                  : (appStateManager ? appStateManager.themeTileBorder : Qt.rgba(0.36, 0.10, 0.22, 0.1))

    color: root.selected
           ? Qt.rgba(1, 0.92, 0.95, 0.95)
           : Qt.rgba(1, 1, 1, 0.75)

    scale: mouseArea.pressed ? 0.97 : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: 120
            easing.type: Easing.OutQuad
        }
    }

    Text {
        anchors.centerIn: parent
        text: root.text
        color: root.selected
               ? (appStateManager ? appStateManager.themeControlText : "#b83268")
               : (appStateManager ? appStateManager.themeTextPrimary : "#3c172b")
        font.pixelSize: 16
        font.weight: Font.Bold
    }

    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: Qt.rgba(0.38, 0.06, 0.2, 0.05)
        z: -1
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
