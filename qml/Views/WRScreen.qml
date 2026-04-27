import Felgo
import QtQuick
import QtQuick.Effects
import "../Components"

AppPage {
    id: root

    navigationBarHidden: true
    property string screenTitle: "Wunderrätsel"
    property bool showBackButton: false
    default property alias content: contentItem.data

    anchors.fill: parent

    FontLoader {
        id: titleFont
        source: "qrc:/assets/fonts/PlayfairDisplay-VariableFont_wght.ttf"
    }

    Rectangle {
        anchors.fill: parent

        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: "#fff7fa" }
            GradientStop { position: 0.44; color: "#f8dce8" }
            GradientStop { position: 1.0; color: "#e9adc5" }
        }

        Item { // TODO: Need more improvement according to what was given on the UI design
            id: glowLayer
            anchors.fill: parent

            Rectangle {
                id: topLeftGlow
                width: parent.width * 0.95
                height: width
                radius: width / 2
                x: -width * 0.42
                y: -height * 0.25
                color: "#FFFFFF"
                opacity: 0.58
            }

            MultiEffect {
                anchors.fill: topLeftGlow
                source: topLeftGlow
                blurEnabled: true
                blur: 0.85
            }

            Rectangle {
                id: topRightGlow
                width: parent.width * 0.9
                height: width
                radius: width / 2
                x: parent.width * 0.55
                y: -height * 0.18
                color: "#F7D5E4"
                opacity: 0.42
            }

            MultiEffect {
                anchors.fill: topRightGlow
                source: topRightGlow
                blurEnabled: true
                blur: 0.75
            }

            Rectangle {
                id: bottomGlow
                width: parent.width * 1.05
                height: width
                radius: width / 2
                x: parent.width * 0.08
                y: parent.height - height * 0.36
                color: "#DE9BB8"
                opacity: 0.18
            }

            MultiEffect {
                anchors.fill: bottomGlow
                source: bottomGlow
                blurEnabled: true
                blur: 0.9
            }
        }
    }


    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 28

        Grid {
            id: topBar

            width: parent.width
            height: 46

            columns: 3
            rows: 1
            columnSpacing: 0

            Item {
                width: 46
                height: 46

                WRBackArrow {
                    anchors.fill: parent
                    visible: root.showBackButton

                    onClicked: navigationStack.pop()
                }
            }

            Text {
                width: topBar.width - 92
                height: 46

                text: root.screenTitle
                font.pixelSize: 42
                font.family: titleFont.name
                font.weight: Font.Bold
                color: "#34101f"

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Item {
                width: 46
                height: 46
            }
        }

        Item {
            id: contentItem

            width: parent.width
            height: parent.height - topBar.height - parent.spacing
        }
    }
}
