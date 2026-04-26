import QtQuick
import Felgo

WRScreen {
    showBackButton: true

    Rectangle {
        anchors.centerIn: parent
        width: 100
        height: 100
        color: "red"
    }

    Text {
        id: name
        text: qsTr("Home Page")
    }

}
