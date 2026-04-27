import QtQuick
import Felgo

WRScreen {

    title: "Wunder"


    Rectangle {
        id: topLeftGlow
        width: parent.width * 0.95
        height: width
        radius: width / 2
        x: -width * 0.42
        y: -height * 0.25
        color: "RED"
        opacity: 0.58
    }

}
