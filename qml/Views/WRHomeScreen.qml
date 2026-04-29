import QtQuick
import QtQuick.Effects
import Felgo

import "../Components"

WRScreen {
    id: root

    showBackButton: false

    // Counter values are pulled live from AppStateManager so the home
    // card always agrees with the gallery screen and the underlying
    // unlocked-image set persisted in storage. The defensive `?:` on
    // appStateManager keeps the QML editor preview happy when the
    // context property isn't injected.
    readonly property int unlockedImages:
        appStateManager ? appStateManager.unlockedImagesCount : 0
    readonly property int totalImages:
        appStateManager ? appStateManager.totalImagesCount : 0
    property string playCardImageSource: "qrc:/assets/images/female/sample.jpg"

    signal playClicked()
    signal galleryClicked()
    signal settingsClicked()

    function refreshPlayCardImage() {
        if (!appStateManager || !appStateManager.galleryImages
                || appStateManager.galleryImages.length === 0) {
            playCardImageSource = "qrc:/assets/images/female/sample.jpg"
            return
        }

        var images = appStateManager.galleryImages
        var randomIndex = Math.floor(Math.random() * images.length)
        playCardImageSource = images[randomIndex].source
    }

    Component.onCompleted: refreshPlayCardImage()

    Connections {
        target: appStateManager
        function onGalleryImagesChanged() {
            root.refreshPlayCardImage()
        }
    }

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

            Item {
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    margins: 14
                }
                height: parent.height - 106
                clip: true

                Image {
                    id: bgImage
                    anchors.fill: parent
                    source: root.playCardImageSource
                    fillMode: Image.PreserveAspectCrop
                }

                MultiEffect {
                    anchors.fill: bgImage
                    source: bgImage
                    blurEnabled: true
                    blur: 0.8
                    saturation: 1.1
                    brightness: 0.1
                }

                Rectangle {
                    anchors.fill: parent
                    color: appStateManager ? appStateManager.themeAccentSoftStart : "#ffeef5"
                    opacity: 0.35
                }
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
                        color: appStateManager ? appStateManager.themeTextPrimary : "#35111f"
                    }

                    Text {
                        text: qsTr("Start a new puzzle")
                        font.pixelSize: 14
                        color: appStateManager ? appStateManager.themeTextSecondary : "#6b3a4f"
                    }
                }

                Rectangle {
                    width: 48
                    height: 48
                    radius: 999

                    gradient: Gradient {
                        GradientStop { position: 0; color: appStateManager ? appStateManager.themeAccentStart : "#eb5c99" }
                        GradientStop { position: 1; color: appStateManager ? appStateManager.themeAccentEnd : "#ad3974" }
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
                        color: appStateManager ? appStateManager.themeTextPrimary : "#35111f"
                    }

                    Text {
                        text: qsTr("Discovered images")
                        font.pixelSize: 14
                        color: appStateManager ? appStateManager.themeTextSecondary : "#6b3a4f"
                    }
                }

                Rectangle {
                    width: 92
                    height: 92
                    radius: 28

                    gradient: Gradient {
                        GradientStop { position: 0; color: appStateManager ? appStateManager.themeAccentSoftStart : "#fff4f9" }
                        GradientStop { position: 1; color: appStateManager ? appStateManager.themeAccentSoftEnd : "#f58ab6" }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: root.unlockedImages + "/" + root.totalImages
                        font.pixelSize: 20
                        font.bold: true
                        color: appStateManager ? appStateManager.themeTextStrong : "#401425"
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
                        color: appStateManager ? appStateManager.themeTextPrimary : "#35111f"
                    }

                    Text {
                        text: qsTr("Selected preferences")
                        font.pixelSize: 14
                        color: appStateManager ? appStateManager.themeTextSecondary : "#6b3a4f"
                    }
                }

                Rectangle {
                    width: 42
                    height: 42
                    radius: 21

                    gradient: Gradient {
                        GradientStop { position: 0; color: appStateManager ? appStateManager.themeAccentSoftStart : "#fff7fb" }
                        GradientStop { position: 1; color: appStateManager ? appStateManager.themeAccentStart : "#e969a1" }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "+"
                        font.pixelSize: 20
                        color: appStateManager ? appStateManager.themeTextStrong : "#401425"
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
