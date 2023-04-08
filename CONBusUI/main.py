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

        self.show()

    def update_coms(self):
        ports = serial.tools.list_ports.comports()
        print("something")
        for port, desc, hwid in sorted(ports):
            print("PORT: ", port)

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = mainWindow()
    sys.exit(app.exec_())
