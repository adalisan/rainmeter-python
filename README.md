rainmeter-python
================

Originally created by [jblume](https://github.com/jblume), now maintained by [glitchassassin](https://github.com/glitchassassin)

Plugin for Rainmeter enabling Python 3 scripting

Installation
------------

All Releases: [![Github All Releases](https://img.shields.io/github/downloads/glitchassassin/rainmeter-python/total.svg)](https://github.com/glitchassassin/rainmeter-python/releases)

Unzip the DLL for your Python version and copy it to the 'Plugins' folder of your Rainmeter installation.

For this plugin to function, you'll need to install the Python distribution matching your Rainmeter's architecture (Win32 or x64). If in doubt, use Python x64.
The corresponding 'python3x.dll' needs to be in your DLL search path; all standard installers of Python 3 automatically put the DLL into your System32 directory, so this should normally be the case. If you see an "Error 126" in Rainmeter's logs while trying to load the plugin, this is probably your issue.

Example (Simple)
-------
```ini
[Measure]
Measure=Plugin
Plugin=Python
PythonHome=c:\Python35-x64
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
Plugin=Python
ScriptPath=IMAP.py
PythonHome=c:\Python35-x64
ClassName=Measure
UpdateDivider=60
Username=username
Password=password
Host=mail.com
```

```python
import imaplib

class Measure:
    def Reload(self, rm, maxValue):
        self.host = rm.RmReadString('Host', 'example.com', False)
        self.username = rm.RmReadString('Username', 'user', False)
        self.password = rm.RmReadString('Password', 'pass', False)

    def Update(self):
        con = imaplib.IMAP4(self.host)
        con.starttls()
        con.login(self.username, self.password)
        con.select('INBOX', True)
        _, msgnums = con.search(None, '(UNSEEN)')
        con.close()
        con.logout()
        return float(len(msgnums[0].split()))
```
