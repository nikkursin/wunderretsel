import QtQuick
import QtQuick.Effects

// Single gallery cell. The visual treatment differs sharply between
// locked and unlocked states so the player always knows which images
// they have already discovered:
//
//   unlocked  → crisp full-colour photo, soft white border, hover lift
//   locked    → live Gaussian-blurred preview tinted rose + a lock pill
//
// The tile takes its size and image source from the parent; nothing
// here knows about the gallery's filter or unlock pool.
Item {
    id: root

    property string imageSource: ""
    property bool   unlocked: false

    signal clicked()

    // ── Backing rounded card ─────────────────────────────────────────
    Rectangle {
        id: tile
        anchors.fill: parent
        radius: Math.min(22, root.width * 0.18)
        clip: true

        // Soft rose placeholder colour shown until the image loads
        // (or as a base under the blurred-locked overlay).
        color: "#f4dde6"

        border.width: 1
        border.color: root.unlocked
                      ? Qt.rgba(1, 1, 1, 0.85)
                      : Qt.rgba(91/255, 25/255, 56/255, 0.10)
        Behavior on border.color { ColorAnimation { duration: 180 } }

        // The actual photograph. We always render it; the blur and
        // tint sit on top for the locked state. This keeps the layout
        // identical between states so toggling unlock is just a fade,
        // not a layout shuffle.
        Image {
            id: photo
            anchors.fill: parent
            source: root.imageSource
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            smooth: true
            cache: true
            visible: status === Image.Ready
        }

        // Live blur for the locked state. We use MultiEffect's
        // blurEnabled (Qt 6 friendly) bound to !unlocked so unlocking
        // animates from blurred → sharp. Saturation is dropped a bit
        // so locked thumbnails read as "ghosted" without going grey.
        MultiEffect {
            anchors.fill: photo
            source: photo
            blurEnabled: !root.unlocked
            blur: 1.0
            blurMax: 32
            saturation: root.unlocked ? 0.0 : -0.35
            brightness: root.unlocked ? 0.0 : -0.05
            visible: !root.unlocked && photo.status === Image.Ready
            Behavior on blur { NumberAnimation { duration: 220 } }
        }

        // Rose veil — adds the brand colour over the blurred preview
        // and fades out fully when the tile becomes unlocked.
        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Qt.rgba(220/255, 70/255, 130/255, 0.22)
            opacity: root.unlocked ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: 240 } }
        }

        // Lock pill — visible only on locked tiles. Sits dead-centre
        // and uses the same rose palette as the rest of the app so
        // locked tiles still feel "on-brand", not error-y.
        Rectangle {
            id: lockPill
            anchors.centerIn: parent
            width:  Math.max(28, root.width * 0.32)
            height: width
            radius: width / 2
            visible: !root.unlocked
            color: Qt.rgba(1, 1, 1, 0.88)
            border.width: 1
            border.color: Qt.rgba(159/255, 47/255, 97/255, 0.22)

            Text {
                anchors.centerIn: parent
                // Unicode "lock" glyph (U+1F512). Renders fine on
                // every desktop / mobile font we ship with Felgo;
                // falls back to a glyph box only on truly empty
                // fonts, which is acceptable.
                text: "\uD83D\uDD12"
                font.pixelSize: lockPill.width * 0.5
                color: "#9f2f61"
            }
        }

        // Hover/press affordance — only meaningful when unlocked,
        // since locked tiles don't accept clicks. Scaling the
        // Rectangle (not the Item) keeps the shadow filter on the
        // Item from being affected.
        scale: root.unlocked
               ? (mouseArea.pressed ? 0.97 : (mouseArea.containsMouse ? 1.03 : 1.0))
               : 1.0
        Behavior on scale {
            NumberAnimation { duration: 140; easing.type: Easing.OutCubic }
        }
    }

    // Drop shadow under the card for an unlocked, "card-y" feel.
    // We disable it for locked tiles so they recede into the grid.
    layer.enabled: root.unlocked
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor:    "#553a0f22"
        shadowOpacity:  0.32
        shadowBlur:     0.7
        shadowVerticalOffset: 4
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        enabled: root.unlocked
        hoverEnabled: true
        cursorShape: root.unlocked ? Qt.PointingHandCursor : Qt.ArrowCursor
        onClicked: root.clicked()
    }
}
