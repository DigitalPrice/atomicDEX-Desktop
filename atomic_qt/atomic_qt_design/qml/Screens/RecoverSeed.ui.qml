import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import QtQuick.Controls.Material 2.12
import "../Components"
import "../Constants"

SetupPage {
    image_scale: 0.7
    image_path: General.image_path + "setup-wallet-restore-2.svg"
    title: "Recovery"
    content: ColumnLayout {
        id: rows

        width: 175

        TextFieldWithTitle {
            id: seed_input
            title: qsTr("Seed")
            field.placeholderText: qsTr("Enter the seed")
        }

        PasswordField {
            id: password_input
        }

        RowLayout {
            id: columns

            Button {
                id: back_button
                text: qsTr("Back")
            }

            Button {
                id: confirm_button
                text: qsTr("Confirm")
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
