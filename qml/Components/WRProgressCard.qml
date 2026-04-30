import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property string title: "Gallery"
    property string subtitle: "Discovered images"
    property string value: "0/0"
    property real progress: 0.0

    width: parent ? parent.width : 320
    height: 170

    Rectangle {
        anchors.fill: parent
        radius: 30
        color: Qt.rgba(1, 1, 1, 0.64)

        border.color: Qt.rgba(0.45, 0.15, 0.3, 0.08)
        border.width: 1

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: "transparent"
            layer.enabled: true
        }

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 18

            Row {
                width: parent.width
                spacing: 12

                Column {
                    spacing: 6

                    Text {
                        text: root.title
                        font.pixelSize: 26
                        font.weight: Font.DemiBold
                        color: "#35111f"
                    }

                    Text {
                        text: root.subtitle
                        font.pixelSize: 14
                        color: "#6a3a4f"
                        opacity: 0.75
                    }
                }

                Item { width: 1; height: 1; Layout.fillWidth: true }

                Rectangle {
                    width: 80
                    height: 80
                    radius: 24

                    color: "transparent"

                    border.color: Qt.rgba(0.45, 0.15, 0.3, 0.08)

                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius

                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#fff4f9" }
                            GradientStop { position: 1.0; color: "#f58ab6" }
                        }

                        opacity: 0.9
                    }

                    Text {
                        anchors.centerIn: parent
                        text: root.value
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        color: "#401425"
                    }
                }
            }

            Rectangle {
                width: parent.width
                height: 12
                radius: 999
                color: Qt.rgba(0.35, 0.15, 0.25, 0.1)

                Rectangle {
                    width: parent.width * root.progress
                    height: parent.height
                    radius: parent.radius

                    gradient: Gradient {
                        GradientStop { position: 0.0; color: "#eb5c99" }
                        GradientStop { position: 1.0; color: "#ad3974" }
                    }
                }
            }
        }
    }
}
