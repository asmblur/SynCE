#!/usr/bin/env python
import sys

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtGui, QtCore, uic

import libxml2
import xml2util
import characteristics
import logging
from commutil import * 

#rapi_session = RAPISession(SYNCE_LOG_LEVEL_DEFAULT)
#rapi_session = RAPISession(SYNCE_LOG_LEVEL_DEFAULT)




class MainWindowsSynceKDM(QtGui.QMainWindow):
    def updateStatusBar(self):
        if self.phoneCommunicator.phoneConnected:
            self.statusBar().showMessage("Phone connected")
        else:
            self.statusBar().showMessage("No phone connected")



    def __init__(self, *args):
        QtGui.QMainWindow.__init__(self, *args)
        uic.loadUi("synce-kdm.ui", self)
        self.phoneCommunicator = PhoneCommunicator()
       
        self.updateStatusBar() 

    def updatePowerInformation(self):
        battPercent, battFlag, aclineStat = self.phoneCommunicator.getPowerStatus()
        
        if self.phoneCommunicator.phoneConnected:
            self.batteryStatus.setValue( battPercent )
        else:
            self.batteryStatus.setValue( 0 ) 

        self.updateStatusBar() 


        
    @pyqtSignature("")
    def on_pushButton_Uninstall_clicked(self):
        #First do an update of the power information, this will make sure the phone is connected
        self.updatePowerInformation() 
       
        #in case no phone was connected, reset everything
        if self.phoneCommunicator.phoneConnected == False:
            self.updateInstalledProgramsList()
            return
        

        #Determine the program
        currentItem = self.listInstalledPrograms.currentItem()

        if currentItem == None:
            return

        programName = currentItem.text()
        
        reply = QMessageBox.question(self, "Confirm uninstall", "Are you really sure you want to uninstall \""+programName+"\"", QMessageBox.Yes, QMessageBox.No|QMessageBox.Default|QMessageBox.Escape) 

        if reply == QMessageBox.Yes:

            print "Going to uninstall:",programName
            doc = libxml2.newDoc("1.0")
            doc_node = doc.newChild(None,"wap-provisioningdoc",None)
            parent=doc_node

            node = parent.newChild(None,"characteristic",None)
            node.setProp("type", "UnInstall") ; 

            parent = node ; 
            node = parent.newChild(None,"characteristic",None)
            node.setProp("type", unicode(programName)) ; 

            parent = node ;
            node = parent.newChild(None,"parm",None)
            node.setProp("name", "uninstall") ; 
            node.setProp("value", "1") ; 

            print "_config_query: CeProcessConfig request is \n", doc_node.serialize("utf-8",1)
            reply = self.phoneCommunicator.rapi_session.process_config(doc_node.serialize("utf-8",0), 1)
            reply_doc = libxml2.parseDoc(reply)

            print "_config_query: CeProcessConfig response is \n", reply_doc.serialize("utf-8",1)

            reply_node = xml2util.GetNodeOnLevel(reply_doc, 2 )




            self.updateInstalledProgramsList()










    def updateInstalledProgramsList(self):
        self.listInstalledPrograms.clear()
        programs = self.phoneCommunicator.getListInstalledPrograms()
        for program in programs:
            self.listInstalledPrograms.addItem( program ) 
        self.updateStatusBar() 



    @pyqtSignature("")
    def on_pushButton_Refresh_clicked(self):
        self.updateInstalledProgramsList() 
        self.updatePowerInformation()
        

    @pyqtSignature("")
    def on_pushButton_Quit_clicked(self):
        QApplication.quit()


app = QtGui.QApplication(sys.argv)
mainWindowsSynceKDM = MainWindowsSynceKDM()
mainWindowsSynceKDM.show()
app.exec_()

        