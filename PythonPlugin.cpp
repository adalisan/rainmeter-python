/*
  Copyright (C) 2013 Johannes Blume

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <Windows.h>
#include <Python.h>
#include "./rainmeter-plugin-sdk/API/RainmeterAPI.h"

PyObject* CreateRainmeterObject(void *rm);

PyThreadState *mainThreadState = NULL;

struct Measure
{
	Measure()
	{
		measureObject = NULL;
		getStringResult = NULL;
	}

	PyObject *measureObject;
	wchar_t *getStringResult;
};

PLUGIN_EXPORT void Initialize(void** data, void* rm)
{
	LPCWSTR pythonHome = RmReadString(rm, L"PythonHome", NULL, FALSE);
	if (pythonHome != NULL)
	{
		Py_SetPythonHome((wchar_t*) pythonHome);
	}
	Py_Initialize();
	PyEval_InitThreads();
	mainThreadState = PyThreadState_Get();
	Measure *measure = new Measure;
	*data = measure;
	PyEval_SaveThread();
}

void AddDirToPath(LPCWSTR dir)
{
	PyObject *pathObj = PySys_GetObject("path");
	PyObject *scriptDirObj = PyUnicode_FromWideChar(dir, -1);
	if (!PySequence_Contains(pathObj, scriptDirObj))
	{
		PyList_Append(pathObj, scriptDirObj);
	}
	Py_DECREF(scriptDirObj);
}

void LoadScript(LPCWSTR scriptPath, char* fileName, LPCWSTR className, Measure* measure)
{
	try 
	{
		FILE* f = _Py_wfopen(scriptPath, L"r");
		if (f == NULL)
		{
			throw L"Error opening Python script";
		}

		PyObject *globals = PyModule_GetDict(PyImport_AddModule("__main__"));
		PyObject *result = PyRun_FileEx(f, fileName, Py_file_input, globals, globals, 1);
		if (result == NULL)
		{
			throw L"Error loading Python script";
		}

		Py_DECREF(result);
		PyObject *classNameObj = PyUnicode_FromWideChar(className, -1);
		PyObject *classObj = PyDict_GetItem(globals, classNameObj);
		Py_DECREF(classNameObj);
		if (classObj == NULL)
		{
			throw L"Python class not found";
		}

		PyObject *measureObj = PyObject_CallObject(classObj, NULL);
		if (measureObj == NULL)
		{
			throw L"Error instantiating Python class";
		}

		measure->measureObject = measureObj;
		measure->getStringResult = NULL;
	}
	catch (wchar_t *error)
	{
		measure->measureObject = NULL;
		measure->getStringResult = error;
	}
}

PLUGIN_EXPORT void Reload(void* data, void* rm, double* maxValue)
{
	Measure *measure = (Measure*) data;
	PyEval_RestoreThread(mainThreadState);

	if (measure->measureObject == NULL)
	{
		LPCWSTR scriptPath = RmReadPath(rm, L"ScriptPath", L"default.py");
		wchar_t scriptBaseName[_MAX_FNAME];
		wchar_t scriptExt[_MAX_EXT];
		wchar_t scriptDir[_MAX_DIR];
		_wsplitpath_s(scriptPath, NULL, 0, scriptDir, _MAX_DIR, scriptBaseName, _MAX_FNAME, scriptExt, _MAX_EXT);
		AddDirToPath(scriptDir);

		wchar_t fileName[_MAX_FNAME + 1 + _MAX_EXT];
		lstrcpyW(fileName, scriptBaseName);
		lstrcatW(fileName, L".");
		lstrcatW(fileName, scriptExt);
		char fileNameMb[_MAX_FNAME + 1 + _MAX_EXT];
		wcstombs_s(NULL, fileNameMb, sizeof(fileNameMb), fileName, sizeof(fileName));
		LPCWSTR className = RmReadString(rm, L"ClassName", L"Measure", FALSE);
		LoadScript(scriptPath, fileNameMb, className, measure);
	}

	if (measure->measureObject != NULL)
	{
		PyObject *rainmeterObject = CreateRainmeterObject(rm);
		PyObject *resultObj = PyObject_CallMethod(measure->measureObject, "Reload", "Od", rainmeterObject, maxValue);
		if (resultObj != NULL)
		{
			Py_DECREF(resultObj);
		}
		else
		{
			PyErr_Clear();
		}
		Py_DECREF(rainmeterObject);
	}

	PyEval_SaveThread();
}

PLUGIN_EXPORT double Update(void* data)
{
	Measure *measure = (Measure*) data;
	if (measure->measureObject == NULL)
	{
		return 0.0;
	}
	PyEval_RestoreThread(mainThreadState);
	PyObject *resultObj = PyObject_CallMethod(measure->measureObject, "Update", NULL);
	double result = 0.0;
	if (resultObj != NULL)
	{
		result = PyFloat_Check(resultObj) ? PyFloat_AsDouble(resultObj) : 0.0;
		Py_DECREF(resultObj);
	}
	else
	{
		PyErr_Clear();
	}
	PyEval_SaveThread();
	return result;
}

PLUGIN_EXPORT LPCWSTR GetString(void* data)
{
	Measure *measure = (Measure*) data;
	if (measure->measureObject == NULL)
	{
		return measure->getStringResult;
	}

	PyEval_RestoreThread(mainThreadState);
	PyObject *resultObj = PyObject_CallMethod(measure->measureObject, "GetString", NULL);
	if (measure->getStringResult != NULL)
	{
		PyMem_Free(measure->getStringResult);
		measure->getStringResult = NULL;
	}
	if (resultObj != NULL)
	{
		if (resultObj != Py_None)
		{
			PyObject *strObj = PyObject_Str(resultObj);
			measure->getStringResult = PyUnicode_AsWideCharString(strObj, NULL);
			Py_DECREF(strObj);
		}
		Py_DECREF(resultObj);
	}
	else
	{
		PyErr_Clear();
	}
	PyEval_SaveThread();
	return measure->getStringResult;
}

PLUGIN_EXPORT void ExecuteBang(void* data, LPCWSTR args)
{
	Measure *measure = (Measure*) data;
	if (measure->measureObject == NULL)
	{
		return;
	}

	PyEval_RestoreThread(mainThreadState);
	PyObject *argsObj = PyUnicode_FromWideChar(args, -1);
	PyObject *resultObj = PyObject_CallMethod(measure->measureObject, "ExecuteBang", "O", argsObj);
	if (resultObj != NULL)
	{
		Py_DECREF(resultObj);
	}
	else
	{
		PyErr_Clear();
	}
	Py_DECREF(argsObj);
	PyEval_SaveThread();
}

PLUGIN_EXPORT void Finalize(void* data)
{
	Measure *measure = (Measure*) data;
	PyEval_RestoreThread(mainThreadState);
	if (measure->measureObject != NULL)
	{
		PyObject *resultObj = PyObject_CallMethod(measure->measureObject, "Finalize", NULL);
		if (resultObj != NULL)
		{
			Py_DECREF(resultObj);
		}
		else
		{
			PyErr_Clear();
		}

		if (measure->getStringResult != NULL)
		{
			PyMem_Free(measure->getStringResult);
		}
	}
	delete measure;
	//Py_Finalize(); // Testing this without killing the interpreter to reset its status
}
