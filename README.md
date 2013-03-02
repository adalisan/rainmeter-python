rainmeter-python
================

Plugin for Rainmeter enabling Python 3 scripting

Installation
------------

For this plugin to function, you need to install the Python 3.3 distribution matching your Rainmeter's architecture.
The corresponding 'python33.dll' needs to be in your DLL search path; all standard installers of Python 3.3 automatically put the DLL into your System32 directory, so this should normally be the case.

Example
-------
```ini
[Measure]
Measure=Plugin
Plugin=Plugins\Python.dll
PythonHome=c:\Python33
ScriptPath=default.py
ClassName=Measure
UpdateRate=1
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
