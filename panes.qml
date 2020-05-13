import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQuick.Dialogs 1.3
import im.status.desktop.Status 1.0

Item {
  anchors.fill: parent

  RowLayout {
    Layout.fillHeight: true
    ColumnLayout {
      //width: 100
      Layout.fillWidth: true

      Text {
        text: "Chats"
      }
      Rectangle {
        color: "#EEEEEE"
        // anchors.left: parent.left
        // anchors.right: parent.right
        // anchors.centerIn: parent
        height: 32
        width: 160
        TextInput {
          id: chatNameInput
          anchors.fill: parent
          focus: true
        }
      }
      RoundButton {
        text: "Add"
        //anchors.topMargin: 40
        //anchors.horizontalCenter: parent.horizontalCenter
        onClicked: {
          let topic = chatNameInput.text;
          let id = 1;
          let cursor = null;
          let limit = 20;
          Status.callPrivateRPC(JSON.stringify(
          {"jsonrpc": "2.0",
           "id": id,
           "method": "wakuext_chatMessages",
           "params": [topic, cursor, limit]}));
        }
      }
    }

    ColumnLayout {
      //width: 300
      Layout.fillWidth: true
      Text {
        text: "right pane"
      }
    }
  }
}
