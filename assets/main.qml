/*
 * Copyright (c) 2011-2014 BlackBerry Limited.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import bb.cascades 1.3

Page {
    
    Container {
        // ======== Identity ===============
        
        id: mainPage
        objectName: "mainPage"
        
        // ======== Properties =============
        
        property bool bluetoothInitialisedState: false
        
        // ======== SIGNAL()s ==============
        
        signal toggleBluetooth(bool on)
        signal sendFile()
        
        // ======== SLOT()s ================
        
        function message(text) {
            logMessage(text);
        }
        
        function onBluetoothInitialisedState(state) {
            mainPage.bluetoothInitialisedState = state;
            if (state) {
                logMessage("Bluetooth Initialise");
            } else {
                logMessage("Bluetooth Terminated");
            }
        }
        
        // ======== Local functions ========
        
        function logMessage(message) {
            log.text += (qsTr("\n") + message );
        }
        
        layout: StackLayout {
        }
        
        topPadding: 10
        leftPadding: 30
        rightPadding: 30
        
        Label {
            text: qsTr("Test OPP")
            textStyle {
                base: SystemDefaults.TextStyles.BigText
                fontWeight: FontWeight.Bold
            }
        }
        
        Container {
            layout: StackLayout {
                orientation: LayoutOrientation.LeftToRight
            }
            Button {
                id: toggleBluetoothButton
                text: !mainPage.bluetoothInitialisedState ? "Start BT" : "Stop BT";
                enabled: true
                horizontalAlignment: HorizontalAlignment.Center
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 50
                }
                onClicked: {
                    mainPage.toggleBluetooth(!mainPage.bluetoothInitialisedState);
                }
            }
            Button {
                id: sendFileButton
                text: "Send File"
                enabled: mainPage.bluetoothInitialisedState
                horizontalAlignment: HorizontalAlignment.Center
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 50
                }
                onClicked: {
                    mainPage.sendFile();
                }
            }
        }
        
        Logger {
            id: log
        }
    }
    
    onCreationCompleted: {
    }
}
