import sys

import serial.tools.list_ports
import can
import json

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

        self.con_write("STATUS BOX", False)

        self.show()

    def toggle_connection(self):
        if self.connected:
            self.connectButton.setStyleSheet("background-color: #FFCCCB")
            self.connectButton.setText("DISCONNECTED")
            self.con_write("DISCONNECTED", False)
            self.connected = False
            self.canbus = None
        else:
            try:
                comPort = self.comComboBox.currentText()
                bitRate = self.baudrateSpinbox.value() * 1000
                self.canbus = can.interface.Bus(bustype='slcan', channel=comPort, bitrate=bitRate)

                self.connectButton.setStyleSheet("background-color: lightgreen")
                self.connectButton.setText("CONNECTED")
                self.con_write("CONNECTED", False)
                self.connected = True
            except Exception as e:
                self.con_write("CONBUS CONNECTION ERROR: " + str(e))


    def con_write(self, value = None, appendText = True):
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
