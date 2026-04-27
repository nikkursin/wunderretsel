import Felgo
import QtQuick

App {

    NavigationStack {

            id: navigation

            Component.onCompleted: {
                console.log("QML onboardingCompleted:", appStateManager.onboardingCompleted)
            }

            initialPage: appStateManager.onboardingCompleted
                ? Qt.resolvedUrl("Views/WRHomeScreen.qml")
                : Qt.resolvedUrl("Views/WROnboardingScreen.qml")
        }
}
