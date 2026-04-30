import QtQuick

Rectangle {
    id: root

    signal clicked()

    property string text: ""
    property bool enabled: true

    width: parent ? parent.width : 300
    height: 54

    radius: 20

    gradient: Gradient {
        GradientStop {
            position: 0
            color: root.enabled
                   ? (appStateManager ? appStateManager.themeAccentStart : "#ef4f91")
                   : (appStateManager ? appStateManager.themeAccentSoftEnd : "#f2a6c3")
        }
        GradientStop {
            position: 1
            color: root.enabled
                   ? (appStateManager ? appStateManager.themeAccentEnd : "#d93279")
                   : (appStateManager ? appStateManager.themeAccentSoftStart : "#e3a9bf")
        }
    }

    opacity: root.enabled ? 1.0 : 0.6

    scale: mouseArea.pressed && root.enabled ? 0.97 : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: 120
            easing.type: Easing.OutQuad
        }
    }

    Text {
        anchors.centerIn: parent
        text: root.text
        color: "#ffffff"
        font.pixelSize: 16
        font.weight: Font.Bold
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.enabled
        onClicked: root.clicked()
    }
}
