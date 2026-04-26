import QtQuick
import QtQuick.Effects
import Felgo

Item {
    id: root

    signal clicked()

    width: 46
    height: 46

    Rectangle {
        id: bg
        anchors.fill: parent
        radius: width / 2
        color: "#94FFFFFF"
    }

    MultiEffect {
        anchors.fill: bg
        source: bg

        shadowEnabled: true

        shadowColor: "#402A0F1F"
        shadowBlur: 0.6
        shadowVerticalOffset: 4
    }

    Text {
        anchors.centerIn: parent
        text: "‹"
        font.pixelSize: 26
        font.weight: Font.DemiBold
        color: "#401425"
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
    }
}
