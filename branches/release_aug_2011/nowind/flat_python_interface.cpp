#include <Python.h>
#include "NwhostService.h"
#include <libftdx.h>
using namespace nowind;

static NwhostService* gNwhostService = 0;

static PyObject *
nowind_init(PyObject *self, PyObject *args)
{
    //const char *command;
    int lReturn = 0;
    //if (!PyArg_ParseTuple(args, "s", &command)) return NULL;
	printf("Nowind init methods (does nothing)");
    return Py_BuildValue("i", lReturn);
}

static PyObject *
setImage(PyObject *self, PyObject *args)
{
    int lReturn = 0;
    const char *lFileName;
    if (!PyArg_ParseTuple(args, "s", &lFileName)) return NULL;
    gNwhostService->setImage(0, lFileName);
    return Py_BuildValue("i", lReturn);
}

static PyObject *
hostImage(PyObject *self, PyObject *args)
{
    int lReturn = 0;
    gNwhostService->invokeHostImage();
    return Py_BuildValue("i", lReturn);
}

static PyObject *
stopHosting(PyObject *self, PyObject *args)
{
    int lReturn = 0;
    gNwhostService->stopHosting();
    return Py_BuildValue("i", lReturn);
}

static PyMethodDef nowind_methods[] = { 
	{"init", nowind_init, METH_VARARGS, "init() doc string"},
	{"setImage", setImage, METH_VARARGS, "setImage() doc string"},
	{"hostImage", hostImage, METH_VARARGS, "hostImage() doc string"},
	{"stopHosting", stopHosting, METH_VARARGS, "stopHosting() doc string"},
	{NULL, NULL}
};

PyMODINIT_FUNC
initnowind(void)
{
	Py_InitModule("nowind", nowind_methods);

    nowind::initialize();
    gNwhostService = new NwhostService();
    gNwhostService->start(ftdx::eLibUsb);
    printf("init nowind library\n");
}
