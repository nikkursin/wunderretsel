import QtQuick
import QtQuick.Effects

Item {
    id: root

    property string imageSource: ""
    property bool   unlocked: false

    signal clicked()

    Rectangle {
        id: tile
        anchors.fill: parent
        radius: Math.min(22, root.width * 0.18)
        clip: true

        color: appStateManager ? appStateManager.themeTileBase : "#f4dde6"

        border.width: 1
        border.color: root.unlocked
                      ? Qt.rgba(1, 1, 1, 0.85)
                      : (appStateManager ? appStateManager.themeTileBorder : Qt.rgba(91/255, 25/255, 56/255, 0.10))
        Behavior on border.color { ColorAnimation { duration: 180 } }

        Image {
            id: photo
            anchors.fill: parent
            source: root.imageSource
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            smooth: true
            cache: true
            visible: status === Image.Ready
        }

        MultiEffect {
            anchors.fill: photo
            source: photo
            blurEnabled: !root.unlocked
            blur: 1.0
            blurMax: 32
            saturation: root.unlocked ? 0.0 : -0.35
            brightness: root.unlocked ? 0.0 : -0.05
            visible: !root.unlocked && photo.status === Image.Ready
            Behavior on blur { NumberAnimation { duration: 220 } }
        }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: appStateManager ? appStateManager.themeTileVeil : Qt.rgba(220/255, 70/255, 130/255, 0.22)
            opacity: root.unlocked ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: 240 } }
        }

        Rectangle {
            id: lockPill
            anchors.centerIn: parent
            width:  Math.max(28, root.width * 0.32)
            height: width
            radius: width / 2
            visible: !root.unlocked
            color: Qt.rgba(1, 1, 1, 0.88)
            border.width: 1
            border.color: appStateManager ? appStateManager.themeControlBorder : Qt.rgba(159/255, 47/255, 97/255, 0.22)

            Text {
                anchors.centerIn: parent
                text: "\uD83D\uDD12"
                font.pixelSize: lockPill.width * 0.5
                color: appStateManager ? appStateManager.themeControlText : "#9f2f61"
            }
        }

        scale: root.unlocked
               ? (mouseArea.pressed ? 0.97 : (mouseArea.containsMouse ? 1.03 : 1.0))
               : 1.0
        Behavior on scale {
            NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
        }
    }

    layer.enabled: root.unlocked
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor:    "#553a0f22"
        shadowOpacity:  0.32
        shadowBlur:     0.7
        shadowVerticalOffset: 4
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.unlocked
        hoverEnabled: true
        cursorShape: root.unlocked ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.clicked()
    }
}
