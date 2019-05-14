import QtQuick 2.9
//import QtQuick.Controls 1.4
import Sailfish.Silica 1.0
import QtQml.Models 2.1
import QtQuick.Window 2.2
import ru.sashikknox 1.0
 
Window {
//IrrQuickWindow {
    id: appWindow

    visible: true
    width: 400
    height: 300
    title: qsTr("Hello World")

    IrrQuickItem {
        id: irrItem
        anchors.fill: parent
        focus: true
    }
     

    Rectangle {
        id: joystick
        width: 150
        height: width
        color:  Qt.rgba(1,1,1,0.7)
        border.color: "white"
        border.width: 2
        radius: width * 0.5
        
        property real joy_x: 0
        property real joy_y: 0
        
        property real m_x: 0
        property real m_y: 0
        
        onM_xChanged: {
            var m_vec = Qt.vector2d(m_x,m_y);
            var l = Math.max(m_vec.length(),joystick.radius);
            joy_x = m_vec.x/l;
            joy_y = m_vec.y/l;
            
            irrItem.joystick = Qt.point(joy_x,joy_y);
        }
        
        onM_yChanged: {
            var m_vec = Qt.vector2d(m_x,m_y);
            var l = Math.max(m_vec.length(),joystick.radius);
            joy_x = m_vec.x/l;
            joy_y = m_vec.y/l;
            
            irrItem.joystick = Qt.point(joy_x,joy_y);
        }
        
        anchors {
            margins: 15
            left: parent.left
            bottom: parent.bottom
        }
        
        MouseArea {
            anchors.fill: parent;
            onMouseXChanged: joystick.m_x =   mouseX  - joystick.radius;
            onMouseYChanged: joystick.m_y = - mouseY + joystick.radius;
            onReleased: {
                joystick.m_x = 0;
                joystick.m_y = 0;
            }
        }
        
        Rectangle {
            id: stick
            width: joystick.radius * 0.8;
            height: width
            color:  Qt.rgba(1,1,1,0.7)
            border.color: "white"
            border.width: 2
            radius: width * 0.5
            
            x: joystick.radius - stick.radius + joystick.joy_x*(joystick.radius - stick.radius)
            y: joystick.radius - stick.radius - joystick.joy_y*(joystick.radius - stick.radius)
        }
    }
    
    Rectangle {
        anchors {
            bottom: parent.bottom
            left: joystick.right
            right: parent.right
            margins: 5
        }
        color:  Qt.rgba(1,1,1,0.7)
        border.color: "black"
        border.width: 2
        radius: 10
        Text {
            id: textm
            text: qsTr("Привет мир, это Irrlicht Engine + Qt QML, и все здорово работает и рендериться!\n")
            anchors.fill: parent
            anchors.margins: 5
            font.pixelSize: 12
            wrapMode: Text.Wrap
        }
        width: 200
        height: Math.max(parent.height * 0.15, textm.contentHeight + anchors.margins * 2 )
    }
    
    Slider {
        to: 5
        from: 40
        orientation: Qt.Horizontal
        value: 20
        anchors {
            top: parent.top
            right: parent.right
            margins: 15
        }
        width: Math.min(parent.width,400)
        
        onValueChanged: {
            irrItem.cameraSpeed = value
        }
    }
}




















