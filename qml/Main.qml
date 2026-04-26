import Felgo
import QtQuick

App {

    NavigationStack {

            id: navigation

            property bool onboardingCompleted: true

            initialPage: onboardingCompleted
                ? Qt.resolvedUrl("Views/WRHomeScreen.qml")
                : Qt.resolvedUrl("Views/WROnboardingScreen.qml")
        }
}
