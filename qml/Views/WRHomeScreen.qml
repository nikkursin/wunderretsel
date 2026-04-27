import QtQuick
import Felgo

import "../Components"

WRScreen {
    id: root

    showBackButton: false

    property int unlockedImages: 8
    property int totalImages: 24

    signal playClicked()
    signal galleryClicked()
    signal settingsClicked()

    Column {
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 20
            topMargin: 120
        }
        spacing: 16

        WRCard {
            id: playCard
            width: parent.width
            height: 310
            radius: 28

            Rectangle {
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: 14
                }
                height: parent.height - 106
                radius: 24

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#f9a2c2" }
                    GradientStop { position: 0.48; color: "#d85d93" }
                    GradientStop { position: 1.0; color: "#87326d" }
                }

                opacity: 0.75
            }

            Row {
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                    margins: 22
                }
                spacing: 16

                Column {
                    width: parent.width - 80
                    spacing: 8

                    Text {
                        text: qsTr("Play")
                        font.pixelSize: 28
                        font.bold: true
                        color: "#35111f"
                    }

                    Text {
                        text: qsTr("Start a new puzzle")
                        font.pixelSize: 14
                        color: "#6b3a4f"
                    }
                }

                Rectangle {
                    width: 48
                    height: 48
                    radius: 999

                    gradient: Gradient {
                        GradientStop { position: 0; color: "#eb5c99" }
                        GradientStop { position: 1; color: "#ad3974" }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "›"
                        font.pixelSize: 24
                        color: "white"
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.playClicked()
            }
        }

        WRCard {
            width: parent.width
            height: 154
            radius: 28

            Row {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 16

                Column {
                    width: parent.width - 110
                    spacing: 8

                    Text {
                        text: qsTr("Gallery")
                        font.pixelSize: 26
                        font.bold: true
                        color: "#35111f"
                    }

                    Text {
                        text: qsTr("Discovered images")
                        font.pixelSize: 14
                        color: "#6b3a4f"
                    }
                }

                Rectangle {
                    width: 92
                    height: 92
                    radius: 28

                    gradient: Gradient {
                        GradientStop { position: 0; color: "#fff4f9" }
                        GradientStop { position: 1; color: "#f58ab6" }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: root.unlockedImages + "/" + root.totalImages
                        font.pixelSize: 20
                        font.bold: true
                        color: "#401425"
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.galleryClicked()
            }
        }

        WRCard {
            width: parent.width
            height: 96
            radius: 24

            Row {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 16

                Column {
                    width: parent.width - 80
                    spacing: 6

                    Text {
                        text: qsTr("Settings")
                        font.pixelSize: 24
                        font.bold: true
                        color: "#35111f"
                    }

                    Text {
                        text: qsTr("Selected preferences")
                        font.pixelSize: 14
                        color: "#6b3a4f"
                    }
                }

                Rectangle {
                    width: 42
                    height: 42
                    radius: 21

                    gradient: Gradient {
                        GradientStop { position: 0; color: "#fff7fb" }
                        GradientStop { position: 1; color: "#e969a1" }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "+"
                        font.pixelSize: 20
                        color: "#401425"
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: root.settingsClicked()
            }
        }
    }
}
