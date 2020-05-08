import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4
import QtQuick.Layouts 1.11
import QtQuick.Window 2.11
import QtQuick.Dialogs 1.3
import im.status.desktop.Status 1.0

Item {
  anchors.fill: parent

  Row {
    Text {
      text: "left pane"
    }

    Text {
      text: "right pane"
    }
  }
}
