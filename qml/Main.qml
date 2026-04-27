import Felgo
import QtQuick
import Wunderretsel 1.0
import "Views"

App {
    id: app

    Loader {
        id: screenLoader
        anchors.fill: parent

        sourceComponent: {
            switch (appStateManager.currentScreen) {
            case AppStateManager.Home:
                return homeComponent
            case AppStateManager.Play:
                return playComponent
            case AppStateManager.Gallery:
                return galleryComponent
            case AppStateManager.Settings:
                return settingsComponent
            case AppStateManager.Onboarding:
                return settingsComponent
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
