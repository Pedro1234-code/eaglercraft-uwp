// This is generally a bad practice, but we need to run scripts in the main context before the DOM loads. Because we are only matching eaglercraft.com, unsafeWindow should be safe to use.
// If someone knows a better way of doing this, please create an issue
try {
    unsafeWindow.console.warn("DANGER: This userscript is  using unsafeWindow. Unsafe websites could potentially use this to gain access to data and other content that the browser normally wouldn't allow!")
    Object.defineProperty(window, "clientWindow", {
        value: unsafeWindow
    }); // If this is a userscript, use unsafeWindow
} catch {
    Object.defineProperty(window, "clientWindow", {
        value: window
    }); // If this is plain javascript, use window
}
clientWindow.console.log("%cEagler Console v1.0.0", "color: #9b918f; font-weight: bold; background-color: #4e4a4c; padding: 0.5vw 2vw;")
// TODO: consolidate all of these into a single object?
clientWindow.keyboardFix = false; // keyboardFix ? "Standard Keyboard" : "Compatibility Mode"
clientWindow.inputFix = false; // If true, Duplicate Mode
clientWindow.blockNextInput = false; // Used for Duplicate Mode 
clientWindow.hiddenInputFocused = false; // Used for keyboard display on mobile
clientWindow.canvasTouchMode = 0; // Used for canvas touch handling
/*
  0   Idle
  1   Touch initiated
  2   Primary touch
  3   Secondary touch
  4   Scroll
  5   Finished
*/
clientWindow.canvasTouchStartX = null;
clientWindow.canvasTouchStartY = null;
clientWindow.canvasTouchPreviousX = null;
clientWindow.canvasTouchPreviousY = null;
clientWindow.canvasPrimaryID = null;
clientWindow.buttonTouchStartX = null;

// charCodeAt is designed for unicode characters, and doesn't match the behavior of the keyCodes used by KeyboardEvents, thus necessitating this function
String.prototype.toKeyCode = function () {
    const keyCodeList = { "0": 48, "1": 49, "2": 50, "3": 51, "4": 52, "5": 53, "6": 54, "7": 55, "8": 56, "9": 57, "backspace": 8, "tab": 9, "enter": 13, "shift": 16, "ctrl": 17, "alt": 18, "pause_break": 19, "caps_lock": 20, "escape": 27, " ": 32, "page_up": 33, "page_down": 34, "end": 35, "home": 36, "left_arrow": 37, "up_arrow": 38, "right_arrow": 39, "down_arrow": 40, "insert": 45, "delete": 46, "a": 65, "b": 66, "c": 67, "d": 68, "e": 69, "f": 70, "g": 71, "h": 72, "i": 73, "j": 74, "k": 75, "l": 76, "m": 77, "n": 78, "o": 79, "p": 80, "q": 81, "r": 82, "s": 83, "t": 84, "u": 85, "v": 86, "w": 87, "x": 88, "y": 89, "z": 90, "left_window_key": 91, "right_window_key": 92, "select_key": 93, "numpad_0": 96, "numpad_1": 97, "numpad_2": 98, "numpad_3": 99, "numpad_4": 100, "numpad_5": 101, "numpad_6": 102, "numpad_7": 103, "numpad_8": 104, "numpad_9": 105, "*": 106, "+": 107, "-": 109, ".": 110, "/": 111, "f1": 112, "f2": 113, "f3": 114, "f4": 115, "f5": 116, "f6": 117, "f7": 118, "f8": 119, "f9": 120, "f10": 121, "f11": 122, "f12": 123, "num_lock": 144, "scroll_lock": 145, ";": 186, "=": 187, ",": 188, "-": 189, ".": 190, "/": 191, "\u0060": 192, "[": 219, "\u005C": 220, "]": 221, "\u0022": 222 };
    return keyCodeList[this];
}
// Overrides the addEventListener behavior to all code injection on keydown event listeners. This function has thrown TypeErrors on some Android devices because fn is not recognized as a function
// This is used by Compatibility Mode to block invalid keyEvents
const _addEventListener = EventTarget.prototype.addEventListener;
Object.defineProperty(EventTarget.prototype, "addEventListener", {
    value: function (type, fn, ...rest) {
        if (type == 'keydown') { // Check if a keydown event is being added
            _addEventListener.call(this, type, function (...args) {
                if (args[0].isTrusted && clientWindow.keyboardFix) { // When we are in compatibility mode, we ignore all trusted keyboard events
                    return;
                }
                return fn.apply(this, args); // Appends the rest of the function specified by addEventListener
            }, ...rest);
        } else { // If it's not a keydown event, behave like normal (hopefully)
            _addEventListener.call(this, type, fn, ...rest);
        }
    }
});
// Key and mouse events
// Note: the client must have the key, keyCode, and which parameters defined or it will crash
// Note: for text inputs, the client only reads from the "key" paramater
//     * an exception to this appears to be the shift and backspace key
// Note: for inGame inputs, the client only reads from the "keyCode character"
function keyEvent(name, state) {
    const charCode = name.toKeyCode();
    let evt = new KeyboardEvent(state, {
        "key": name,
        "keyCode": charCode,
        "which": charCode
    });
    clientWindow.dispatchEvent(evt);
}
function mouseEvent(number, state, element, event = { "clientX": 0, "clientY": 0, "screenX": 0, "screenY": 0 }) {
    element.dispatchEvent(new PointerEvent(state, {
        "button": number,
        "buttons": number,
        "clientX": event.clientX,
        "clientY": event.clientY,
        "screenX": event.screenX,
        "screenY": event.screenY
    }));
}
function wheelEvent(element, delta) {
    element.dispatchEvent(new WheelEvent("wheel", {
        "wheelDeltaY": delta
    }));
}

