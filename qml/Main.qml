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
