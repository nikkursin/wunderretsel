import Felgo
import QtQuick
import "Views"

App {
    id: app

    Loader {
        id: screenLoader
        anchors.fill: parent

        sourceComponent: {
            switch (appStateManager.currentScreen) {
            case 0:
                return homeComponent
            case 1:
                return playComponent
            case 2:
                return galleryComponent
            case 3:
                return settingsComponent
            case 4:
                return onboardingComponent
            default:
                return homeComponent
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        visible: screenLoader.status === Loader.Error
        color: "#fff7fa"
        z: 999

        Text {
            anchors.centerIn: parent
            width: parent.width - 48
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            color: "#401425"
            font.pixelSize: 16
            text: qsTr("Failed to load screen component. Check Android logcat for 'QML warning:' lines.")
        }
    }

    Component {
        id: homeComponent

        WRHomeScreen {
            onPlayClicked: appStateManager.goPlay()
            onGalleryClicked: appStateManager.goGallery()
            onSettingsClicked: appStateManager.goSettings()
        }
    }

    Component {
        id: playComponent
        WRPlayScreen {
            showBackButton: true
            // onBackClicked: appStateManager.goHome()
            onGalleryClicked: appStateManager.goGallery()
            onNextPuzzleClicked: appStateManager.goPlay()
        }
    }

    Component {
        id: galleryComponent
        WRGalleryScreen {
            showBackButton: true
        }
    }

    Component {
        id: settingsComponent
        WRSettingsScreen {
            showBackButton: true
        }
    }

    Component {
        id: onboardingComponent
        WROnboardingScreen {
            showBackButton: false
        }
    }
}
