#include <Python.h>
#include "NwhostService.h"
#include <libftdx.h>
using namespace nowind;

static NwhostService* gNwhostService = 0;

static PyObject *
nowind_init(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command)) return NULL;
    sts = 0; //system(command);
	printf("cmd: %s", command);
    return Py_BuildValue("i", sts);
}

static PyObject *
insertDisk(PyObject *self, PyObject *args)
{
    const char *lFileName;
    if (!PyArg_ParseTuple(args, "s", &lFileName)) return NULL;
    gNwhostService->setImage(0, lFileName);
}

static PyObject *
hostImage(PyObject *self, PyObject *args)
{
    gNwhostService->hostImage();
}

static PyMethodDef nowind_methods[] = {
	{"init", nowind_init, METH_VARARGS, "system() doc string"},
	{NULL, NULL}
};

PyMODINIT_FUNC
initlibnowind(void)
{
	Py_InitModule("nowind", nowind_methods);

    nowind::initialize();
    gNwhostService = new NwhostService();
    gNwhostService->start(ftdx::eLibUsb);
}
