import QtQuick
import QtQuick.Layouts
import "../Components"

// Gallery view. Strict MVVM: this file owns *only* presentation. The
// list of images, their unlocked state, and both counters all come
// from `appStateManager` (see AppStateManager::galleryImages).
WRScreen {
    id: root

    // Emitted when the user taps an unlocked tile. Back navigation is handled
    // globally by WRBackArrow → appStateManager.goBack(); do not duplicate
    // here (there is no WRScreen.backClicked signal, so `onBackClicked`
    // bindings are invalid on this root).
    signal imageClicked(int imageId, string imageSource)

    showBackButton: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 14

        // ── Gallery card ─────────────────────────────────────────────
        // Header row (title + counter badge) followed by a flickable
        // grid of tiles. The whole card stretches to fill the page.
        WRCard {
            id: galleryCard
            Layout.fillWidth:  true
            Layout.fillHeight: true
            interactive: false

            // Grid tunables — kept on the card so they're easy to find.
            readonly property int columnsCount: 3
            readonly property int gap: 12
            readonly property int innerPadding: 18
            readonly property real availableWidth:
                width - innerPadding * 2
            readonly property real tileSize:
                Math.max(48,
                         (availableWidth - gap * (columnsCount - 1))
                         / columnsCount)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: galleryCard.innerPadding
                spacing: 16

                // ── Header: title (left) + counter pill (right) ──
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Column {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignVCenter
                        spacing: 4

                        Text {
                            text: qsTr("Gallery")
                            font.pixelSize: 24
                            font.weight: Font.Bold
                            color: appStateManager ? appStateManager.themeTextPrimary : "#35111f"
                        }
                        Text {
                            text: qsTr("Discovered images")
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                            color: appStateManager ? appStateManager.themeTextSecondary : Qt.rgba(0.42, 0.23, 0.31, 0.72)
                        }
                    }

                    // Counter pill — pinned to the right side of the
                    // card header. Same gradient and proportions as the
                    // home-screen badge so the two screens read as one
                    // visual system.
                    Rectangle {
                        id: counterBadge
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                        width:  78
                        height: 78
                        radius: 24

                        gradient: Gradient {
                            GradientStop { position: 0.0; color: appStateManager ? appStateManager.themeAccentSoftStart : "#fff4f9" }
                            GradientStop { position: 1.0; color: appStateManager ? appStateManager.themeAccentSoftEnd : "#f58ab6" }
                        }
                        border.width: 1
                        border.color: appStateManager ? appStateManager.themeTileBorder : Qt.rgba(0.45, 0.15, 0.3, 0.10)

                        Text {
                            anchors.centerIn: parent
                            text: (appStateManager
                                       ? appStateManager.unlockedImagesCount
                                       : 0)
                                  + "/"
                                  + (appStateManager
                                       ? appStateManager.totalImagesCount
                                       : 0)
                            font.pixelSize: 19
                            font.weight: Font.Black
                            color: appStateManager ? appStateManager.themeTextStrong : "#401425"
                        }
                    }
                }

                // ── Image grid ───────────────────────────────────
                Item {
                    Layout.fillWidth:  true
                    Layout.fillHeight: true

                    Flickable {
                        id: flick
                        anchors.fill: parent
                        contentWidth: width
                        contentHeight: imageGrid.height
                        clip: true
                        boundsBehavior: Flickable.StopAtBounds

                        Grid {
                            id: imageGrid
                            width: flick.width
                            columns: galleryCard.columnsCount
                            spacing: galleryCard.gap

                            Repeater {
                                // Pure-data model from C++: a list of
                                // { id, source, unlocked } maps. The
                                // Repeater rebuilds whenever AppStateManager
                                // emits galleryImagesChanged, which it
                                // does on init, preference change and
                                // every successful puzzle solve.
                                model: appStateManager
                                       ? appStateManager.galleryImages
                                       : []

                                WRGalleryTile {
                                    width:  galleryCard.tileSize
                                    height: galleryCard.tileSize
                                    imageSource: modelData.source
                                    unlocked:    modelData.unlocked
                                    onClicked: root.imageClicked(modelData.id,
                                                                 modelData.source)
                                }
                            }
                        }
                    }

                    // Empty-state hint — only shown when the storage
                    // layer returned no images at all (e.g. malformed
                    // images.json). Keeps the screen from looking
                    // broken if assets are missing.
                    Text {
                        anchors.centerIn: parent
                        visible: appStateManager
                                 && appStateManager.totalImagesCount === 0
                        text: qsTr("No images available yet.")
                        font.pixelSize: 14
                        color: Qt.rgba(0.23, 0.09, 0.15, 0.55)
                    }
                }
            }
        }

        // ── Footer hint ──────────────────────────────────────────────
        Row {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8

            Rectangle {
                width: 14; height: 14; radius: 7
                color: appStateManager ? appStateManager.themeTileVeil : Qt.rgba(0.86, 0.24, 0.52, 0.16)

                Rectangle {
                    anchors.centerIn: parent
                    width: 6; height: 6; radius: 3
                    color: appStateManager ? appStateManager.themeAccentStart : "#d94b86"
                }
            }

            Text {
                text: qsTr("Solve puzzles to unlock more images")
                font.pixelSize: 13
                font.weight: Font.DemiBold
                color: appStateManager ? appStateManager.themeTextSecondary : Qt.rgba(0.23, 0.09, 0.15, 0.62)
            }
        }
    }
}
