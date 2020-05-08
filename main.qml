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
    //console.log("openAccounts response: " + accountsStr);
    if (accounts.length > 0) {
      mainPane.source = "login.qml";
      //mainPane.item.accounts = accounts;
      mainPane.item.loadAccounts(accounts);
      mainPane.item.login.connect(handleLogin);
    }
    else {
      mainPane.source = "introWizard.qml";
    }
  }

  function handleLogin(accountId) {
    console.log("handleLogin: " + accountId);
  }

   Loader {
     id: mainPane
     focus: true
     anchors.fill: parent
   }
  

}
