# USB DMX Interface
This project is a usb dmx interface with four ports, based on the STM32G441. It will register itself as an FX5-Clone as to be recognized by most software. It will register four HID-Devices, one for each port.

![](https://github.com/CShark/usb_dmx/raw/master/Images/project.jpg)

## Schematic
The project uses an STM32G441RB clocked at ~140MHz. The ports are controlled by a MAX3440E each, which will provide some ESD protection in addition to the TVS-Diodes. The device will draw at most 300mA, galvanic isolation was omitted because it needs too much space :D, and because it is not recommended for the transmitting ports of DMX-Devices anyway (although it is recommended for receiving ports, but not mandatory).

![](https://github.com/CShark/usb_dmx/raw/master/Images/schematic.png)

## PCB
The PCB is designed as a four layer board, as with this small size the price was acceptable. The pcb was designed with a `TEKAL 31.29`-case from TEKO in mind, placing three ports at the rear and one port beside the usb port at the front of the case.
![](https://github.com/CShark/usb_dmx/raw/master/Images/render.jpg)
![](https://github.com/CShark/usb_dmx/raw/master/Images/render_2.jpg)

## Firmware
The firmware is completely custom, only based on CMSIS (no HAL) and aims to be compatible with the FX5-Device, which is no longer in production. In theory, the ports can be configured using the resistors at the backside of the pcb but I did not program the logic for it, so currently the first three are Outputs and the fourth is an Input. The configuration can easily be changed at the beginning of main.

To adjust the name of the interface, look at `USB_GetString` in the `usb_config.c`.

## QLC+ Integration
The device works with QLC+ out of the box, but will show up as four devices with the same name and all having input and output capabilites. Therefore I extended the capabilities of the hidplugin a little bit to accomodate for multiple interfaces in a single usb-device. The following changes have been made:

- The Naming of the device has been adjusted to contain an index, if a single USB-Device exposes multiple USB-Interface definitions
- QLC+ will query a string descriptor at `0xA0 + Interface number` to fetch the capabilities of this interface. If the descriptor is not present, it is assumed to offer both transmit and receive capabilities
- The handling of Input messages has been changed, as the old implementation did assume that every HID-Device has an input (which is no longer a given) and therefore mapped signals to the wrong interfaces

Here is a list of all the diffs, the changed source files are included in this repo as well as a precompiled plugin as this can be a little tedious to set up.

<details>
<summary>hiddevice.h</summary>

```diff
diff --git a/plugins/hid/hiddevice.h b/plugins/hid/hiddevice.h
index 6d352e9d2..614358933 100644
--- a/plugins/hid/hiddevice.h
+++ b/plugins/hid/hiddevice.h
@@ -83,6 +83,8 @@ public:
 protected:
     QString m_filename;
     QFile m_file;
+    bool m_Input = true;
+    bool m_Output = true;
 
     /*************************************************************************
      * Line
```

</details>

<details>
<summary>hiddmxdevice.cpp</summary>

```diff
diff --git a/plugins/hid/hiddmxdevice.cpp b/plugins/hid/hiddmxdevice.cpp
index 5bec1d3e5..7e263e1a7 100644
--- a/plugins/hid/hiddmxdevice.cpp
+++ b/plugins/hid/hiddmxdevice.cpp
@@ -37,11 +37,12 @@
 #include "hidapi.h"
 #include "hidplugin.h"
 
-HIDDMXDevice::HIDDMXDevice(HIDPlugin* parent, quint32 line, const QString &name, const QString& path)
+HIDDMXDevice::HIDDMXDevice(HIDPlugin* parent, quint32 line, const QString &name, const int &iface, const QString& path)
     : HIDDevice(parent, line, name, path)
 {
     m_capabilities = QLCIOPlugin::Output;
     m_mode = DMX_MODE_NONE;
+    m_interface = iface;
     init();
 }
 
@@ -65,12 +66,30 @@ void HIDDMXDevice::init()
         return;
     }
 
+    if(m_interface >= 0) {
+        wchar_t buffer[8] = {0};
+        int result = hid_get_indexed_string(m_handle, m_interface | 0xA0, buffer, 8);
+
+        if(result == 0) {
+            m_Input = buffer[0] != '0';
+            m_Output = buffer[1] != '0';
+        }
+    }
+
     /** Reset channels when opening the interface: */
     m_dmx_cmp.fill(0, 512);
     m_dmx_in_cmp.fill(0, 512);
     outputDMX(m_dmx_cmp, true);
 }
 
+bool HIDDMXDevice::hasInput() {
+    return m_Input;
+}
+
+bool HIDDMXDevice::hasOutput() {
+    return m_Output;
+}
+
 /*****************************************************************************
  * File operations
  *****************************************************************************/
```

</details>

<details>
<summary>hiddmxdevice.h</summary>

```diff
diff --git a/plugins/hid/hiddmxdevice.h b/plugins/hid/hiddmxdevice.h
index fb8720628..f9ae11661 100644
--- a/plugins/hid/hiddmxdevice.h
+++ b/plugins/hid/hiddmxdevice.h
@@ -51,7 +51,7 @@ class HIDDMXDevice : public HIDDevice
     Q_OBJECT
 
 public:
-    HIDDMXDevice(HIDPlugin* parent, quint32 line, const QString& name, const QString& path);
+    HIDDMXDevice(HIDPlugin* parent, quint32 line, const QString& name, const int &iface, const QString& path);
     virtual ~HIDDMXDevice();
 
 protected:
@@ -59,10 +59,10 @@ protected:
     void init();
 
     /** @reimp */
-    bool hasInput() { return true; }
+    bool hasInput();
 
     /** @reimp */
-    bool hasOutput() { return true; }
+    bool hasOutput();
 
     /*********************************************************************
      * File operations
@@ -126,6 +126,9 @@ private:
     /** The device current open mode */
     int m_mode;
 
+    /** The interface number */
+    int m_interface;
+
     /** Last universe data that has been received */
     QByteArray m_dmx_in_cmp;
 ```

</details>

<details>
<summary>hidplugin.cpp</summary>

```diff
diff --git a/plugins/hid/hidplugin.cpp b/plugins/hid/hidplugin.cpp
index 062d60679..5d9562025 100644
--- a/plugins/hid/hidplugin.cpp
+++ b/plugins/hid/hidplugin.cpp
@@ -68,7 +68,7 @@ int HIDPlugin::capabilities() const
 
 bool HIDPlugin::openInput(quint32 input, quint32 universe)
 {
-    HIDDevice* dev = device(input);
+    HIDDevice* dev = deviceInput(input);
     if (dev != NULL)
     {
         connect(dev, SIGNAL(valueChanged(quint32,quint32,quint32,uchar)),
@@ -83,7 +83,7 @@ bool HIDPlugin::openInput(quint32 input, quint32 universe)
 
 void HIDPlugin::closeInput(quint32 input, quint32 universe)
 {
-    HIDDevice* dev = device(input);
+    HIDDevice* dev = deviceInput(input);
     if (dev != NULL)
     {
         removeFromMap(input, universe, Input);
@@ -136,7 +136,7 @@ QString HIDPlugin::inputInfo(quint32 input)
     {
         /* A specific input line selected. Display its information if
            available. */
-        HIDDevice* dev = device(input);
+        HIDDevice* dev = deviceInput(input);
         if (dev != NULL)
             str += dev->infoText();
     }
@@ -273,29 +273,37 @@ void HIDPlugin::rescanDevices()
                 (cur_dev->vendor_id == HID_DMX_INTERFACE_VENDOR_ID_4
                 && cur_dev->product_id == HID_DMX_INTERFACE_PRODUCT_ID_4))
         {
+            QString name = QString::fromWCharArray(cur_dev->manufacturer_string) + " " + QString::fromWCharArray(cur_dev->product_string);
+
+            if(cur_dev->interface_number >= 0) {
+                name += QString(": %1").arg(cur_dev->interface_number);
+            }
+
             /* Device is a USB DMX Interface, add it */
-            dev = new HIDDMXDevice(this, line++,
-                                   QString::fromWCharArray(cur_dev->manufacturer_string) + " " +
-                                   QString::fromWCharArray(cur_dev->product_string),
+            dev = new HIDDMXDevice(this, line,
+                                   name, cur_dev->interface_number,
                                    QString(cur_dev->path));
             addDevice(dev);
         }
 #if defined(Q_WS_X11) || defined(Q_OS_LINUX)
         else if (QString(cur_dev->path).contains("js"))
         {
-            dev = new HIDLinuxJoystick(this, line++, cur_dev);
+            dev = new HIDLinuxJoystick(this, line, cur_dev);
 #elif defined(WIN32) || defined (Q_OS_WIN)
         else if(HIDWindowsJoystick::isJoystick(cur_dev->vendor_id, cur_dev->product_id) == true)
         {
-            dev = new HIDWindowsJoystick(this, line++, cur_dev);
+            dev = new HIDWindowsJoystick(this, line, cur_dev);
 #elif defined (__APPLE__) || defined(Q_OS_MACX)
         else if(HIDOSXJoystick::isJoystick(cur_dev->usage) == true)
         {
-            dev = new HIDOSXJoystick(this, line++, cur_dev);
+            dev = new HIDOSXJoystick(this, line, cur_dev);
 #endif
             addDevice(dev);
         }
 
+        if(dev != NULL && dev->hasInput()) {
+            line++;
+        }
         cur_dev = cur_dev->next;
     }
 
@@ -353,6 +361,24 @@ HIDDevice* HIDPlugin::deviceOutput(quint32 index)
     return NULL;
 }
 
+HIDDevice* HIDPlugin::deviceInput(quint32 index)
+{
+    QListIterator <HIDDevice*> it(m_devices);
+    quint32 pos = 0;
+    while (it.hasNext() == true)
+    {
+        HIDDevice* dev = it.next();
+        if (dev->hasInput())
+        {
+            if (pos == index)
+                return dev;
+            else
+                pos++;
+        }
+    }
+    return NULL;
+}
+
 void HIDPlugin::addDevice(HIDDevice* device)
 {
     Q_ASSERT(device != NULL);
```

</details>

<details>
<summary>hidplugin.h</summary>

```diff
diff --git a/plugins/hid/hidplugin.h b/plugins/hid/hidplugin.h
index e38a03b5f..da63a742a 100644
--- a/plugins/hid/hidplugin.h
+++ b/plugins/hid/hidplugin.h
@@ -119,6 +119,7 @@ protected:
     HIDDevice* device(const QString& path);
     HIDDevice* device(quint32 index);
     HIDDevice* deviceOutput(quint32 index);
+    HIDDevice* deviceInput(quint32 index);
 
     void addDevice(HIDDevice* device);
     void removeDevice(HIDDevice* device);
```

</details>