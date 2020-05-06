import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11

ApplicationWindow {
  id: window
  width: 400
  height: 500
  visible: true

  SwipeView {
    id: swipeView
    anchors.fill: parent
    currentIndex: 0

    Item {
      id: wizardStep1

      Text {
        text: "Welcome"
        font.pointSize: 36
        anchors.centerIn: parent
      }

      Button {
        text: "Generate"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        onClicked: {
          swipeView.incrementCurrentIndex();
        }
      }
    }


    Item {
      id: wizardStep2

      Text {
        text: "Generated accounts"
        font.pointSize: 36
        anchors.top: parent.top
        anchors.topMargin: 20
      }
    }
  }
}
