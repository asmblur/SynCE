#!/usr/bin/env python
# -*- coding: utf-8 -*-
import sys
import dbus
import dbus.glib
import gtk

class TestApp:
    def __init__(self):
        self.devices = {}

        bus = dbus.SystemBus()
        self.bus = bus

        
        try:
            proxy_obj_dbus = bus.get_object("org.freedesktop.DBus", "/org/freedesktop/DBus")
        except dbus.DBusException:
            print "Error: Could not connect to dbus. Is it started?"
            print "Without DBus there really is no use for this program..."
            sys.exit(1) 


        mgrOdccm = dbus.Interface(proxy_obj_dbus, "org.freedesktop.DBus")
        mgrOdccm.connect_to_signal("NameOwnerChanged", self.odccm_status_changed_cb)

        session_bus = dbus.SessionBus()
        notif_obj = session_bus.get_object("org.freedesktop.Notifications", "/org/freedesktop/Notifications")
        self.notify_iface = dbus.Interface(notif_obj, "org.freedesktop.Notifications")

        
        try:
            proxy_obj = bus.get_object("org.synce.odccm", "/org/synce/odccm/DeviceManager")
            self.odccmRunning = True
        except dbus.DBusException:
            self.odccmRunning = False


        if self.odccmRunning:
            try:
                self.mgr = dbus.Interface(proxy_obj, "org.synce.odccm.DeviceManager")

                self.mgr.connect_to_signal("DeviceConnected", self.device_connected_cb)
                self.mgr.connect_to_signal("DeviceDisconnected", self.device_disconnected_cb)

                for obj_path in self.mgr.GetConnectedDevices():
                    self._add_device(obj_path, False)
            except:
                print "Odccm seems to be running, but there are problems with connecting to it"
                raise

        if self.odccmRunning == False:
            print "Waiting for odccm to start"
        else:
            print "Waiting for device to hotplug"

    def odccm_status_changed_cb(self, obj_path, param2, param3):
        if obj_path == "org.synce.odccm":
            #If this parameter is empty, the odccm just came online 
            if param2 == "":
                self.odccmRunning = True

            #If this parameter is empty, the odccm just went offline
            if param3 == "":
                self.odccmRunning = False
                print "Waiting for odccm to start"
            
            
            if self.odccmRunning:
                try:
                    proxy_obj = self.bus.get_object("org.synce.odccm", "/org/synce/odccm/DeviceManager")
                    mgr = dbus.Interface(proxy_obj, "org.synce.odccm.DeviceManager")

                    mgr.connect_to_signal("DeviceConnected", self.device_connected_cb)
                    mgr.connect_to_signal("DeviceDisconnected", self.device_disconnected_cb)

                    for obj_path in mgr.GetConnectedDevices():
                        self._add_device(obj_path, False)
                    
                    print "Waiting for device to hotplug"
                except:
                    print "Something went really wrong, odccm came online, but we can't connect..."
                    raise



    def device_connected_cb(self, obj_path):
        self._add_device(obj_path, True)

    def device_disconnected_cb(self, obj_path):
        if obj_path in self.devices:
            device = self.devices[obj_path]
            self.notify_iface.Notify("SynCE", 0, "", "PDA disconnected", "'%s' just disconnected." % device.name, [], {}, 3000)
            del self.devices[obj_path]

    def _add_device(self, obj_path, just_connected):
        device = CeDevice(self.bus, obj_path)
        self.devices[obj_path] = device

        if just_connected:
            self.notify_iface.Notify("SynCE", 0, "", "PDA connected", "A %s %s '%s' just connected." % \
                (device.model_name, device.platform_name, device.name), [], {}, 3000)


ODCCM_DEVICE_PASSWORD_FLAG_SET     = 1
ODCCM_DEVICE_PASSWORD_FLAG_PROVIDE = 2

class CeDevice:
    def __init__(self, bus, obj_path):
        self.obj_path = obj_path
        dev_obj = bus.get_object("org.synce.odccm", obj_path)
        dev = dbus.Interface(dev_obj, "org.synce.odccm.Device")
        self.name = dev.GetName()
        self.platform_name = dev.GetPlatformName()
        self.model_name = dev.GetModelName()
        self.dev_iface = dev
        
        self._print_debug()
        
        self._password_flags_changed()

    def _print_debug(self):
        dev = self.dev_iface
        print "Created CeDevice with obj_path=\"%s\"" % self.obj_path
        print "  GetIpAddress:", dev.GetIpAddress()
        print "  GetGuid:", dev.GetGuid()
        print "  GetOsVersion:", dev.GetOsVersion()
        print "  GetName:", dev.GetName()
        print "  GetVersion:", dev.GetVersion()
        print "  GetCpuType:", dev.GetCpuType()
        print "  GetCurrentPartnerId:", dev.GetCurrentPartnerId()
        print "  GetId:", dev.GetId()
        print "  GetPlatformName:", dev.GetPlatformName()
        print "  GetModelName:", dev.GetModelName()
        print "  GetPasswordFlags:", dev.GetPasswordFlags()

    def password_flags_changed_cb(self, added, removed):
        print "password_flags_changed_cb: added=0x%08x removed=0x%08x" % (added, removed)
        self._password_flags_changed()

    def _password_flags_changed(self):
        flags = self.dev_iface.GetPasswordFlags()

        if flags & ODCCM_DEVICE_PASSWORD_FLAG_PROVIDE:
            print "Device requires password, asking user"
            authenticated = False
            while not authenticated:
                dlg = EntryDialog(None, "Password required",
                                  "The PDA '%s' is password-protected.  Enter password:" % self.name,
                                  True)
                if dlg.run() != gtk.RESPONSE_ACCEPT:
                    print "Dialog canceled by user"
                    dlg.destroy()
                    return
                authenticated = self.dev_iface.ProvidePassword(dlg.get_text())
                dlg.destroy()
                if not authenticated:
                    print "Password mismatch"
            print "Password accepted. Have a nice day."
        else:
            print "Device is not requiring a password"


class EntryDialog(gtk.Dialog):
    def __init__(self, parent, title, text, password=False):
        gtk.Dialog.__init__(self, title, parent,
                            gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT | gtk.DIALOG_NO_SEPARATOR,
                            (gtk.STOCK_CANCEL, gtk.RESPONSE_REJECT,
                             gtk.STOCK_OK, gtk.RESPONSE_ACCEPT | gtk.CAN_DEFAULT)
                            )

        self.set_default_response(gtk.RESPONSE_ACCEPT)
        label = gtk.Label(text)
        label.set_alignment(0.0, 0.5)
        self.vbox.pack_start(label, False)
        self._label = label

        entry = gtk.Entry()
        entry.set_visibility(not password)
        entry.set_activates_default(True)
        self.vbox.pack_start(entry, False, True, 5)
        self._entry = entry

        self.show_all()

    def get_text(self):
        return self._entry.get_text()


TestApp()
gtk.main()
