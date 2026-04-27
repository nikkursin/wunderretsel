import QtQuick

Item {
    id: root

    property string imageSource: "qrc:/assets/images/female/sample.jpg"
    property bool unlocked: false
    property bool blurred: !unlocked

    signal clicked()

    Rectangle {
        anchors.fill: parent
        radius: Math.min(22, root.width * 0.18)
        clip: true
        color: "#eadbe2"

        Image {
            anchors.fill: parent
            source: root.imageSource
            fillMode: Image.PreserveAspectCrop
            smooth: true
            opacity: root.unlocked ? 1.0 : 0.18
        }

        Rectangle {
            anchors.fill: parent
            visible: root.blurred
            color: Qt.rgba(0.42, 0.25, 0.34, 0.32)
        }

        Text {
            anchors.centerIn: parent
            visible: !root.unlocked
            text: "⌕"
            font.pixelSize: 24
            font.weight: Font.Bold
            color: "#8c6a7a"
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.unlocked
        onClicked: root.clicked()
    }
}
