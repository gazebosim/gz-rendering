import QtQuick 2.0
import IgnitionRendering 1.0

Item {
    width: 800
    height: 600

    Renderer {
        id: renderer
        anchors.fill: parent
        anchors.margins: 10
        opacity: 0
        Component.onCompleted: renderer.opacity = 1;
    }

    Rectangle {
        id: labelFrame
        anchors.margins: -10
        radius: 5
        color: "white"
        border.color: "black"
        opacity: 0.8
        anchors.fill: label
    }

    Text {
        id: label
        anchors.bottom: renderer.bottom
        anchors.left: renderer.left
        anchors.right: renderer.right
        anchors.margins: 20
        wrapMode: Text.WordWrap
        text: "QML09 Ignition : `simple_demo` example using ignition-rendering and QML."
    }
}
