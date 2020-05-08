import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQuick.Dialogs 1.3
import im.status.desktop.Status 1.0

ApplicationWindow {
  id: window
  width: 400
  height: 500
  visible: true

  Component.onCompleted: {
    console.log("applicationwindow loaded");
    Status.initKeystore();
    let accountsStr = Status.openAccounts();
    let accounts = JSON.parse(accountsStr);
    console.log("openAccounts response: " + accountsStr);
  }

  SwipeView {
    id: swipeView
    anchors.fill: parent
    currentIndex: 0

    property var generatedAccounts: null

    onCurrentIndexChanged: {
      console.log("onCurrentIndexChanged: " + currentIndex);
      if (currentIndex == 2) {
        passwordInput.clear();
      }
    }

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
          let generatedAccounts = Status.multiAccountGenerateAndDeriveAddresses(5, 12, "");
          console.log("js generatedAccounts: " + generatedAccounts);
          swipeView.generatedAccounts = JSON.parse(generatedAccounts);
          swipeView.incrementCurrentIndex();
        }
      }
    }

    ListModel {
      id: generatedAccountsModel
    }

    Item {
      id: wizardStep2
      property int selectedIndex: 0

      Text {
        text: "Generated accounts"
        font.pointSize: 36
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
      }

      Item {
        anchors.top: parent.top
        anchors.topMargin: 50
        //anchors.horizontalCenter: parent.horizontalCenter

        Column {
          spacing: 10
          ButtonGroup {
            id: accountGroup
          }

          Repeater {
            model: generatedAccountsModel
            Rectangle {
              height: 32
              width: 32
              border.width: 1
              anchors.leftMargin: 20
              anchors.rightMargin: 20
              Row {
                RadioButton {
                  checked: index == 0 ? true : false
                  ButtonGroup.group: accountGroup
                  onClicked: {
                    wizardStep2.selectedIndex = index;
                  }
                }
                Column {
                  Text {
                    text: alias
                  }
                  Text {
                    text: publicKey
                    width: 160
                    elide: Text.ElideMiddle
                  }

                }
              }
            }
          }
        }


      }

      Button {
        text: "Select"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        onClicked: {
          console.log("button: " + wizardStep2.selectedIndex);

          swipeView.incrementCurrentIndex();
        }
      }
    }

    Item {
      id: wizardStep3

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
        text: "Next"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        onClicked: {
          console.log("password: " + passwordInput.text);

          swipeView.incrementCurrentIndex();
        }
      }
    }

    Item {
      id: wizardStep4

      Rectangle {
        color: "#EEEEEE"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.centerIn: parent
        height: 32
        width: parent.width - 40

        TextInput {
          id: confirmPasswordInput
          anchors.fill: parent
          focus: true
          echoMode: TextInput.Password
        }
      }

      MessageDialog {
        id: passwordsDontMatchError
        title: "Error"
        text: "Passwords don't match"
        icon: StandardIcon.Warning
        standardButtons: StandardButton.Ok
        onAccepted: {
          confirmPasswordInput.clear();
        }
      }

      Button {
        text: "Finish"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 20
        onClicked: {
          console.log("confirm clicked " + confirmPasswordInput.text + " : " + passwordInput.text);

          if (confirmPasswordInput.text != passwordInput.text) {
            passwordsDontMatchError.open();
          }
          else {
            swipeView.incrementCurrentIndex();
            let selectedAccount = swipeView.generatedAccounts[wizardStep2.selectedIndex];
            console.log("selectedAccount: " + JSON.stringify(selectedAccount));
            let storeResponse = Status.multiAccountStoreDerivedAccounts(JSON.stringify(selectedAccount), passwordInput.text);
            console.log("storeResponse: " + storeResponse);
            selectedAccount.derived = JSON.parse(storeResponse);
          }
        }
      }
    }
    onGeneratedAccountsChanged: {
      if (generatedAccounts == null) {
        return;
      }
      generatedAccountsModel.clear();
      for (let i = 0; i < generatedAccounts.length; ++i) {
        let acc = generatedAccounts[i];
        console.log("acc: " + JSON.stringify(acc));
        let alias = Status.generateAlias(acc.publicKey);
        generatedAccountsModel.append({"publicKey": acc.publicKey, "alias": alias});
      }
    }
  }
}
