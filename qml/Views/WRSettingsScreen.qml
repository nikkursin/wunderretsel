import QtQuick
import QtQuick.Layouts
import "../Components"

WRScreen {
    id: root
    showBackButton: true


    Flickable {
        anchors.fill: parent
        anchors.margins: 16

        contentWidth: width
        contentHeight: contentColumn.implicitHeight
        clip: true

        ColumnLayout {
            id: contentColumn

            width: parent.width
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                anchors.margins: 20
                spacing: 14

                Item {
                    Layout.fillHeight: true
                }

                Text {
                    text: "Settings"
                    font.pixelSize: 32
                    font.bold: true
                    font.weight: Font.Bold
                    font.letterSpacing: -1.2
                    color: "#35111f"
                }

                Text {
                    text: "Selected preferences"
                    font.pixelSize: 15
                    font.weight: Font.Bold
                    color: Qt.rgba(0.23, 0.09, 0.15, 0.66)
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            WRCard {
                Layout.fillWidth: true
                Layout.preferredHeight: 240
                interactive: false

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 14


                    Text {
                        text: "Language level"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#35111f"
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 3
                        rowSpacing: 10
                        columnSpacing: 10

                        Repeater {
                            model: ["A1", "A2", "B1", "B2", "C1", "C2"]

                            WRChoiceCard {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 56

                                text: modelData
                                selected: appStateManager.languageLevel === modelData

                                onClicked: {
                                    appStateManager.languageLevel = modelData
                                }
                            }
                        }
                    }
                }
            }

            WRCard {
                Layout.fillWidth: true
                Layout.preferredHeight: 280
                interactive: false

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 14

                    Text {
                        text: "Character preference"
                        font.pixelSize: 18
                        font.bold: true
                        color: "#35111f"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Repeater {
                            model: [
                                { label: "Female", value: "female" },
                                { label: "Male", value: "male" },
                                { label: "Mixed", value: "mixed" }
                            ]

                            WRChoiceCard {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 56

                                text: modelData.label
                                selected: appStateManager.characterType === modelData.value

                                onClicked: {
                                    appStateManager.characterType = modelData.value
                                }
                            }
                        }
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                Layout.leftMargin: 2
                Layout.rightMargin: 2

                text: "Changes affect future puzzles and image pool. Preferences are stored locally on this device."
                wrapMode: Text.WordWrap
                font.pixelSize: 13
                lineHeight: 1.25
                color: Qt.rgba(0.23, 0.09, 0.15, 0.58)
            }

            WRButton {
                Layout.fillWidth: true
                Layout.topMargin: 18

                text: "Save changes"

                onClicked: {
                    appStateManager.savePreferences()
                    appStateManager.goHome()
                }
            }
        }
    }
}