// POINTERLOCK
// When requestpointerlock is called, this dispatches an event, saves the requested element to window.fakelock, and unhides the touch controls
window.fakelock = null;

Object.defineProperty(Element.prototype, "requestPointerLock", {
    value: function() {
        window.fakelock = this
        document.dispatchEvent(new Event('pointerlockchange'));
        setButtonVisibility(true);
        return true
    }
});


// Makes pointerLockElement return window.fakelock
Object.defineProperty(Document.prototype, "pointerLockElement", {
    get: function() {
        return window.fakelock;
    }
});
// When exitPointerLock is called, this dispatches an event, clears the
Object.defineProperty(Document.prototype, "exitPointerLock", {
    value: function() {
        window.fakelock = null
        document.dispatchEvent(new Event('pointerlockchange'));
        setButtonVisibility(false);
        return true
    }
});

let inGameStyle = false;
let inMenuStyle = false;
let chatlog = false;

function setButtonVisibility(pointerLocked) {
    if (pointerLocked) {
        inGameStyle = true;
        inMenuStyle = false;
        document.getElementById("eccursor").style.display = "none";
        chatlog = false;
    }
    else {
        inGameStyle = false;
        inMenuStyle = true;
        document.getElementById("eccursor").style.display = "block";
    }
};

// FULLSCREEN
clientWindow.fakefull = null;
// Stops the client from crashing when fullscreen is requested
Object.defineProperty(Element.prototype, "requestFullscreen", {
    value: function () {
        clientWindow.fakefull = this
        document.dispatchEvent(new Event('fullscreenchange'));
        return true
    }
});
Object.defineProperty(document, "fullscreenElement", {
    get: function () {
        return clientWindow.fakefull;
    }
});
Object.defineProperty(Document.prototype, "exitFullscreen", {
    value: function () {
        clientWindow.fakefull = null
        document.dispatchEvent(new Event('fullscreenchange'));
        return true
    }
});

// FILE UPLOADING
// Safari doesn't recognize the element.click() used to display the file uploader as an action performed by the user, so it ignores it.
// This hijacks the element.createElement() function to add the file upload to the DOM, so the user can manually press the button again.
const _createElement = document.createElement;
document.createElement = function (type, ignore) {
    this._createElement = _createElement;
    var element = this._createElement(type);
    if (type == "input" && !ignore) { // We set the ingore flag to true when we create the hiddenInput
        document.querySelectorAll('#fileUpload').forEach(e => e.parentNode.removeChild(e)); // Get rid of any left over fileUpload inputs
        element.id = "fileUpload";
        element.addEventListener('change', function (e) {
            element.hidden = true;
            element.style.display = "none";
        }, { passive: false, once: true });
        clientWindow.addEventListener('focus', function (e) {
            setTimeout(() => {
                element.hidden = true;
                element.style.display = "none";
            }, 300)
        }, { once: true })
        document.body.appendChild(element);
    }
    return element;
}

// The canvas is created by the client after it finishes unzipping and loading. When the canvas is created, this applies any necessary event listeners and creates buttons
function waitForElm(selector) {
    return new Promise(resolve => {
        if (document.querySelector(selector)) {
            return resolve(document.querySelector(selector));
        }
        const observer = new MutationObserver(mutations => {
            if (document.querySelector(selector)) {
                observer.disconnect();
                resolve(document.querySelector(selector));
            }
        });
        observer.observe(document.documentElement, {
            childList: true,
            subtree: true
        });
    });
}

