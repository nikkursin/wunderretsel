import QtQuick
import QtQuick.Layouts
import "../Components"

WRScreen {
    id: root

    signal backClicked()
    signal imageClicked(int index)

    showBackButton: true
    onBackClicked: root.backClicked()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 16

        WRProgressCard {
            Layout.fillWidth: true
        }

        WRCard {
            id: galleryGridCard
            Layout.fillWidth: true
            Layout.fillHeight: true  // now actually works
            interactive: false

            property int itemCount: 24
            property int columnsCount: 3
            property int gap: 12
            property int innerPadding: 16
            property real tileSize: (width - innerPadding * 2 - gap * (columnsCount - 1)) / columnsCount

            Flickable {
                id: flick
                anchors.fill: parent
                anchors.margins: galleryGridCard.innerPadding
                contentWidth: width
                contentHeight: grid.implicitHeight
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                Grid {
                    id: grid
                    width: flick.width
                    columns: galleryGridCard.columnsCount
                    spacing: galleryGridCard.gap

                    Repeater {
                        model: galleryGridCard.itemCount
                        WRGalleryTile {
                            width: galleryGridCard.tileSize
                            height: galleryGridCard.tileSize
                            imageSource: "qrc:/assets/images/female/sample.jpg"
                            unlocked: index % 2 === 0
                            blurred: !unlocked
                            onClicked: root.imageClicked(index)
                        }
                    }
                }
            }
        }

        Row {
             spacing: 8

             Rectangle {
                 width: 14
                 height: 14
                 radius: 7
                 color: Qt.rgba(0.86, 0.24, 0.52, 0.16)

                 Rectangle {
                     anchors.centerIn: parent
                     width: 6
                     height: 6
                     radius: 3
                     color: "#d94b86"
                 }
             }

             Text {
                 text: "Progress grows after completed puzzles"
                 font.pixelSize: 13
                 font.weight: Font.DemiBold
                 color: Qt.rgba(0.23, 0.09, 0.15, 0.62)
             }
        }
    }
}
