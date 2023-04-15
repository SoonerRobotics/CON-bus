import sys

import serial.tools.list_ports
import can
import json
import struct

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5 import uic


class mainWindow(QMainWindow):
    def __init__(self):
        super(mainWindow, self).__init__()
        uic.loadUi("./CONBusUI/CONbus.ui", self)  # Load the .ui

        self.update_coms()
        
        self.updateComButton.released.connect(self.update_coms)

        self.connected = False
        self.canbus = None
        self.connectButton.released.connect(self.toggle_connection)
        self.connectButton.setStyleSheet("background-color: #FFCCCB")

        self.parameters = {}
        self.openRegisterValueButton.released.connect(self.open_json)

        self.loadSelectedButton.released.connect(self.send_selected)

        self.con_print("STATUS BOX", False)

        self.currentName = ""
        self.currentDeviceID = -1

        self.show()

    def send_selected(self):
        try:
            row = self.changeTable.currentRow()
            name = self.changeTable.item(row, 0).text()
            value = self.changeTable.item(row, 1).text()
            varType = self.changeTable.item(row, 2).text()

            if varType == "float":
                dataLen = 4
                value = bytearray(struct.pack("f", float(value)))

            elif varType == "int":
                dataLen = 4
                value = int(value).to_bytes(dataLen, 'little')

            else:
                raise Exception("invalid variable type")
            

            id = 0x1100 + self.currentDeviceID
            data = bytearray()

            parameter = row.to_bytes(1, 'little')
            length = dataLen.to_bytes(1, 'little')
            reserve = b'\x00'
            
            data = parameter + length + reserve + value
            print(data)
            msg = can.message(arbitration_id = id, data = data, is_extended_id = False)
            self.canbus.send(msg)
        except Exception as e:
            self.con_print("ERROR Send Selected: " + str(e))


    def toggle_connection(self):
        if self.connected:
            self.connectButton.setStyleSheet("background-color: #FFCCCB")
            self.connectButton.setText("DISCONNECTED")
            self.con_print("DISCONNECTED", False)
            self.connected = False
            self.canbus = None
        else:
            try:
                comPort = self.comComboBox.currentText()
                bitRate = self.baudrateSpinbox.value() * 1000
                self.canbus = can.interface.Bus(bustype='slcan', channel=comPort, bitrate=bitRate)

                self.connectButton.setStyleSheet("background-color: lightgreen")
                self.connectButton.setText("CONNECTED")
                self.con_print("CONNECTED", False)
                self.connected = True
            except Exception as e:
                self.con_print("CONBUS CONNECTION ERROR: " + str(e))

    def open_json(self):
        try:
            bin = QFileDialog.getOpenFileName(
                self, "Open device json file", ".", "*.json" 
            )
            if bin[0] != "":
                file = open(bin[0])
                self.parameters = json.load(file)
                self.changeTable.clear()
                self.changeTable.resizeColumnsToContents()
                self.changeTable.resizeRowsToContents()
                registers = self.parameters["registers"]
                size = len(registers.keys())
                self.changeTable.setColumnCount(3)
                self.changeTable.setRowCount(size)
                self.changeTable.setHorizontalHeaderLabels(["Name", "Value", "Type"])
                
                self.currentDeviceID = self.parameters["deviceID"]
                self.currentName = self.parameters["name"]


                for key in registers.keys():
                    
                    row = int(key)
                    self.changeTable.setVerticalHeaderItem(row, QTableWidgetItem("ID " + key))
                    name = QTableWidgetItem(registers[key]["registerName"])
                    varType = QTableWidgetItem(registers[key]["type"])
                    value = QTableWidgetItem(str(registers[key]["defaultValue"]))
                    self.changeTable.setItem(row, 0, name)
                    self.changeTable.setItem(row, 2, varType)
                    self.changeTable.setItem(row, 1, value)
                
                self.changeTable.resizeColumnsToContents()
                self.changeTable.resizeRowsToContents()
                self.con_print("JSON OPEN SUCCESSFULLY with device ID: " + str(self.currentDeviceID) + " and Name: " + self.currentName)
        except Exception as e:
            self.con_print("JSON file open FAIL: " + str(e))

    def con_print(self, value = None, appendText = True):
        if value is not None and appendText is True:
            self.statusBox.append(value)
        elif value is not None and appendText is False:
            self.statusBox.clear()
            self.statusBox.append(value)
        else:
            self.statusBox.clear()

    def update_coms(self):
        ports = serial.tools.list_ports.comports()
        self.comComboBox.clear()
        self.comComboBox.addItem("SELECT CAN to USB DEVICE")
        self.statusBox.append("COM LIST UPDATED")
        for port, desc, hwid in sorted(ports):
            self.comComboBox.addItem(port)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mainWindow()
    sys.exit(app.exec_())
