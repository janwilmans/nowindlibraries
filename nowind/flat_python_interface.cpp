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
insertDisk(PyObject *self, PyObject *args)
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
    gNwhostService->hostImage();
    return Py_BuildValue("i", lReturn);
}

static PyMethodDef nowind_methods[] = {
	{"init", nowind_init, METH_VARARGS, "init() doc string"},
	{"insertDisk", insertDisk, METH_VARARGS, "insertDisk() doc string"},
	{"hostImage", hostImage, METH_VARARGS, "hostImage() doc string"},
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
