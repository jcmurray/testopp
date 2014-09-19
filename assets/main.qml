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
        
        // ======== SIGNAL()s ==============
        
        signal task1Signal()
        signal task2Signal()
        
        // ======== SLOT()s ================
        
        function message(text) {
            logMessage(text);
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
                id: buttonTask1
                text: "Task 2"
                horizontalAlignment: HorizontalAlignment.Center
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 50
                }
                onClicked: {
                    mainPage.task1Signal();
                }
            }
            Button {
                id: buttonTask2
                text: "Task 2"
                horizontalAlignment: HorizontalAlignment.Center
                layoutProperties: StackLayoutProperties {
                    spaceQuota: 50
                }
                onClicked: {
                    mainPage.task2Signal();
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
