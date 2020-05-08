import QtQuick 2.14
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQuick.Dialogs 1.3
import im.status.desktop.Status 1.0

SwipeView {
  id: swipeView
  property var accounts: {}
  property int selectedAccount: 0

  anchors.fill: parent
  currentIndex: 0

  signal login(string accountId)

  // Option 1 (property change signal)
  // onAccountsChanged: {

  //   if (!accounts) {
  //     return;
  //   }

  //   console.log("onAccountsChanged: " + accounts.length);
  //   for (let i = 0; i < accountsList.accounts.length; ++i) {
  //     let acc = accountsList.accounts[i];
  //     accountsListModel.append({"name": acc.name, "keyUid": acc["key-uid"]});
  //   }
  // }

  // Option 2 (invoke fn directly)
  function loadAccounts(accounts) {
    for (let i = 0; i < accounts.length; ++i) {
      let acc = accounts[i];
      accountsListModel.append({"name": acc.name, "keyUid": acc["key-uid"]});
    }
  }




  onCurrentIndexChanged: {
    console.log("onCurrentIndexChanged: " + currentIndex);
    if (currentIndex == 1) {
      passwordInput.clear();
    }
  }

  Item {
    id: accountsList
    Column {
      spacing: 10

      ListModel {
        id: accountsListModel
      }
      Repeater {
        model: accountsListModel
        Column {
          TapHandler {
            onTapped: {
              console.log("onTapped");
              swipeView.selectedAccount = index;
              swipeView.incrementCurrentIndex();
            }
          }
          Text {
            text: "id: " + keyUid
          }
          Text {
            text: "desc: " + name
          }
        }
      }

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
    id: loginScreen
    Rectangle {
      color: "#EEEEEE"
      anchors.left: parent.left
      anchors.right: parent.right
      anchors.centerIn: parent
      height: 32
      width: parent.width - 40
      TextInput {
        id: passwordInput
        anchors.fill: parent
        focus: true
        echoMode: TextInput.Password
      }
    }

    Button {
      text: "Login"
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.bottom: parent.bottom
      anchors.bottomMargin: 20
      onClicked: {
        console.log("password: " + passwordInput.text);
        swipeView.login(accountsListModel.get(swipeView.selectedAccount)["keyUid"]);
      }
    }
  }
}

