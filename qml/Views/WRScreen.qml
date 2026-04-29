import Felgo
import QtQuick
import "../Components"

AppPage {
    id: root

    navigationBarHidden: true
    property string screenTitle: "Wunderrätsel"
    property bool showBackButton: false
    default property alias content: contentItem.data

    anchors.fill: parent

    Rectangle {
        anchors.fill: parent

        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop { position: 0.0; color: appStateManager ? appStateManager.themeTintLight : "#fff7fa" }
            GradientStop { position: 0.44; color: appStateManager ? appStateManager.themeTintMid : "#f8dce8" }
            GradientStop { position: 1.0; color: appStateManager ? appStateManager.themeTintDeep : "#e9adc5" }
        }

        Item { // TODO: Need more improvement according to what was given on the UI design
            id: glowLayer
            anchors.fill: parent

            Rectangle {
                id: topLeftGlow
                width: parent.width * 0.95
                height: width
                radius: width / 2
                x: -width * 0.42
                y: -height * 0.25
                color: "#FFFFFF"
                opacity: 0.58
            }

            Rectangle {
                id: topRightGlow
                width: parent.width * 0.9
                height: width
                radius: width / 2
                x: parent.width * 0.55
                y: -height * 0.18
                color: appStateManager ? appStateManager.themeTintMid : "#F7D5E4"
                opacity: 0.42
            }

            Rectangle {
                id: bottomGlow
                width: parent.width * 1.05
                height: width
                radius: width / 2
                x: parent.width * 0.08
                y: parent.height - height * 0.36
                color: appStateManager ? appStateManager.themeTintDeep : "#DE9BB8"
                opacity: 0.18
            }

        }
    }


    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 28

        Grid {
            id: topBar

            width: parent.width
            height: 46

            columns: 3
            rows: 1
            columnSpacing: 0

            Item {
                width: 46
                height: 46

                WRBackArrow {
                    anchors.fill: parent
                    visible: root.showBackButton

                    // Back navigation is owned by AppStateManager (see
                    // AppStateManager::goBack). There is no Felgo
                    // NavigationStack in Main.qml — referencing
                    // `navigationStack` here used to throw
                    // ReferenceError on screen creation, which on
                    // Android (Qt 6.8 / RHI) is enough to leave the
                    // window on the white pre-render frame. Guard the
                    // call so the component still loads when
                    // appStateManager is missing (e.g. Live App).
                    onClicked: {
                        if (typeof appStateManager !== "undefined"
                                && appStateManager
                                && appStateManager.goBack)
                            appStateManager.goBack()
                    }
                }
            }

            Text {
                width: topBar.width - 92
                height: 46

                text: root.screenTitle
                font.pixelSize: 42
                font.weight: Font.Bold
                color: appStateManager ? appStateManager.themeTextPrimary : "#34101f"
                renderType: Qt.platform.os === "android"
                            ? Text.NativeRendering
                            : Text.QtRendering

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Item {
                width: 46
                height: 46
            }
        }

        Item {
            id: contentItem

            width: parent.width
            height: parent.height - topBar.height - parent.spacing
        }
    }
}
