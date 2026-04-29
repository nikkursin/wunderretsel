import Felgo
import QtQuick
import "Views"

App {
    id: app

    // Global custom font. Kept at App level so it lives for the whole
    // application lifetime instead of being created/destroyed every time
    // the screen Loader swaps a component (which on Android races with
    // the QSGDistanceFieldGlyphCache and crashes Qt Quick text rendering).
    //
    // QML reads `app.titleFontFamily` instead of `titleFont.name` directly,
    // so screens always get an empty string (→ system font) until the
    // font has actually finished loading. No glyphs are ever released
    // for a font engine that's about to go away.
    FontLoader {
        id: titleFont
        source: "qrc:/assets/fonts/PlayfairDisplay-VariableFont_wght.ttf"
    }

    readonly property string titleFontFamily:
        titleFont.status === FontLoader.Ready ? titleFont.name : ""

    Loader {
        id: screenLoader
        anchors.fill: parent
        asynchronous: false

        sourceComponent: {
            if (!appStateManager)
                return homeComponent

            switch (appStateManager.currentScreen) {
            case 0: return homeComponent
            case 1: return playComponent
            case 2: return galleryComponent
            case 3: return settingsComponent
            case 4: return onboardingComponent
            default: return homeComponent
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
