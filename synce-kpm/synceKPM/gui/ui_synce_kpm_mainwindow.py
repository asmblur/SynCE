#!/usr/bin/env python
# coding=UTF-8
#
# Generated by pykdeuic4 from ../data/ui/synce-kpm-mainwindow.ui on Mon Apr  5 00:17:56 2010
#
# WARNING! All changes to this file will be lost.
from PyKDE4 import kdecore
from PyKDE4 import kdeui
from PyQt4 import QtCore, QtGui

class Ui_synce_kpm_mainwindow(object):
    def setupUi(self, synce_kpm_mainwindow):
        synce_kpm_mainwindow.setObjectName("synce_kpm_mainwindow")
        synce_kpm_mainwindow.resize(743, 640)
        self.groupBox = QtGui.QGroupBox(synce_kpm_mainwindow)
        self.groupBox.setGeometry(QtCore.QRect(210, 10, 521, 51))
        self.groupBox.setObjectName("groupBox")
        self.label = QtGui.QLabel(self.groupBox)
        self.label.setGeometry(QtCore.QRect(10, 20, 71, 16))
        self.label.setObjectName("label")
        self.label_devicename = QtGui.QLabel(self.groupBox)
        self.label_devicename.setGeometry(QtCore.QRect(100, 20, 44, 14))
        self.label_devicename.setObjectName("label_devicename")
        self.labelDeviceName = QtGui.QLabel(self.groupBox)
        self.labelDeviceName.setGeometry(QtCore.QRect(100, 20, 161, 18))
        self.labelDeviceName.setObjectName("labelDeviceName")
        self.toolButtonDeviceIsLocked = QtGui.QToolButton(self.groupBox)
        self.toolButtonDeviceIsLocked.setEnabled(True)
        self.toolButtonDeviceIsLocked.setGeometry(QtCore.QRect(270, 20, 25, 23))
        self.toolButtonDeviceIsLocked.setObjectName("toolButtonDeviceIsLocked")
        self.labelDeviceIsLocked = QtGui.QLabel(self.groupBox)
        self.labelDeviceIsLocked.setGeometry(QtCore.QRect(300, 20, 221, 18))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.labelDeviceIsLocked.setFont(font)
        self.labelDeviceIsLocked.setScaledContents(True)
        self.labelDeviceIsLocked.setObjectName("labelDeviceIsLocked")
        self.pushButton_Quit = QtGui.QPushButton(synce_kpm_mainwindow)
        self.pushButton_Quit.setGeometry(QtCore.QRect(660, 610, 75, 24))
        self.pushButton_Quit.setObjectName("pushButton_Quit")
        self.tabWidget = QtGui.QTabWidget(synce_kpm_mainwindow)
        self.tabWidget.setEnabled(False)
        self.tabWidget.setGeometry(QtCore.QRect(10, 210, 721, 391))
        self.tabWidget.setTabPosition(QtGui.QTabWidget.North)
        self.tabWidget.setObjectName("tabWidget")
        self.tab = QtGui.QWidget()
        self.tab.setObjectName("tab")
        self.label_3 = QtGui.QLabel(self.tab)
        self.label_3.setGeometry(QtCore.QRect(10, 20, 131, 18))
        self.label_3.setObjectName("label_3")
        self.labelDeviceOwner = QtGui.QLabel(self.tab)
        self.labelDeviceOwner.setGeometry(QtCore.QRect(150, 20, 180, 16))
        self.labelDeviceOwner.setObjectName("labelDeviceOwner")
        self.label_5 = QtGui.QLabel(self.tab)
        self.label_5.setGeometry(QtCore.QRect(10, 50, 131, 18))
        self.label_5.setObjectName("label_5")
        self.labelModelName = QtGui.QLabel(self.tab)
        self.labelModelName.setGeometry(QtCore.QRect(150, 50, 180, 16))
        self.labelModelName.setObjectName("labelModelName")
        self.label_7 = QtGui.QLabel(self.tab)
        self.label_7.setGeometry(QtCore.QRect(310, 110, 91, 16))
        self.label_7.setObjectName("label_7")
        self.labelStorageTotal = QtGui.QLabel(self.tab)
        self.labelStorageTotal.setGeometry(QtCore.QRect(370, 110, 91, 16))
        self.labelStorageTotal.setLayoutDirection(QtCore.Qt.RightToLeft)
        self.labelStorageTotal.setObjectName("labelStorageTotal")
        self.storageSelector = QtGui.QComboBox(self.tab)
        self.storageSelector.setGeometry(QtCore.QRect(150, 110, 141, 20))
        self.storageSelector.setObjectName("storageSelector")
        self.label_9 = QtGui.QLabel(self.tab)
        self.label_9.setGeometry(QtCore.QRect(10, 110, 131, 18))
        self.label_9.setObjectName("label_9")
        self.label_11 = QtGui.QLabel(self.tab)
        self.label_11.setGeometry(QtCore.QRect(310, 140, 91, 16))
        self.label_11.setObjectName("label_11")
        self.labelStorageUsed = QtGui.QLabel(self.tab)
        self.labelStorageUsed.setGeometry(QtCore.QRect(370, 140, 91, 16))
        self.labelStorageUsed.setLayoutDirection(QtCore.Qt.RightToLeft)
        self.labelStorageUsed.setObjectName("labelStorageUsed")
        self.label_13 = QtGui.QLabel(self.tab)
        self.label_13.setGeometry(QtCore.QRect(310, 170, 91, 16))
        self.label_13.setObjectName("label_13")
        self.labelStorageFree = QtGui.QLabel(self.tab)
        self.labelStorageFree.setGeometry(QtCore.QRect(370, 170, 91, 16))
        self.labelStorageFree.setLayoutDirection(QtCore.Qt.RightToLeft)
        self.labelStorageFree.setObjectName("labelStorageFree")
        self.batteryStatus = QtGui.QProgressBar(self.tab)
        self.batteryStatus.setGeometry(QtCore.QRect(150, 80, 141, 20))
        self.batteryStatus.setProperty("value", 0)
        self.batteryStatus.setInvertedAppearance(False)
        self.batteryStatus.setObjectName("batteryStatus")
        self.label_2 = QtGui.QLabel(self.tab)
        self.label_2.setGeometry(QtCore.QRect(10, 80, 131, 18))
        self.label_2.setObjectName("label_2")
        self.tabWidget.addTab(self.tab, "")
        self.tab_2 = QtGui.QWidget()
        self.tab_2.setObjectName("tab_2")
        self.listInstalledPrograms = QtGui.QListWidget(self.tab_2)
        self.listInstalledPrograms.setGeometry(QtCore.QRect(10, 10, 611, 311))
        self.listInstalledPrograms.setObjectName("listInstalledPrograms")
        self.pushButton_Refresh = QtGui.QPushButton(self.tab_2)
        self.pushButton_Refresh.setGeometry(QtCore.QRect(630, 10, 75, 24))
        self.pushButton_Refresh.setObjectName("pushButton_Refresh")
        self.pushButton_Uninstall = QtGui.QPushButton(self.tab_2)
        self.pushButton_Uninstall.setGeometry(QtCore.QRect(630, 40, 75, 24))
        self.pushButton_Uninstall.setObjectName("pushButton_Uninstall")
        self.pushButton_InstallCAB = QtGui.QPushButton(self.tab_2)
        self.pushButton_InstallCAB.setGeometry(QtCore.QRect(10, 330, 75, 24))
        self.pushButton_InstallCAB.setObjectName("pushButton_InstallCAB")
        self.tabWidget.addTab(self.tab_2, "")
        self.tab_3 = QtGui.QWidget()
        self.tab_3.setObjectName("tab_3")
        self.viewPartnerships = QtGui.QTreeView(self.tab_3)
        self.viewPartnerships.setGeometry(QtCore.QRect(10, 10, 611, 311))
        self.viewPartnerships.setItemsExpandable(False)
        self.viewPartnerships.setObjectName("viewPartnerships")
        self.labelSyncEngineNotRunning = QtGui.QLabel(self.tab_3)
        self.labelSyncEngineNotRunning.setGeometry(QtCore.QRect(90, 110, 281, 18))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.labelSyncEngineNotRunning.setFont(font)
        self.labelSyncEngineNotRunning.setScaledContents(True)
        self.labelSyncEngineNotRunning.setObjectName("labelSyncEngineNotRunning")
        self.button_add_pship = QtGui.QPushButton(self.tab_3)
        self.button_add_pship.setGeometry(QtCore.QRect(630, 10, 75, 24))
        self.button_add_pship.setObjectName("button_add_pship")
        self.button_delete_pship = QtGui.QPushButton(self.tab_3)
        self.button_delete_pship.setGeometry(QtCore.QRect(630, 40, 75, 24))
        self.button_delete_pship.setObjectName("button_delete_pship")
        self.tabWidget.addTab(self.tab_3, "")
        self.tab_4 = QtGui.QWidget()
        self.tab_4.setObjectName("tab_4")
        self.registryKeyView = QtGui.QTreeView(self.tab_4)
        self.registryKeyView.setGeometry(QtCore.QRect(0, 10, 251, 311))
        self.registryKeyView.setProperty("cursor", QtCore.Qt.WaitCursor)
        self.registryKeyView.setHorizontalScrollBarPolicy(QtCore.Qt.ScrollBarAsNeeded)
        self.registryKeyView.setHeaderHidden(True)
        self.registryKeyView.setObjectName("registryKeyView")
        self.registryKeyView.header().setVisible(False)
        self.registryValueView = QtGui.QTableView(self.tab_4)
        self.registryValueView.setGeometry(QtCore.QRect(270, 10, 431, 311))
        self.registryValueView.setHorizontalScrollMode(QtGui.QAbstractItemView.ScrollPerPixel)
        self.registryValueView.setShowGrid(False)
        self.registryValueView.setGridStyle(QtCore.Qt.NoPen)
        self.registryValueView.setWordWrap(False)
        self.registryValueView.setObjectName("registryValueView")
        self.tabWidget.addTab(self.tab_4, "")
        self.groupBox_2 = QtGui.QGroupBox(synce_kpm_mainwindow)
        self.groupBox_2.setGeometry(QtCore.QRect(210, 70, 521, 121))
        self.groupBox_2.setObjectName("groupBox_2")
        self.label_partnership_2 = QtGui.QLabel(self.groupBox_2)
        self.label_partnership_2.setGeometry(QtCore.QRect(10, 40, 141, 18))
        self.label_partnership_2.setObjectName("label_partnership_2")
        self.label_partnership_3 = QtGui.QLabel(self.groupBox_2)
        self.label_partnership_3.setGeometry(QtCore.QRect(10, 60, 141, 18))
        self.label_partnership_3.setObjectName("label_partnership_3")
        self.label_partnership_1 = QtGui.QLabel(self.groupBox_2)
        self.label_partnership_1.setGeometry(QtCore.QRect(10, 20, 141, 18))
        self.label_partnership_1.setObjectName("label_partnership_1")
        self.sync_progressbar = QtGui.QProgressBar(self.groupBox_2)
        self.sync_progressbar.setGeometry(QtCore.QRect(160, 90, 351, 23))
        self.sync_progressbar.setProperty("value", 0)
        self.sync_progressbar.setTextVisible(False)
        self.sync_progressbar.setObjectName("sync_progressbar")
        self.label_partnership_1_text = QtGui.QLabel(self.groupBox_2)
        self.label_partnership_1_text.setGeometry(QtCore.QRect(170, 20, 241, 18))
        self.label_partnership_1_text.setObjectName("label_partnership_1_text")
        self.label_partnership_3_text = QtGui.QLabel(self.groupBox_2)
        self.label_partnership_3_text.setGeometry(QtCore.QRect(170, 60, 241, 18))
        self.label_partnership_3_text.setObjectName("label_partnership_3_text")
        self.label_partnership_2_text = QtGui.QLabel(self.groupBox_2)
        self.label_partnership_2_text.setGeometry(QtCore.QRect(170, 40, 241, 18))
        self.label_partnership_2_text.setObjectName("label_partnership_2_text")
        self.label_sync_stat = QtGui.QLabel(self.groupBox_2)
        self.label_sync_stat.setGeometry(QtCore.QRect(10, 90, 121, 18))
        self.label_sync_stat.setObjectName("label_sync_stat")
        self.activesyncStatusIcon = QtGui.QToolButton(self.groupBox_2)
        self.activesyncStatusIcon.setEnabled(True)
        self.activesyncStatusIcon.setGeometry(QtCore.QRect(450, 20, 61, 61))
        self.activesyncStatusIcon.setObjectName("activesyncStatusIcon")
        self.labelSyncEngineNotRunning_2 = QtGui.QLabel(self.groupBox_2)
        self.labelSyncEngineNotRunning_2.setGeometry(QtCore.QRect(120, 40, 281, 18))
        font = QtGui.QFont()
        font.setWeight(75)
        font.setBold(True)
        self.labelSyncEngineNotRunning_2.setFont(font)
        self.labelSyncEngineNotRunning_2.setScaledContents(True)
        self.labelSyncEngineNotRunning_2.setObjectName("labelSyncEngineNotRunning_2")
        self.labelDisplayPictureDevice = QtGui.QLabel(synce_kpm_mainwindow)
        self.labelDisplayPictureDevice.setGeometry(QtCore.QRect(10, 10, 190, 190))
        self.labelDisplayPictureDevice.setObjectName("labelDisplayPictureDevice")

        self.retranslateUi(synce_kpm_mainwindow)
        self.tabWidget.setCurrentIndex(0)
        QtCore.QMetaObject.connectSlotsByName(synce_kpm_mainwindow)

    def retranslateUi(self, synce_kpm_mainwindow):
        synce_kpm_mainwindow.setWindowTitle(kdecore.i18n("SynCE KDE PDA Manager"))
        self.groupBox.setTitle(kdecore.i18n("Device Status"))
        self.label.setText(kdecore.i18n("Device:"))
        self.toolButtonDeviceIsLocked.setText(kdecore.i18n("..."))
        self.labelDeviceIsLocked.setText(kdecore.i18n("Device is locked, please unlock"))
        self.pushButton_Quit.setText(kdecore.i18n("&Quit"))
        self.label_3.setText(kdecore.i18n("Owner:"))
        self.label_5.setText(kdecore.i18n("Model name:"))
        self.label_7.setText(kdecore.i18n("Total"))
        self.label_9.setText(kdecore.i18n("Storage space"))
        self.label_11.setText(kdecore.i18n("Used:"))
        self.label_13.setText(kdecore.i18n("Free:"))
        self.label_2.setText(kdecore.i18n("Battery status:"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab), kdecore.i18n("System Information"))
        self.pushButton_Refresh.setText(kdecore.i18n("&Refresh"))
        self.pushButton_Uninstall.setText(kdecore.i18n("&Uninstall"))
        self.pushButton_InstallCAB.setText(kdecore.i18n("&Install CAB"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_2), kdecore.i18n("Software Manager"))
        self.labelSyncEngineNotRunning.setText(kdecore.i18n("Make sure Sync-Engine is running..."))
        self.button_add_pship.setText(kdecore.i18n("Add"))
        self.button_delete_pship.setText(kdecore.i18n("Delete"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_3), kdecore.i18n("Partnership manager"))
        self.tabWidget.setTabText(self.tabWidget.indexOf(self.tab_4), kdecore.i18n("Registry Viewer"))
        self.groupBox_2.setTitle(kdecore.i18n("ActiveSync Status"))
        self.activesyncStatusIcon.setText(kdecore.i18n("..."))
        self.labelSyncEngineNotRunning_2.setText(kdecore.i18n("Make sure Sync-Engine is running..."))

