import QtQuick
import QtQuick.Layouts
import QtQuick.Effects
import Felgo
import "../Components"

// Use AppPage so we fully control the background and top bar,
// without WRScreen's decorative blob layer consuming vertical space.
AppPage {
    id: root

    // ── Public signals ───────────────────────────────────────────────────────
    signal nextPuzzleClicked()
    signal galleryClicked()
    signal backClicked()

    navigationBarHidden: true   // we draw our own minimal top bar

    // ── Puzzle data ──────────────────────────────────────────────────────────
    // Bind words / letters / grid to AppStateManager / PuzzleGenerator later.
    property var puzzleData: ({
        words:    ["ROSE", "STONE", "NOTE", "TONE"],
        letters:  ["R", "O", "S", "T", "N", "E"],
        // 7 × 7 flat grid:  "" = invisible gap | "_" = unsolved cell | letter = cell value
        grid: [
            "", "", "R", "O", "S", "E", "",
            "", "", "_", "", "_", "", "",
            "S", "T", "O", "N", "E", "", "",
            "", "", "_", "", "_", "", "",
            "", "N", "O", "T", "E", "", "",
            "", "_", "", "", "_", "", "",
            "", "T", "O", "N", "E", "", ""
        ],
        wordRows: [0, 2, 4, 6]
    })

    // Letters live in their own property so shuffling doesn't mutate puzzleData.
    property var letters: puzzleData.letters.slice()

    // ── Game state ───────────────────────────────────────────────────────────
    property var  solvedWords:     []
    property var  currentPath:     []
    property bool isDragging:      false
    property bool puzzleCompleted: solvedWords.length === puzzleData.words.length

    property bool shaking:  false
    property bool flashing: false

    // ── Helpers ──────────────────────────────────────────────────────────────
    function isWordSolved(wordIndex) {
        return solvedWords.indexOf(puzzleData.words[wordIndex]) !== -1
    }

    function isSolvedCell(cellIndex) {
        var row = Math.floor(cellIndex / 7)
        for (var wi = 0; wi < puzzleData.wordRows.length; wi++) {
            if (puzzleData.wordRows[wi] === row && isWordSolved(wi))
                return true
        }
        return false
    }

    function currentWord() {
        var w = ""
        for (var i = 0; i < currentPath.length; i++)
            w += letters[currentPath[i]]
        return w
    }

    function clearPath() {
        currentPath = []
        isDragging  = false
        if (dragCanvas.available) dragCanvas.requestPaint()
    }

    function shuffleLetters() {
        var copy = letters.slice()
        for (var i = copy.length - 1; i > 0; i--) {
            var j   = Math.floor(Math.random() * (i + 1))
            var tmp = copy[i]; copy[i] = copy[j]; copy[j] = tmp
        }
        letters = copy
        clearPath()
    }

    function submitWord() {
        var word        = currentWord().toUpperCase()
        var isValid     = puzzleData.words.indexOf(word) !== -1
        var alreadyDone = solvedWords.indexOf(word) !== -1

        if (isValid && !alreadyDone) {
            flashing    = true
            flashTimer.restart()
            solvedWords = solvedWords.concat([word])
        } else if (currentPath.length > 0) {
            shaking = true
            shakeTimer.restart()
        }
        clearPath()
    }

    function resetPuzzle() {
        solvedWords = []
        letters = puzzleData.letters.slice()
        clearPath()
    }

    function hitTestLetter(mx, my) {
        for (var i = 0; i < letterRepeater.count; i++) {
            var item = letterRepeater.itemAt(i)
            if (!item) continue
            var dx = mx - (item.x + item.width  / 2)
            var dy = my - (item.y + item.height / 2)
            if ((dx * dx + dy * dy) < Math.pow(item.width / 2 + dp(8), 2))
                return i
        }
        return -1
    }

    // ── Timers ───────────────────────────────────────────────────────────────
    Timer { id: shakeTimer; interval: 440; onTriggered: root.shaking  = false }
    Timer { id: flashTimer; interval: 540; onTriggered: root.flashing = false }

    // ── Full-screen photo background with rose-tinted reveal ─────────────────
    // The image fills the entire AppPage. A base rose-tint Rectangle keeps the
    // foreground UI readable at all times. The 2 × 2 reveal grid covers the
    // whole screen with extra rose tint in unsolved quadrants and fades to
    // fully transparent (showing more of the photo) when each word is solved.
    Item {
        anchors.fill: parent
        z: -1

        Image {
            id: bgImage
            anchors.fill: parent
            source: "qrc:/assets/images/female/sample.jpg"
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            visible: status === Image.Ready
        }

        // Fallback rose gradient when the image isn't available
        Rectangle {
            anchors.fill: parent
            visible: !bgImage.visible
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0;  color: "#fff0f6" }
                GradientStop { position: 0.55; color: "#f8d7e8" }
                GradientStop { position: 1.0;  color: "#e8a8c8" }
            }
        }

        // Base rose tint — always present so the crossword/wheel are readable
        Rectangle {
            anchors.fill: parent
            color: Qt.rgba(1, 0.94, 0.96, 0.50)
        }

        // 2 × 2 reveal segments — each unsolved word adds extra rose haze
        // over its quadrant; solving the word fades that quadrant clear so
        // more of the photo shows through.
        Grid {
            id: revealGrid
            anchors.fill: parent
            columns: 2
            rows: 2

            Repeater {
                model: puzzleData.words.length

                Rectangle {
                    width:  Math.max(0, revealGrid.width  / 2)
                    height: Math.max(0, revealGrid.height / 2)
                    color: root.isWordSolved(index)
                           ? "transparent"
                           : Qt.rgba(1, 0.96, 0.97, 0.85)
                    Behavior on color { ColorAnimation { duration: 480 } }
                }
            }
        }

        // Bottom-down gradient that subtly darkens the lower half so the
        // letter wheel and controls have enough contrast against the photo.
        Rectangle {
            anchors.fill: parent
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop { position: 0.0;  color: Qt.rgba(1, 1, 1, 0) }
                GradientStop { position: 0.55; color: Qt.rgba(0.97, 0.84, 0.91, 0.10) }
                GradientStop { position: 1.0;  color: Qt.rgba(0.91, 0.66, 0.78, 0.32) }
            }
        }
    }

    // ── Content frame ────────────────────────────────────────────────────────
    // Constrains the playable UI to a phone-shaped area, centered on the page.
    // On a phone this fills the screen; on a big desktop / external monitor it
    // stays a sensible size so the crossword and wheel don't sprawl absurdly
    // wide and the photo background shows around the edges.
    Item {
        id: contentFrame
        anchors.centerIn: parent
        width:  Math.min(parent.width,  dp(440))
        height: Math.min(parent.height, dp(820))

    // ── Master ColumnLayout ──────────────────────────────────────────────────
    ColumnLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.topMargin:    dp(4)
        anchors.bottomMargin: dp(8)
        anchors.leftMargin:   dp(12)
        anchors.rightMargin:  dp(12)
        spacing: dp(6)

        // ── Top bar ───────────────────────────────────────────────────────
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: dp(44)

            // Back button (round translucent chip with shadow)
            Rectangle {
                id: backChip
                width: dp(36); height: dp(36)
                radius: width / 2
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                color: backArea.pressed
                       ? Qt.rgba(1, 1, 1, 0.65)
                       : Qt.rgba(1, 1, 1, 0.84)
                Behavior on color { ColorAnimation { duration: 140 } }

                Text {
                    anchors.centerIn: parent
                    anchors.horizontalCenterOffset: -1
                    text: "‹"
                    font.pixelSize: dp(22)
                    font.weight: Font.DemiBold
                    color: "#401425"
                }

                MouseArea {
                    id: backArea
                    anchors.fill: parent
                    onClicked: {
                        if (typeof appStateManager !== "undefined" && appStateManager.goHome)
                            appStateManager.goHome()
                        else
                            root.backClicked()
                    }
                }
            }
            // Soft drop shadow under the back chip
            MultiEffect {
                anchors.fill: backChip
                source: backChip
                shadowEnabled: true
                shadowColor: "#502A0F1F"
                shadowOpacity: 0.5
                shadowBlur: 0.6
                shadowVerticalOffset: 3
            }

            // Title — anchored between the back chip and the right edge so it
            // never overlaps surrounding chrome, with auto font shrinking.
            Text {
                anchors.left:  backChip.right
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin:  dp(8)
                anchors.rightMargin: dp(36)   // visual balance for back chip
                text: "Wunderrätsel"
                horizontalAlignment: Text.AlignHCenter
                fontSizeMode: Text.HorizontalFit
                minimumPixelSize: dp(14)
                font.pixelSize: dp(20)
                font.weight: Font.Black
                font.letterSpacing: -dp(0.4)
                color: "#3a0f22"
                elide: Text.ElideRight
            }
        }

        // ── Crossword ─────────────────────────────────────────────────────
        Item {
            id: gridWrapper
            Layout.fillWidth: true
            Layout.preferredHeight: xGrid.totalH + dp(4)
            visible: !root.puzzleCompleted

            Grid {
                id: xGrid
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter

                columns: 7
                rows:    7

                readonly property real gap: dp(3)
                // Cell size: clamped by available width AND by page height so
                // the 7-row grid never crowds out the wheel — height factor
                // tuned so the whole crossword takes ~22 % of page height.
                readonly property real cellSz:
                    Math.max(dp(10),
                             Math.min(dp(26),
                                      Math.min((gridWrapper.width - gap * 6) / 7,
                                               root.height * 0.026)))
                readonly property real totalH: cellSz * 7 + gap * 6

                columnSpacing: gap
                rowSpacing:    gap

                Repeater {
                    model: puzzleData.grid.length

                    Item {
                        id: cellHolder
                        readonly property string cv:    puzzleData.grid[index]
                        readonly property bool   isGap:  cv === ""
                        readonly property bool   isBlank: cv === "_"
                        readonly property bool   solved:  root.isSolvedCell(index)

                        width:  xGrid.cellSz
                        height: xGrid.cellSz
                        visible: !isGap

                        Rectangle {
                            id: cellBg
                            anchors.fill: parent
                            radius: xGrid.cellSz * 0.24
                            border.width: 1.2
                            border.color: cellHolder.solved
                                          ? Qt.rgba(220/255, 70/255, 130/255, 0.55)
                                          : Qt.rgba(91/255, 25/255, 56/255, 0.14)

                            color: cellHolder.solved
                                   ? "#fff0f6"
                                   : Qt.rgba(1, 1, 1, 0.92)
                            Behavior on color { ColorAnimation { duration: 320 } }

                            // Solved cells get a soft pink gradient on top
                            Rectangle {
                                anchors.fill: parent
                                radius: parent.radius
                                visible: cellHolder.solved
                                border.width: 0
                                gradient: Gradient {
                                    orientation: Gradient.Vertical
                                    GradientStop { position: 0.0; color: "#fff0f6" }
                                    GradientStop { position: 1.0; color: "#ffd4e8" }
                                }
                            }

                            // Pop animation on solve — proper sequential bounce
                            SequentialAnimation {
                                id: popAnim
                                NumberAnimation {
                                    target: cellBg; property: "scale"
                                    from: 0.55; to: 1.08
                                    duration: 200; easing.type: Easing.OutQuad
                                }
                                NumberAnimation {
                                    target: cellBg; property: "scale"
                                    from: 1.08; to: 1.0
                                    duration: 140; easing.type: Easing.OutQuad
                                }
                            }
                            Connections {
                                target: cellHolder
                                function onSolvedChanged() {
                                    if (cellHolder.solved) popAnim.restart()
                                }
                            }

                            Text {
                                anchors.centerIn: parent
                                text: (cellHolder.solved && !cellHolder.isBlank) ? cellHolder.cv : ""
                                font.pixelSize: xGrid.cellSz * 0.5
                                font.weight: Font.Black
                                color: "#b03268"
                            }
                        }
                    }
                }
            }
        }

        // ── Word-badge row ────────────────────────────────────────────────
        Row {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: dp(20)
            visible: !root.puzzleCompleted
            spacing: dp(5)

            Repeater {
                model: puzzleData.words.length

                Rectangle {
                    readonly property bool done: root.isWordSolved(index)

                    height: dp(20)
                    width:  bt.implicitWidth + dp(14)
                    radius: height / 2
                    color: done ? Qt.rgba(239/255, 79/255, 145/255, 0.34)
                                : Qt.rgba(1, 1, 1, 0.72)
                    border.width: 1
                    border.color: done ? Qt.rgba(239/255, 79/255, 145/255, 0.55)
                                       : Qt.rgba(159/255, 47/255, 97/255, 0.22)
                    Behavior on color { ColorAnimation { duration: 360 } }

                    Text {
                        id: bt
                        anchors.centerIn: parent
                        text: {
                            if (done) return puzzleData.words[index]
                            var s = ""; var n = puzzleData.words[index].length
                            for (var i = 0; i < n; i++) s += (i > 0 ? " " : "") + "•"
                            return s
                        }
                        font.pixelSize: dp(11)
                        font.weight: Font.Black
                        font.letterSpacing: dp(0.6)
                        color: done ? "#b03268" : Qt.rgba(159/255, 47/255, 97/255, 0.62)
                    }
                }
            }
        }

        // ── Word preview (big floating letters, like Words of Wonders) ────
        Item {
            id: previewArea
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: dp(34)
            Layout.preferredWidth: Math.max(dp(140), pvText.implicitWidth + dp(40))
            visible: !root.puzzleCompleted

            // Inner shaker — we animate THIS x-offset, never previewArea.x,
            // because previewArea is positioned by ColumnLayout.
            Item {
                id: shaker
                anchors.fill: parent
                anchors.horizontalCenterOffset: 0

                SequentialAnimation on x {
                    running: root.shaking; loops: 1
                    NumberAnimation { to:  dp(8);  duration: 55 }
                    NumberAnimation { to: -dp(8);  duration: 55 }
                    NumberAnimation { to:  dp(5);  duration: 45 }
                    NumberAnimation { to: -dp(5);  duration: 45 }
                    NumberAnimation { to:  0;      duration: 40 }
                }

                // Soft pill that only appears when text exists or feedback is firing.
                Rectangle {
                    id: previewPill
                    anchors.fill: parent
                    radius: height / 2
                    opacity: (root.currentPath.length > 0 || root.shaking || root.flashing) ? 1 : 0
                    Behavior on opacity { NumberAnimation { duration: 120 } }

                    color: root.shaking  ? Qt.rgba(1.0, 0.82, 0.82, 0.92)
                         : root.flashing ? Qt.rgba(0.78, 1.0, 0.86, 0.92)
                         :                 Qt.rgba(1, 1, 1, 0.82)
                    border.width: 1
                    border.color: root.shaking  ? Qt.rgba(0.88, 0.28, 0.28, 0.45)
                                : root.flashing ? Qt.rgba(0.18, 0.72, 0.38, 0.45)
                                :                 Qt.rgba(159/255, 47/255, 97/255, 0.18)

                    Behavior on color        { ColorAnimation { duration: 80 } }
                    Behavior on border.color { ColorAnimation { duration: 80 } }
                }

                Text {
                    id: pvText
                    anchors.centerIn: parent
                    text: root.currentPath.length > 0
                          ? root.currentWord().split("").join(" ")
                          : " "
                    font.pixelSize: dp(18)
                    font.weight: Font.Black
                    font.letterSpacing: dp(3)
                    color: root.shaking  ? "#b03030"
                         : root.flashing ? "#1a7a40"
                         :                 "#b83268"
                    Behavior on color { ColorAnimation { duration: 80 } }
                }
            }
        }

        // ── Letter wheel ──────────────────────────────────────────────────
        // Container absorbs all remaining vertical space so the wheel is always
        // visible — no matter how tall or short the page is.
        Item {
            id: wheelContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !root.puzzleCompleted
            Layout.minimumHeight: dp(140)

            Item {
                id: wheel
                anchors.centerIn: parent

                // Square wheel sized to fit available area, with sane caps.
                readonly property real diameter:
                    Math.max(dp(140),
                             Math.min(dp(260),
                                      Math.min(parent.width, parent.height) - dp(4)))
                readonly property real wR:     diameter / 2
                readonly property real orbitR: diameter * 0.36

                width:  diameter
                height: diameter

            // ── Bowl: large translucent rose circle behind the letters ──
            Rectangle {
                id: bowl
                anchors.centerIn: parent
                width: wheel.diameter; height: width; radius: width / 2
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.0; color: Qt.rgba(255/255, 230/255, 240/255, 0.78) }
                    GradientStop { position: 1.0; color: Qt.rgba(220/255, 116/255, 160/255, 0.62) }
                }
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.65)
            }
            // Soft glow / shadow under the bowl
            MultiEffect {
                anchors.fill: bowl
                source: bowl
                shadowEnabled: true
                shadowColor: "#553a0f22"
                shadowOpacity: 0.50
                shadowBlur: 1.0
                shadowVerticalOffset: 6
            }

            // Inner highlight ring
            Rectangle {
                anchors.centerIn: parent
                width: wheel.diameter * 0.86; height: width; radius: width / 2
                color: "transparent"
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.30)
            }

            // Drag-line canvas (above bowl, below letter buttons)
            Canvas {
                id: dragCanvas
                anchors.fill: parent
                z: 1
                renderTarget: Canvas.FramebufferObject

                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)
                    if (root.currentPath.length < 1) return

                    // Soft outer glow stroke
                    ctx.strokeStyle = "rgba(239,79,145,0.30)"
                    ctx.lineWidth   = dp(11)
                    ctx.lineCap     = "round"
                    ctx.lineJoin    = "round"
                    ctx.beginPath()
                    for (var i = 0; i < root.currentPath.length; i++) {
                        var b = letterRepeater.itemAt(root.currentPath[i])
                        if (!b) continue
                        var cx = b.x + b.width  / 2
                        var cy = b.y + b.height / 2
                        if (i === 0) ctx.moveTo(cx, cy)
                        else         ctx.lineTo(cx, cy)
                    }
                    ctx.stroke()

                    // Crisp inner stroke
                    ctx.strokeStyle = "rgba(223,63,131,0.85)"
                    ctx.lineWidth   = dp(5)
                    ctx.beginPath()
                    for (var j = 0; j < root.currentPath.length; j++) {
                        var bb = letterRepeater.itemAt(root.currentPath[j])
                        if (!bb) continue
                        var ccx = bb.x + bb.width  / 2
                        var ccy = bb.y + bb.height / 2
                        if (j === 0) ctx.moveTo(ccx, ccy)
                        else         ctx.lineTo(ccx, ccy)
                    }
                    ctx.stroke()

                    // Dot at each visited node
                    ctx.fillStyle = "rgba(223,63,131,0.90)"
                    for (var k = 0; k < root.currentPath.length; k++) {
                        var bn = letterRepeater.itemAt(root.currentPath[k])
                        if (!bn) continue
                        var dx = bn.x + bn.width  / 2
                        var dy = bn.y + bn.height / 2
                        ctx.beginPath()
                        ctx.arc(dx, dy, dp(4), 0, Math.PI * 2)
                        ctx.fill()
                    }
                }
            }

            // Letter buttons
            Repeater {
                id: letterRepeater
                model: root.letters

                Item {
                    id: lb
                    readonly property real aRad:
                        (-Math.PI / 2) + index * 2 * Math.PI / Math.max(1, root.letters.length)
                    readonly property bool active:  root.currentPath.indexOf(index) !== -1
                    readonly property real btnSz:
                        Math.max(dp(38), Math.min(dp(56), wheel.diameter * 0.235))

                    width: btnSz; height: btnSz
                    z: 2

                    x: wheel.wR + Math.cos(aRad) * wheel.orbitR - btnSz / 2
                    y: wheel.wR + Math.sin(aRad) * wheel.orbitR - btnSz / 2

                    Rectangle {
                        id: btnBg
                        anchors.fill: parent
                        radius: width / 2
                        color: Qt.rgba(1, 1, 1, 0.94)
                        border.width: lb.active ? 0 : 1.2
                        border.color: Qt.rgba(91/255, 25/255, 56/255, 0.10)

                        // Pink gradient layer (visible only when active)
                        Rectangle {
                            anchors.fill: parent
                            radius: parent.radius
                            visible: lb.active
                            border.width: 0
                            gradient: Gradient {
                                orientation: Gradient.Vertical
                                GradientStop { position: 0; color: "#ef4f91" }
                                GradientStop { position: 1; color: "#d93279" }
                            }
                        }

                        Text {
                            anchors.centerIn: parent
                            text: modelData
                            font.pixelSize: lb.btnSz * 0.42
                            font.weight: Font.Black
                            color: lb.active ? "#ffffff" : "#3c172b"
                            Behavior on color { ColorAnimation { duration: 110 } }
                        }
                    }

                    // Drop shadow on every letter button for that premium WoW look
                    MultiEffect {
                        anchors.fill: btnBg
                        source: btnBg
                        shadowEnabled: true
                        shadowColor: lb.active ? "#80c4307a" : "#603a0f22"
                        shadowOpacity: lb.active ? 0.65 : 0.45
                        shadowBlur: 0.75
                        shadowVerticalOffset: lb.active ? 6 : 4
                        Behavior on shadowOpacity { NumberAnimation { duration: 110 } }
                    }

                    scale: lb.active ? 1.13 : 1.0
                    Behavior on scale { NumberAnimation { duration: 110; easing.type: Easing.OutQuad } }
                }
            }

            // Single MouseArea drives the whole drag interaction
            MouseArea {
                anchors.fill: parent
                z: 3

                onPressed: function(mouse) {
                    root.clearPath()
                    root.isDragging = true
                    var hit = root.hitTestLetter(mouse.x, mouse.y)
                    if (hit >= 0) {
                        root.currentPath = [hit]
                        dragCanvas.requestPaint()
                    }
                }
                onPositionChanged: function(mouse) {
                    if (!root.isDragging) return
                    var hit = root.hitTestLetter(mouse.x, mouse.y)
                    if (hit >= 0 && root.currentPath.indexOf(hit) === -1) {
                        root.currentPath = root.currentPath.concat([hit])
                        dragCanvas.requestPaint()
                    }
                }
                onReleased: {
                    if (root.currentPath.length > 0) root.submitWord()
                    else                             root.clearPath()
                }
                onCanceled: root.clearPath()
            }
            } // end inner wheel Item
        } // end wheelContainer

        // ── Controls (Clear / Shuffle / Submit) ───────────────────────────
        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: dp(2)
            visible: !root.puzzleCompleted
            spacing: dp(8)

            // Clear (secondary, soft)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: dp(38)
                radius: dp(14)
                color: clearArea.pressed
                       ? Qt.rgba(1, 1, 1, 0.70)
                       : Qt.rgba(1, 1, 1, 0.86)
                border.width: 1
                border.color: Qt.rgba(159/255, 47/255, 97/255, 0.22)
                Behavior on color { ColorAnimation { duration: 100 } }

                Text {
                    anchors.centerIn: parent
                    text: "Clear"
                    font.pixelSize: dp(15)
                    font.weight: Font.Black
                    color: "#9f2f61"
                }
                MouseArea {
                    id: clearArea
                    anchors.fill: parent
                    onClicked: root.clearPath()
                }
            }

            // Shuffle (icon + label, secondary)
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: dp(38)
                radius: dp(14)
                color: shuffleArea.pressed
                       ? Qt.rgba(1, 1, 1, 0.70)
                       : Qt.rgba(1, 1, 1, 0.86)
                border.width: 1
                border.color: Qt.rgba(159/255, 47/255, 97/255, 0.22)
                Behavior on color { ColorAnimation { duration: 100 } }

                Row {
                    anchors.centerIn: parent
                    spacing: dp(6)
                    Text {
                        text: "↻"
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: dp(18)
                        font.weight: Font.Black
                        color: "#9f2f61"
                        rotation: shuffleArea.pressed ? 180 : 0
                        Behavior on rotation {
                            NumberAnimation { duration: 280; easing.type: Easing.OutCubic }
                        }
                    }
                    Text {
                        text: "Shuffle"
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: dp(15)
                        font.weight: Font.Black
                        color: "#9f2f61"
                    }
                }
                MouseArea {
                    id: shuffleArea
                    anchors.fill: parent
                    onClicked: root.shuffleLetters()
                }
            }

            // Submit (primary, gradient, with shadow)
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: dp(38)
                opacity: root.currentPath.length > 0 ? 1.0 : 0.5
                Behavior on opacity { NumberAnimation { duration: 120 } }

                Rectangle {
                    id: submitBg
                    anchors.fill: parent
                    radius: dp(14)
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop { position: 0; color: "#ef4f91" }
                        GradientStop { position: 1; color: "#d93279" }
                    }
                    scale: submitArea.pressed ? 0.97 : 1.0
                    Behavior on scale { NumberAnimation { duration: 110 } }

                    Text {
                        anchors.centerIn: parent
                        text: "Submit"
                        font.pixelSize: dp(15)
                        font.weight: Font.Black
                        color: "white"
                    }

                    MouseArea {
                        id: submitArea
                        anchors.fill: parent
                        enabled: root.currentPath.length > 0
                        onClicked: root.submitWord()
                    }
                }
                MultiEffect {
                    anchors.fill: submitBg
                    source: submitBg
                    shadowEnabled: true
                    shadowColor: "#90c4307a"
                    shadowOpacity: 0.55
                    shadowBlur: 0.8
                    shadowVerticalOffset: 5
                }
            }
        }

        // ── Completion card ───────────────────────────────────────────────
        WRCard {
            Layout.fillWidth: true
            Layout.preferredHeight: dp(118)
            visible: root.puzzleCompleted
            interactive: false

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: dp(16)
                spacing: dp(10)

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "🎉  Puzzle Completed!"
                    font.pixelSize: dp(18)
                    font.weight: Font.Black
                    color: "#35111f"
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: dp(10)

                    WRButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: dp(42)
                        text: "Play again"
                        onClicked: {
                            root.resetPuzzle()
                            root.nextPuzzleClicked()
                        }
                    }
                    WRButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: dp(42)
                        text: "Gallery"
                        onClicked: {
                            if (typeof appStateManager !== "undefined" &&
                                    appStateManager.goGallery)
                                appStateManager.goGallery()
                            else
                                root.galleryClicked()
                        }
                    }
                }
            }
        }
    }
    } // end contentFrame
}
