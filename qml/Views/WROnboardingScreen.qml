import QtQuick
import QtQuick.Layouts
import "../Components"

WRScreen {
    id: root

    property int currentStep: 0
    property string selectedLanguageLevel: ""
    property string selectedCharacterType: ""

    signal onboardingCompleted()

    showBackButton: false

    readonly property var languageLevels: ["A1", "A2", "B1", "B2", "C1", "C2"]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 18

        WRCard {
            Layout.fillWidth: true
            Layout.fillHeight: true
            interactive: false

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 22

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Rectangle {
                        Layout.fillWidth: true
                        height: 5
                        radius: 999
                        color: "#ec4d8d"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 5
                        radius: 999
                        color: root.currentStep === 1 ? "#ec4d8d" : Qt.rgba(0.37, 0.13, 0.25, 0.12)
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        text: root.currentStep === 0 ? "Step 1" : "Step 2"
                        color: "#c43e78"
                        font.pixelSize: 12
                        font.weight: Font.Bold
                    }

                    Text {
                        text: root.currentStep === 0
                              ? "Choose your language level"
                              : "Choose character type"
                        color: "#2e1423"
                        font.pixelSize: 28
                        font.weight: Font.Bold
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    Text {
                        text: root.currentStep === 0
                              ? "Select one level for the first game setup."
                              : "Select one image preference for puzzle content."
                        color: Qt.rgba(0.2, 0.1, 0.16, 0.68)
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }
                }

                GridLayout {
                    visible: root.currentStep === 0
                    Layout.fillWidth: true
                    columns: 3
                    rowSpacing: 10
                    columnSpacing: 10

                    Repeater {
                        model: root.languageLevels

                        WRChoiceCard {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 74

                            text: modelData
                            selected: root.selectedLanguageLevel === modelData

                            onClicked: root.selectedLanguageLevel = modelData
                        }
                    }
                }

                ColumnLayout {
                    visible: root.currentStep === 1
                    Layout.fillWidth: true
                    spacing: 12

                    WRChoiceCard {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 64

                        text: "♀  Female characters"
                        selected: root.selectedCharacterType === "female"

                        onClicked: root.selectedCharacterType = "female"
                    }

                    WRChoiceCard {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 64

                        text: "♂  Male characters"
                        selected: root.selectedCharacterType === "male"

                        onClicked: root.selectedCharacterType = "male"
                    }

                    WRChoiceCard {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 64

                        text: "◇  Mixed characters"
                        selected: root.selectedCharacterType === "mixed"

                        onClicked: root.selectedCharacterType = "mixed"
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                WRButton {
                    Layout.fillWidth: true

                    text: root.currentStep === 0 ? "Continue" : "Start"

                    enabled: root.currentStep === 0
                             ? root.selectedLanguageLevel.length > 0
                             : root.selectedCharacterType.length > 0

                    onClicked: {
                        if (root.currentStep === 0) {
                            root.currentStep = 1
                            return
                        }

                        appStateManager.completeOnboarding(
                            root.selectedLanguageLevel,
                            root.selectedCharacterType
                        )

                        root.onboardingCompleted()
                    }
                }
            }
        }
    }
}
