import QtQuick
import QtQuick.Controls
import "../components"
import ".."

Item {
    id: root

    implicitWidth: 456
    implicitHeight: 204

    property string selectedGame: "snake" // "snake", "2048", "flappy"

    // Request window focus when entering games so arrow keys and space work immediately
    Component.onCompleted: {
        gameKeySensor.forceActiveFocus()
        if (typeof mainWindow !== "undefined" && mainWindow && mainWindow.requestWindowFocus) {
            mainWindow.requestWindowFocus()
        }
    }

    // Top Game Selector Pills
    Row {
        id: gameHeader
        anchors.top: parent.top
        anchors.topMargin: 2
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 6

        Repeater {
            model: [
                { id: "snake", label: "🐍 Snake" },
                { id: "2048", label: "🔢 2048" },
                { id: "flappy", label: "🚀 Flappy" }
            ]

            Rectangle {
                width: 82
                height: 24
                radius: 12
                color: root.selectedGame === modelData.id ? Theme.accent : Qt.rgba(1.0, 1.0, 1.0, 0.08)
                border.color: root.selectedGame === modelData.id ? Qt.rgba(1.0, 1.0, 1.0, 0.3) : Qt.rgba(1.0, 1.0, 1.0, 0.06)
                border.width: 1

                scale: gameSelectorMouse.pressed ? 0.96 : (gameSelectorMouse.containsMouse ? 1.03 : 1.0)
                Behavior on scale { SpringAnimation { spring: 4.0; damping: 0.6 } }
                Behavior on color { ColorAnimation { duration: 140 } }

                Text {
                    anchors.centerIn: parent
                    text: modelData.label
                    color: root.selectedGame === modelData.id ? "#FFFFFF" : Theme.textSecondary
                    font.family: Theme.fontFamily
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }

                MouseArea {
                    id: gameSelectorMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.selectedGame = modelData.id
                        gameKeySensor.forceActiveFocus()
                    }
                }
            }
        }
    }

    // Game Container
    Item {
        id: gameContainer
        anchors.top: gameHeader.bottom
        anchors.topMargin: 4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 2
        anchors.left: parent.left
        anchors.leftMargin: 4
        anchors.right: parent.right
        anchors.rightMargin: 4

        // ═════════════════════════════════════════════════════
        // 1. SNAKE GAME
        // ═════════════════════════════════════════════════════
        Item {
            id: snakeGame
            anchors.fill: parent
            visible: root.selectedGame === "snake"

            property int gridCols: 22
            property int gridRows: 9
            property real cellW: width / gridCols
            property real cellH: height / gridRows

            property var snake: [{x: 8, y: 4}, {x: 7, y: 4}, {x: 6, y: 4}]
            property var food: ({x: 15, y: 4})
            property string dir: "RIGHT"
            property string nextDir: "RIGHT"
            property int score: 0
            property int highScore: 0
            property bool gameOver: false
            property bool running: false

            function startGame() {
                snake = [{x: 8, y: 4}, {x: 7, y: 4}, {x: 6, y: 4}]
                dir = "RIGHT"
                nextDir = "RIGHT"
                score = 0
                gameOver = false
                spawnFood()
                running = true
                snakeTimer.restart()
                snakeCanvas.requestPaint()
            }

            function spawnFood() {
                var fx = Math.floor(Math.random() * gridCols)
                var fy = Math.floor(Math.random() * gridRows)
                food = {x: fx, y: fy}
            }

            function step() {
                if (!running || gameOver) return
                dir = nextDir
                var head = {x: snake[0].x, y: snake[0].y}
                if (dir === "UP") head.y--
                else if (dir === "DOWN") head.y++
                else if (dir === "LEFT") head.x--
                else if (dir === "RIGHT") head.x++

                // Wall wrap or collision
                if (head.x < 0) head.x = gridCols - 1
                if (head.x >= gridCols) head.x = 0
                if (head.y < 0) head.y = gridRows - 1
                if (head.y >= gridRows) head.y = 0

                // Self collision
                for (var i = 0; i < snake.length; i++) {
                    if (snake[i].x === head.x && snake[i].y === head.y) {
                        gameOver = true
                        running = false
                        if (score > highScore) highScore = score
                        snakeCanvas.requestPaint()
                        return
                    }
                }

                snake.unshift(head)

                if (head.x === food.x && head.y === food.y) {
                    score += 10
                    spawnFood()
                } else {
                    snake.pop()
                }

                snakeCanvas.requestPaint()
            }

            Timer {
                id: snakeTimer
                interval: 110
                repeat: true
                running: snakeGame.running && !snakeGame.gameOver
                onTriggered: snakeGame.step()
            }

            Canvas {
                id: snakeCanvas
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    // Draw subtle grid dots
                    ctx.fillStyle = "rgba(255, 255, 255, 0.04)"
                    for (var r = 0; r < snakeGame.gridRows; r++) {
                        for (var c = 0; c < snakeGame.gridCols; c++) {
                            ctx.fillRect(c * snakeGame.cellW + snakeGame.cellW / 2 - 1, r * snakeGame.cellH + snakeGame.cellH / 2 - 1, 2, 2)
                        }
                    }

                    // Draw glowing apple food
                    var fx = snakeGame.food.x * snakeGame.cellW + 2
                    var fy = snakeGame.food.y * snakeGame.cellH + 2
                    var fw = snakeGame.cellW - 4
                    var fh = snakeGame.cellH - 4
                    ctx.fillStyle = "#FF453A"
                    ctx.beginPath()
                    ctx.arc(fx + fw / 2, fy + fh / 2, Math.min(fw, fh) / 2, 0, Math.PI * 2)
                    ctx.fill()

                    // Draw snake
                    for (var i = 0; i < snakeGame.snake.length; i++) {
                        var seg = snakeGame.snake[i]
                        var sx = seg.x * snakeGame.cellW + 1.5
                        var sy = seg.y * snakeGame.cellH + 1.5
                        var sw = snakeGame.cellW - 3
                        var sh = snakeGame.cellH - 3

                        ctx.fillStyle = (i === 0) ? (Theme.accent || "#0A84FF") : Qt.rgba(0.04, 0.72, 1.0, 0.75 - (i * 0.02))
                        ctx.beginPath()
                        ctx.roundRect(sx, sy, sw, sh, 4)
                        ctx.fill()
                    }
                }
            }

            // HUD
            Row {
                anchors.top: parent.top
                anchors.topMargin: 4
                anchors.left: parent.left
                anchors.leftMargin: 8
                spacing: 12

                Text {
                    text: "Score: " + snakeGame.score
                    color: "#FFFFFF"
                    font.family: Theme.fontMono
                    font.pixelSize: 11
                    font.bold: true
                }
                Text {
                    text: "Best: " + snakeGame.highScore
                    color: Theme.textSecondary
                    font.family: Theme.fontMono
                    font.pixelSize: 11
                }
            }

            // Overlay for Start / Game Over
            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(0, 0, 0, 0.75)
                visible: !snakeGame.running || snakeGame.gameOver
                radius: 12

                Column {
                    anchors.centerIn: parent
                    spacing: 6

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: snakeGame.gameOver ? "Game Over! Score: " + snakeGame.score : "Retro Arcade Snake"
                        color: snakeGame.gameOver ? Theme.danger : "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 13
                        font.bold: true
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Use Arrow Keys or WASD to navigate"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                    }
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 100
                        height: 26
                        radius: 13
                        color: Theme.accent
                        Text {
                            anchors.centerIn: parent
                            text: snakeGame.gameOver ? "Play Again" : "Start Game"
                            color: "#FFFFFF"
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                snakeGame.startGame()
                                gameKeySensor.forceActiveFocus()
                            }
                        }
                    }
                }
            }
        }

        // ═════════════════════════════════════════════════════
        // 2. 2048 PUZZLE GAME
        // ═════════════════════════════════════════════════════
        Item {
            id: g2048
            anchors.fill: parent
            visible: root.selectedGame === "2048"

            property var grid: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
            property int score: 0
            property int bestScore: 0

            function newGame() {
                var fresh = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
                grid = fresh
                score = 0
                spawnTile()
                spawnTile()
                gridRepeater.model = 0
                gridRepeater.model = 16
            }

            function spawnTile() {
                var empty = []
                for (var i = 0; i < 16; i++) {
                    if (grid[i] === 0) empty.push(i)
                }
                if (empty.length > 0) {
                    var idx = empty[Math.floor(Math.random() * empty.length)]
                    grid[idx] = Math.random() < 0.9 ? 2 : 4
                }
            }

            function slideRow(row) {
                var arr = row.filter(function(v) { return v !== 0 })
                for (var i = 0; i < arr.length - 1; i++) {
                    if (arr[i] === arr[i + 1]) {
                        arr[i] *= 2
                        score += arr[i]
                        if (score > bestScore) bestScore = score
                        arr[i + 1] = 0
                    }
                }
                arr = arr.filter(function(v) { return v !== 0 })
                while (arr.length < 4) arr.push(0)
                return arr
            }

            function moveLeft() {
                var changed = false
                var newGrid = grid.slice()
                for (var r = 0; r < 4; r++) {
                    var row = [newGrid[r*4], newGrid[r*4+1], newGrid[r*4+2], newGrid[r*4+3]]
                    var res = slideRow(row)
                    for (var c = 0; c < 4; c++) {
                        if (newGrid[r*4+c] !== res[c]) changed = true
                        newGrid[r*4+c] = res[c]
                    }
                }
                if (changed) {
                    grid = newGrid
                    spawnTile()
                    gridRepeater.model = 0
                    gridRepeater.model = 16
                }
            }

            function moveRight() {
                var changed = false
                var newGrid = grid.slice()
                for (var r = 0; r < 4; r++) {
                    var row = [newGrid[r*4+3], newGrid[r*4+2], newGrid[r*4+1], newGrid[r*4]]
                    var res = slideRow(row)
                    for (var c = 0; c < 4; c++) {
                        if (newGrid[r*4+(3-c)] !== res[c]) changed = true
                        newGrid[r*4+(3-c)] = res[c]
                    }
                }
                if (changed) {
                    grid = newGrid
                    spawnTile()
                    gridRepeater.model = 0
                    gridRepeater.model = 16
                }
            }

            function moveUp() {
                var changed = false
                var newGrid = grid.slice()
                for (var c = 0; c < 4; c++) {
                    var col = [newGrid[c], newGrid[c+4], newGrid[c+8], newGrid[c+12]]
                    var res = slideRow(col)
                    for (var r = 0; r < 4; r++) {
                        if (newGrid[c+r*4] !== res[r]) changed = true
                        newGrid[c+r*4] = res[r]
                    }
                }
                if (changed) {
                    grid = newGrid
                    spawnTile()
                    gridRepeater.model = 0
                    gridRepeater.model = 16
                }
            }

            function moveDown() {
                var changed = false
                var newGrid = grid.slice()
                for (var c = 0; c < 4; c++) {
                    var col = [newGrid[c+12], newGrid[c+8], newGrid[c+4], newGrid[c]]
                    var res = slideRow(col)
                    for (var r = 0; r < 4; r++) {
                        if (newGrid[c+(3-r)*4] !== res[r]) changed = true
                        newGrid[c+(3-r)*4] = res[r]
                    }
                }
                if (changed) {
                    grid = newGrid
                    spawnTile()
                    gridRepeater.model = 0
                    gridRepeater.model = 16
                }
            }

            Component.onCompleted: newGame()

            Row {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: 22
                spacing: 12

                Text {
                    text: "Score: " + g2048.score
                    color: "#FFFFFF"
                    font.family: Theme.fontMono
                    font.pixelSize: 11
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "Best: " + g2048.bestScore
                    color: Theme.textSecondary
                    font.family: Theme.fontMono
                    font.pixelSize: 11
                    anchors.verticalCenter: parent.verticalCenter
                }
                Rectangle {
                    width: 70
                    height: 20
                    radius: 10
                    color: Qt.rgba(1.0, 1.0, 1.0, 0.12)
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        anchors.centerIn: parent
                        text: "New Game"
                        color: "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 9
                        font.bold: true
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            g2048.newGame()
                            gameKeySensor.forceActiveFocus()
                        }
                    }
                }
            }

            // 4x4 Tiles Grid
            Grid {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: 12
                columns: 4
                rows: 4
                spacing: 4

                Repeater {
                    id: gridRepeater
                    model: 16

                    Rectangle {
                        width: 44
                        height: 32
                        radius: 8
                        property int val: g2048.grid[index] || 0

                        color: {
                            if (val === 0) return Qt.rgba(1.0, 1.0, 1.0, 0.05)
                            if (val === 2) return "#2A2A30"
                            if (val === 4) return "#3A3A42"
                            if (val === 8) return "#D97706"
                            if (val === 16) return "#EA580C"
                            if (val === 32) return "#E11D48"
                            if (val === 64) return "#7C3AED"
                            if (val === 128) return "#2563EB"
                            if (val >= 256) return "#059669"
                            return "#1E293B"
                        }
                        border.color: val > 0 ? Qt.rgba(1.0, 1.0, 1.0, 0.2) : "transparent"
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            visible: parent.val > 0
                            text: parent.val
                            color: "#FFFFFF"
                            font.family: Theme.fontMono
                            font.pixelSize: parent.val > 1000 ? 9 : (parent.val > 100 ? 11 : 13)
                            font.bold: true
                        }
                    }
                }
            }
        }

        // ═════════════════════════════════════════════════════
        // 3. FLAPPY ISLAND
        // ═════════════════════════════════════════════════════
        Item {
            id: flappyGame
            anchors.fill: parent
            visible: root.selectedGame === "flappy"

            property real birdY: height / 2
            property real birdVy: 0
            property real gravity: 0.38
            property real flapForce: -5.8

            property var pipes: []
            property int score: 0
            property int highScore: 0
            property bool running: false
            property bool gameOver: false

            function start() {
                birdY = height / 2
                birdVy = 0
                pipes = [
                    {x: width + 20, topH: 35, gap: 60},
                    {x: width + 140, topH: 50, gap: 60}
                ]
                score = 0
                gameOver = false
                running = true
                flappyTimer.restart()
                flappyCanvas.requestPaint()
            }

            function flap() {
                if (!running) {
                    start()
                    return
                }
                if (gameOver) {
                    start()
                    return
                }
                birdVy = flapForce
            }

            function step() {
                if (!running || gameOver) return

                birdVy += gravity
                birdY += birdVy

                if (birdY < 4) { birdY = 4; birdVy = 0 }
                if (birdY > height - 12) {
                    gameOver = true
                    running = false
                    if (score > highScore) highScore = score
                    flappyCanvas.requestPaint()
                    return
                }

                // Move pipes
                for (var i = 0; i < pipes.length; i++) {
                    var p = pipes[i]
                    p.x -= 2.2

                    // Check score
                    if (!p.scored && p.x < width / 3) {
                        p.scored = true
                        score++
                        if (score > highScore) highScore = score
                    }

                    // Collision check
                    var birdX = width / 3
                    if (p.x < birdX + 16 && p.x + 24 > birdX) {
                        if (birdY < p.topH || birdY + 12 > p.topH + p.gap) {
                            gameOver = true
                            running = false
                            if (score > highScore) highScore = score
                            flappyCanvas.requestPaint()
                            return
                        }
                    }

                    // Respawn pipe
                    if (p.x < -30) {
                        p.x = width + 40
                        p.topH = 20 + Math.random() * (height - 90)
                        p.scored = false
                    }
                }

                flappyCanvas.requestPaint()
            }

            Timer {
                id: flappyTimer
                interval: 20
                repeat: true
                running: flappyGame.running && !flappyGame.gameOver
                onTriggered: flappyGame.step()
            }

            Canvas {
                id: flappyCanvas
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d")
                    ctx.clearRect(0, 0, width, height)

                    // Draw pipes
                    for (var i = 0; i < flappyGame.pipes.length; i++) {
                        var p = flappyGame.pipes[i]
                        ctx.fillStyle = Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.4)
                        // Top pipe
                        ctx.fillRect(p.x, 0, 22, p.topH)
                        // Bottom pipe
                        ctx.fillRect(p.x, p.topH + p.gap, 22, height - (p.topH + p.gap))
                    }

                    // Draw Flappy Island Capsule (Bird)
                    var bx = width / 3
                    var by = flappyGame.birdY
                    ctx.fillStyle = "#FFFFFF"
                    ctx.beginPath()
                    ctx.roundRect(bx, by, 22, 11, 5)
                    ctx.fill()

                    // Glow around capsule
                    ctx.strokeStyle = Theme.accent
                    ctx.lineWidth = 1.5
                    ctx.beginPath()
                    ctx.roundRect(bx - 1, by - 1, 24, 13, 6)
                    ctx.stroke()
                }
            }

            // HUD
            Row {
                anchors.top: parent.top
                anchors.topMargin: 4
                anchors.left: parent.left
                anchors.leftMargin: 8
                spacing: 12

                Text {
                    text: "Score: " + flappyGame.score
                    color: "#FFFFFF"
                    font.family: Theme.fontMono
                    font.pixelSize: 11
                    font.bold: true
                }
                Text {
                    text: "Best: " + flappyGame.highScore
                    color: Theme.textSecondary
                    font.family: Theme.fontMono
                    font.pixelSize: 11
                }
            }

            // Tap sensor overlay
            MouseArea {
                anchors.fill: parent
                onClicked: flappyGame.flap()
            }

            // Start / Game Over Card
            Rectangle {
                anchors.fill: parent
                color: Qt.rgba(0, 0, 0, 0.72)
                visible: !flappyGame.running || flappyGame.gameOver
                radius: 12

                Column {
                    anchors.centerIn: parent
                    spacing: 6

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: flappyGame.gameOver ? "Crashed! Score: " + flappyGame.score : "Flappy Capsule Island"
                        color: flappyGame.gameOver ? Theme.danger : "#FFFFFF"
                        font.family: Theme.fontFamily
                        font.pixelSize: 13
                        font.bold: true
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "Tap Space or Click to Flap through Gates"
                        color: Theme.textSecondary
                        font.family: Theme.fontFamily
                        font.pixelSize: 10
                    }
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 100
                        height: 26
                        radius: 13
                        color: Theme.accent
                        Text {
                            anchors.centerIn: parent
                            text: flappyGame.gameOver ? "Try Again" : "Launch"
                            color: "#FFFFFF"
                            font.family: Theme.fontFamily
                            font.pixelSize: 11
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                flappyGame.start()
                                gameKeySensor.forceActiveFocus()
                            }
                        }
                    }
                }
            }
        }
    }

    // Keyboard Sensor for all games
    Item {
        id: gameKeySensor
        anchors.fill: parent
        focus: true

        Keys.onPressed: function(event) {
            // Snake controls
            if (root.selectedGame === "snake") {
                if (event.key === Qt.Key_Up || event.key === Qt.Key_W) {
                    if (snakeGame.dir !== "DOWN") snakeGame.nextDir = "UP"
                    event.accepted = true
                } else if (event.key === Qt.Key_Down || event.key === Qt.Key_S) {
                    if (snakeGame.dir !== "UP") snakeGame.nextDir = "DOWN"
                    event.accepted = true
                } else if (event.key === Qt.Key_Left || event.key === Qt.Key_A) {
                    if (snakeGame.dir !== "RIGHT") snakeGame.nextDir = "LEFT"
                    event.accepted = true
                } else if (event.key === Qt.Key_Right || event.key === Qt.Key_D) {
                    if (snakeGame.dir !== "LEFT") snakeGame.nextDir = "RIGHT"
                    event.accepted = true
                } else if (event.key === Qt.Key_Space) {
                    snakeGame.startGame()
                    event.accepted = true
                }
            }
            // 2048 controls
            else if (root.selectedGame === "2048") {
                if (event.key === Qt.Key_Left || event.key === Qt.Key_A) {
                    g2048.moveLeft()
                    event.accepted = true
                } else if (event.key === Qt.Key_Right || event.key === Qt.Key_D) {
                    g2048.moveRight()
                    event.accepted = true
                } else if (event.key === Qt.Key_Up || event.key === Qt.Key_W) {
                    g2048.moveUp()
                    event.accepted = true
                } else if (event.key === Qt.Key_Down || event.key === Qt.Key_S) {
                    g2048.moveDown()
                    event.accepted = true
                }
            }
            // Flappy controls
            else if (root.selectedGame === "flappy") {
                if (event.key === Qt.Key_Space || event.key === Qt.Key_Up) {
                    flappyGame.flap()
                    event.accepted = true
                }
            }
        }
    }
}
