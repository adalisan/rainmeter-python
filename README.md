rainmeter-python
================

Plugin for Rainmeter enabling Python 3 scripting

Installation
------------

Binaries: [x86-32](http://www.jblume.com/rainmeter-python/x86/Python.dll) and [x86-64](http://www.jblume.com/rainmeter-python/x64/Python.dll)

Simply copy the appropriate file to the 'Plugins' folder of your Rainmeter installation.

For this plugin to function, you need to install the Python 3.3 distribution matching your Rainmeter's architecture.
The corresponding 'python33.dll' needs to be in your DLL search path; all standard installers of Python 3.3 automatically put the DLL into your System32 directory, so this should normally be the case.

Example (Simple)
-------
```ini
[Measure]
Measure=Plugin
Plugin=Plugins\Python.dll
PythonHome=c:\Python33
ScriptPath=default.py
ClassName=Measure
UpdateDivider=1
```

```python
class Measure:
  def Reload(self, rm, maxValue):
    rm.RmLog(rm.LOG_NOTICE, "Reload called")

  def Update(self):
    return 1.0

  def GetString(self):
    return 'Test'

  def ExecuteBang(self, args):
    pass

  def Finalize(self):
    pass
```


Example (IMAP Unread Mail Count)
-------
```ini
[Measure]
Measure=Plugin
Plugin=Plugins\Python.dll
ScriptPath=IMAP.py
PythonHome=c:\Python33
UpdateDivider=60
Username=username
Password=password
Host=mail.com
```

```python
import imaplib

class Measure:
    def Reload(self, nm, maxValue):
        self.host = nm.RmReadString('Host', 'example.com', False)
        self.username = nm.RmReadString('Username', 'user', False)
        self.password = nm.RmReadString('Password', 'pass', False)

    def Update(self):
        con = imaplib.IMAP4(self.host)
        con.starttls()
        con.login(self.username, self.password)
        con.select('INBOX', True)
        _, msgnums = con.search(None, '(UNSEEN)')
        return float(len(msgnums[0].split()))
```