waitForElm('canvas').then(() => { insertCanvasElements() });
function insertCanvasElements() {
    var canvas = document.querySelector('canvas');

    let controllerIndex = null;

    let ljsleftPressed = false;
    let ljsrightPressed = false;
    let ljsupPressed = false;
    let ljsdownPressed = false;
    let rjsupPressed = false;
    let rjsdownPressed = false;

    let leftPressed = false;
    let rightPressed = false;
    let upPressed = false;
    let downPressed = false;

    let bluePressed = false;
    let yellowPressed = false;
    let redPressed = false;
    let greenPressed = false;

    let pausePressed = false;
    let selectPressed = false;
    let lbPressed = false;
    let rbPressed = false;
    let ltPressed = false;
    let rtPressed = false;
    let ljsPressed = false;
    let rjsPressed = false;

    let ltpress = true;
    let rtpress = true;

    let perspective = false;
    let debug = false;
    let drop = false;
    let chat = false;
    let jump = false;
    let zoom = false;
    let inventory1 = false;
    let lefthotbar = false;
    let righthotbar = false;
    let pause = false;
    let players = false;
    let exit = false;
    let scrollup = false;
    let scrolldown = false;

    let rHorizontalValue = 0;
    let rVerticalValue = 0;
    let leftRightValue = 0;
    let upDownValue = 0;
    let sizex = window.innerWidth / 2;
    let sizey = window.innerHeight / 2;

    window.addEventListener("gamepadconnected", (event) => {
        controllerIndex = event.gamepad.index;
        console.log("%cController Connected!", "color: #9b918f; font-weight: bold; background-color: #4e4a4c; padding: 0.5vw 2vw;");
        const eccursor = document.createElement("div");
        eccursor.id = "eccursor";
        document.body.appendChild(eccursor);
        canvas.style.cursor = "none";
        setButtonVisibility(false);
    });

    window.addEventListener("gamepaddisconnected", () => {
        console.log("%cController Disconnected!", "color: #9b918f; font-weight: bold; background-color: #4e4a4c; padding: 0.5vw 2vw;");
        controllerIndex = null;
        canvas.style.cursor = "default";
        document.getElementById("eccursor").remove();
    });

    function controllerInput() {
        if (controllerIndex !== null) {            
            const gamepad = navigator.getGamepads()[controllerIndex];
            const buttons = gamepad.buttons;
            const stickDeadZone = 0.4;

            // Joystick Axes
            leftRightValue = gamepad.axes[0];
            upDownValue = gamepad.axes[1];
            rHorizontalValue = gamepad.axes[2];
            rVerticalValue = gamepad.axes[3];

            // Handles Player Movement
            if (leftRightValue >= stickDeadZone) {ljsrightPressed = true;}
            else if (leftRightValue <= stickDeadZone) {ljsrightPressed = false;}
            if (leftRightValue <= -stickDeadZone) {ljsleftPressed = true;}
            else if (leftRightValue >= -stickDeadZone) {ljsleftPressed = false;}

            if (upDownValue >= stickDeadZone) {ljsdownPressed = true;}
            else if (upDownValue <= stickDeadZone) {ljsdownPressed = false;}
            if (upDownValue <= -stickDeadZone) {ljsupPressed = true;}
            else if (upDownValue >= -stickDeadZone) {ljsupPressed = false;}

            // Handles UI Scrolling
            if (inMenuStyle === true) {
                if (rVerticalValue >= stickDeadZone) {rjsdownPressed = true;}
                else if (rVerticalValue <= stickDeadZone) {rjsdownPressed = false;}
                if (rVerticalValue <= -stickDeadZone) {rjsupPressed = true;}
                else if (rVerticalValue >= -stickDeadZone) {rjsupPressed = false;}
            }

            // Handle Button Presses
            greenPressed = buttons[0].pressed;
            redPressed = buttons[1].pressed;
            bluePressed = buttons[2].pressed;
            yellowPressed = buttons[3].pressed;

            lbPressed = buttons[4].pressed;
            rbPressed = buttons[5].pressed;
            ltPressed = buttons[6].pressed;
            rtPressed = buttons[7].pressed;

            selectPressed = buttons[8].pressed;
            pausePressed = buttons[9].pressed;

            ljsPressed = buttons[10].pressed;
            rjsPressed = buttons[11].pressed;

            upPressed = buttons[12].pressed;
            downPressed = buttons[13].pressed;
            leftPressed = buttons[14].pressed;
            rightPressed = buttons[15].pressed;
        }
    }
    function rotateCamera() {
        const sensitivity = 14;
        if (rHorizontalValue >= 0.12 || rVerticalValue >= 0.12) {
            let movementX = rHorizontalValue * sensitivity;
            let movementY = rVerticalValue * sensitivity;
            canvas.dispatchEvent(new MouseEvent("mousemove", {
                "movementX": movementX,
                "movementY": movementY
            }));
        }
        else if (rHorizontalValue <= -0.12 || rVerticalValue <= -0.12) {
            let movementX = rHorizontalValue * sensitivity;
            let movementY = rVerticalValue * sensitivity;
            canvas.dispatchEvent(new MouseEvent("mousemove", {
                "movementX": movementX,
                "movementY": movementY
            }));
        }
    };

    function moveCursor() {
        const gamepad = navigator.getGamepads()[controllerIndex];
        const lx = gamepad.axes[0] || 0, ly = gamepad.axes[1] || 0;
        const dz = 0.18;
        const dx = Math.abs(lx) > dz ? lx : 0;
        const dy = Math.abs(ly) > dz ? ly : 0;
        window.addEventListener('resize', () => {
            sizex = Math.max(0, Math.min(window.innerWidth, sizex));
            sizey = Math.max(0, Math.min(window.innerHeight, sizey));
        });
        sizex += dx * 8;
        sizey += dy * 8;
        sizex = Math.max(0, Math.min(window.innerWidth, sizex));
        sizey = Math.max(0, Math.min(window.innerHeight, sizey));
        document.getElementById("eccursor").style.left = sizex + 'px';
        document.getElementById("eccursor").style.top = sizey + 'px';
        canvas.dispatchEvent(new MouseEvent("mousemove", {
            "clientX": sizex,
            "clientY": sizey
        }));
    };

    function simulateClick(scclientX, scclientY, scwhich = 'left', scshift = false) {
        const button = scwhich === 'left' ? 0 : scwhich === 'right' ? 2 : 1;
        const mouseOpts = {
            clientX: scclientX, clientY: scclientY, bubbles: true, button, shiftKey: scshift
        };
        canvas.dispatchEvent(new MouseEvent('mousedown', mouseOpts));
        setTimeout(() => {
            if (scwhich === 'left') {
                canvas.dispatchEvent(new MouseEvent('click', mouseOpts));
                canvas.dispatchEvent(new MouseEvent('mouseup', mouseOpts));
            }
            if (scwhich === 'right') {
                canvas.dispatchEvent(new MouseEvent('contextmenu', mouseOpts));
            }
        }, 10);
    };

    function movePlayer() {
        if (inGameStyle === true) {
            if (ljsupPressed) { keyEvent("w", "keydown"); }
            else if (!ljsupPressed) { keyEvent("w", "keyup"); }
            if (ljsleftPressed) { keyEvent("a", "keydown"); }
            else if (!ljsleftPressed) { keyEvent("a", "keyup"); }
            if (ljsdownPressed) { keyEvent("s", "keydown"); }
            else if (!ljsdownPressed) { keyEvent("s", "keyup"); }
            if (ljsrightPressed) { keyEvent("d", "keydown"); }
            else if (!ljsrightPressed) { keyEvent("d", "keyup"); }

            if (greenPressed) { keyEvent(" ", "keydown"); jump = false; }
            else if (!greenPressed) { keyEvent(" ", "keyup"); jump = true; }
            if (bluePressed && zoom) { keyEvent("c", "keydown"); zoom = false; }
            else if (!bluePressed) { keyEvent("c", "keyup"); zoom = true; }
            if (yellowPressed && inventory1) { keyEvent("e", "keydown"); inventory1 = false; }
            else if (!yellowPressed) { keyEvent("e", "keyup"); inventory1 = true; }

            if (lbPressed && lefthotbar) { wheelEvent(canvas, 10); lefthotbar = false; }
            else if (!lbPressed) { lefthotbar = true; }
            if (rbPressed && righthotbar) { wheelEvent(canvas, -10); righthotbar = false; }
            else if (!rbPressed) { righthotbar = true; }
            if (ltPressed && ltpress) { mouseEvent(2, "mousedown", canvas); ltpress = false; }
            else if (!ltPressed) { mouseEvent(2, "mouseup", canvas); ltpress = true; }
            if (rtPressed && rtpress) { mouseEvent(0, "mousedown", canvas); rtpress = false; }
            else if (!rtPressed) { mouseEvent(0, "mouseup", canvas); rtpress = true; }

            if (pausePressed && pause) { keyEvent("`", "keydown"); pause = false; }
            else if (!pausePressed) { keyEvent("`", "keyup"); pause = true; }
            if (selectPressed && players) { keyEvent("tab", "keydown"); players = false; }
            else if (!selectPressed) { keyEvent("Tab", "keyup"); players = true; }
            if (ljsPressed) { keyEvent("r", "keydown"); }
            else if (!ljsPressed) { keyEvent("r", "keyup"); }
            if (rjsPressed || redPressed) { keyEvent("shift", "keydown"); }
            else if (!rjsPressed || !redPressed) { keyEvent("shift", "keyup"); }

            if (upPressed && perspective) { keyEvent("f5", "keydown"); perspective = false; }
            else if (!upPressed) { keyEvent("f5", "keyup"); perspective = true; }
            if (leftPressed && debug) { keyEvent("f3", "keydown"); debug = false; }
            else if (!leftPressed) { keyEvent("f3", "keyup"); debug = true; }
            if (downPressed && drop) { keyEvent("q", "keydown"); drop = false; }
            else if (!downPressed) { keyEvent("q", "keyup"); drop = true; }
            if (rightPressed && chat) { keyEvent("t", "keydown"); chat = false; chatlog = true; }
            else if (!rightPressed) { keyEvent("t", "keyup"); chat = true; }
        }

        if (inMenuStyle === true) {
            if (greenPressed && jump) { simulateClick(sizex, sizey, 'left', yellowPressed); jump = false; }
            else if (!greenPressed) { jump = true; }
            if (!chatlog) {
                if (redPressed && exit) { keyEvent("`", "keydown"); exit = false}
                else if (!redPressed) { keyEvent("`", "keyup"); exit = true;}
            }
            if (bluePressed && zoom) { simulateClick(sizex, sizey, 'right', yellowPressed); zoom = false; }
            else if (!bluePressed) { zoom = true; }

            if (rjsupPressed && scrollup) { wheelEvent(canvas, 10); scrollup = false; }
            else if (!rjsupPressed) { scrollup = true; }
            if (rjsdownPressed && scrolldown) { wheelEvent(canvas, -10); scrolldown = false; }
            else if (!rjsdownPressed) { scrolldown = true; }
        }
    }

    function gameLoop() {
        if (inGameStyle === true) {rotateCamera();}
        if (inMenuStyle === true) {moveCursor();}
        controllerInput();
        movePlayer();
        requestAnimationFrame(gameLoop);
    }

    gameLoop();
}
// CSS for touch screen buttons, along with fixing iOS's issues with 100vh ignoring the naviagtion bar, and actually disabling zoom because safari ignores user-scalable=no :(
let customStyle = document.createElement("style");
customStyle.textContent = `
    html, body, canvas {
        height: 100svh !important;
        height: -webkit-fill-available !important;
        touch-action: pan-x pan-y;
        -webkit-touch-callout: none;
        -webkit-user-select: none;
        -khtml-user-select: none;
        -moz-user-select: none;
        -ms-user-select: none;
        user-select: none;
        outline: none;
        -webkit-tap-highlight-color: rgba(255, 255, 255, 0);
    }
    #fileUpload {
        position: absolute;
        left: 0;
        right: 100vw;
        top: 0;
        bottom: 100vh;
        width: 100vw;
        height: 100vh;
        background-color:rgba(255,255,255,0.5);
    }
    #eccursor {
        display: none;
        position: absolute;
        width: 2.5vw;
        height: 2.5vw;
        z-index: 2;
        top: 50vh;
        left: 50vw;
        transform: translate(-50%, -50%);
        background: url(data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAABEUlEQVR4AeyUWQ7DIAxEce9/ZzqPCIlQHLaPKhKIYbHNeGpSPmGvRR0Hmtb6joCoFoBSL4vYEaC8+/0IOBU4FXhtBVoPT8vWfShWKqDHL+YXMCWQIe+nRcwKUC4/h5wI8gPwVpgRIP6L28yC2S/gVlCaGEaQBcDcQ81nMpTQ9tZ7fPgDAiT6ulMt0l16843+YeOdr+2iiAjQ/L+OAFNr3mltH5VZn/P24jMEaA7lXXpr4kpwhyVKH2uPp7Snb4DgEeiHcDa43wkkCkoTwwhyBUZiiRH/JYJNDTkx+QF4K8wK4LjyWNDAOoE10MaEqb4igAStRC0bsY9YFfBIOuM8Ak4FTgVeXQE9fpZfRPP++z37FwAA//+PVAMWAAAABklEQVQDACXOkD99mCgPAAAAAElFTkSuQmCC) center no-repeat;
        background-size: cover;
        image-rendering: pixilated;
    }
`;
document.documentElement.appendChild(customStyle);