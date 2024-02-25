#include <stdio.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <Python.h>
#include <pythread.h>
#include <structmember.h>
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
#include "JpegCoder.hpp"

typedef struct
{
    PyObject_HEAD
    JpegCoder* m_handle;
} GpuJpeg;


static PyMemberDef GpuJpeg_DataMembers[] =
{
        {(char*)"m_handle",   T_OBJECT, offsetof(GpuJpeg, m_handle),   0, (char*)"GpuJpeg handle ptr"},
        {NULL, 0, 0, 0, NULL}
};

int GpuJpeg_init(PyObject *self, PyObject *args, PyObject *kwds) {
  ((GpuJpeg*)self)->m_handle = new JpegCoder();
  return 0;
}


static void GpuJpeg_Destruct(PyObject* self)
{
    delete (JpegCoder*)(((GpuJpeg*)self)->m_handle);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject* GpuJpeg_Str(PyObject* Self)
{
    return Py_BuildValue("s", "<gpujpeg-python.gpujpeg>");
}

static PyObject* GpuJpeg_Repr(PyObject* Self)
{
    return GpuJpeg_Str(Self);
}

static PyObject* GpuJpeg_read(GpuJpeg* Self, PyObject* Argvs)
{
	JpegCoder* m_handle = (JpegCoder*)Self->m_handle;
    JpegDecoderBytes* img;
	unsigned char* jpegFile;
	
    if(!PyArg_ParseTuple(Argvs, "s", &jpegFile)){
        PyErr_SetString(PyExc_ValueError, "Parse the argument FAILED! You should pass jpeg file path string!");
        return NULL;
    }
   
    m_handle->ensureThread(PyThread_get_thread_ident());
    img = m_handle->read((const char* )jpegFile);
	
	npy_intp dims[3] = {(npy_intp)(img->height), (npy_intp)(img->width), 3};
	PyObject* temp = PyArray_SimpleNewFromData(3, dims, NPY_UINT8, img->data);
	delete(img);
    return temp;
}

static PyObject* GpuJpeg_decode(GpuJpeg* Self, PyObject* Argvs)
{		
	JpegCoder* m_handle = (JpegCoder*)Self->m_handle;
    JpegDecoderBytes* img;
	
    Py_buffer pyBuf;
    unsigned char* jpegData;
    if(!PyArg_ParseTuple(Argvs, "y*", &pyBuf)){
        PyErr_SetString(PyExc_ValueError, "Parse the argument FAILED! You should pass jpegData byte string!");
        return NULL;
    }
    
    jpegData = (unsigned char*)pyBuf.buf;
    try {
        m_handle->ensureThread(PyThread_get_thread_ident());
        img = m_handle->decode((uint8_t *)jpegData, (size_t)pyBuf.len);
        PyBuffer_Release(&pyBuf);
    } catch (JpegCoderError e) {		
        PyBuffer_Release(&pyBuf);
        PyErr_Format(PyExc_ValueError, "%s, Code: %d", e.what(), e.code());
        return NULL;
    }
	
	npy_intp dims[3] = {(npy_intp)(img->height), (npy_intp)(img->width), 3};
	PyObject* temp = PyArray_SimpleNewFromData(3, dims, NPY_UINT8, img->data);
	delete(img);
    return temp;
}

static PyObject* GpuJpeg_write(GpuJpeg* Self, PyObject* Argvs)
{
	unsigned char* jpegFile;
    PyArrayObject *vecin;
    unsigned int quality = 70;
    if (!PyArg_ParseTuple(Argvs, "sO!|I", &jpegFile, &PyArray_Type, &vecin, &quality)){
        PyErr_SetString(PyExc_ValueError, "Parse the argument FAILED! You should pass a filename and a BGR image numpy array!");
        return NULL;
    }

    if (NULL == vecin){
        Py_INCREF(Py_None);
        return Py_None;
    }

    if (PyArray_NDIM(vecin) != 3){
        PyErr_SetString(PyExc_ValueError, "Parse the argument FAILED! You should pass BGR image numpy array by height*width*channel!");
        return NULL;
    }

    if(quality>100){
        quality = 100;
    }

    JpegCoder* m_handle = (JpegCoder*)Self->m_handle;

    PyObject* bytes = PyObject_CallMethod((PyObject*)vecin, "tobytes", NULL);

    Py_buffer pyBuf;

    unsigned char* buffer;
    PyArg_Parse(bytes, "y*", &pyBuf);
    buffer = (unsigned char*)pyBuf.buf;

    m_handle->ensureThread(PyThread_get_thread_ident());
	
	size_t width = PyArray_DIM(vecin, 1);
	size_t height = PyArray_DIM(vecin, 0);
	size_t channels = 3;
    int jpegDataSize = m_handle->write((const char* )jpegFile, width, height, channels, buffer, quality);

    PyBuffer_Release(&pyBuf);
    Py_DECREF(bytes);

    return Py_BuildValue("l", (long)jpegDataSize);
}

static PyObject* GpuJpeg_encode(GpuJpeg* Self, PyObject* Argvs)
{
    PyArrayObject *vecin;
    unsigned int quality = 70;
    if (!PyArg_ParseTuple(Argvs, "O!|I", &PyArray_Type, &vecin, &quality)){
        PyErr_SetString(PyExc_ValueError, "Parse the argument FAILED! You should pass BGR image numpy array!");
        return NULL;
    }

    if (NULL == vecin){
        Py_INCREF(Py_None);
        return Py_None;
    }

    if (PyArray_NDIM(vecin) != 3){
        PyErr_SetString(PyExc_ValueError, "Parse the argument FAILED! You should pass BGR image numpy array by height*width*channel!");
        return NULL;
    }

    if(quality>100){
        quality = 100;
    }

    JpegCoder* m_handle = (JpegCoder*)Self->m_handle;

    PyObject* bytes = PyObject_CallMethod((PyObject*)vecin, "tobytes", NULL);

    Py_buffer pyBuf;

    unsigned char* buffer;
    PyArg_Parse(bytes, "y*", &pyBuf);
    buffer = (unsigned char*)pyBuf.buf;

    m_handle->ensureThread(PyThread_get_thread_ident());
	
	size_t width = PyArray_DIM(vecin, 1);
	size_t height = PyArray_DIM(vecin, 0);
	size_t channels = 3;
    auto data = m_handle->encode(width, height, channels, buffer, quality);

    PyBuffer_Release(&pyBuf);
    Py_DECREF(bytes);

    PyObject* rtn = PyBytes_FromStringAndSize((const char*)data->data, data->size);

    delete(data);
    return rtn;
}



static PyMethodDef GpuJpeg_MethodMembers[] =
{
        {"imencode",  (PyCFunction)GpuJpeg_encode,  METH_VARARGS,  "encode jpeg"},
        {"imdecode", (PyCFunction)GpuJpeg_decode, METH_VARARGS,  "decode jpeg"},
        {"imread", (PyCFunction)GpuJpeg_read, METH_VARARGS,  "read jpeg file and decode"},
        {"imwrite", (PyCFunction)GpuJpeg_write, METH_VARARGS,  "encode and write jpeg file"},
        {NULL, NULL, 0, NULL}
};


static PyTypeObject GpuJpeg_ClassInfo =
{
        PyVarObject_HEAD_INIT(NULL, 0)
        "gpujpg.GpuJpeg",
        sizeof(GpuJpeg),
        0
};


void GpuJpeg_module_destroy(void *_){
    JpegCoder::cleanUpEnv();
}

static PyModuleDef ModuleInfo =
{
        PyModuleDef_HEAD_INIT,
        "GpuJpeg Module",
        "GpuJpeg by Daniel S.",
        -1,
        NULL, NULL, NULL, NULL,
        GpuJpeg_module_destroy
};

PyMODINIT_FUNC
PyInit_gpujpeg(void) {
    PyObject * pReturn = NULL;

    GpuJpeg_ClassInfo.tp_dealloc   = GpuJpeg_Destruct;
    GpuJpeg_ClassInfo.tp_repr      = GpuJpeg_Repr;
    GpuJpeg_ClassInfo.tp_str       = GpuJpeg_Str;
    GpuJpeg_ClassInfo.tp_flags     = Py_TPFLAGS_DEFAULT;
    GpuJpeg_ClassInfo.tp_doc       = "GpuJpeg Python Objects";
    GpuJpeg_ClassInfo.tp_weaklistoffset = 0;
    GpuJpeg_ClassInfo.tp_methods   = GpuJpeg_MethodMembers;
    GpuJpeg_ClassInfo.tp_members   = GpuJpeg_DataMembers;
    GpuJpeg_ClassInfo.tp_dictoffset = 0;
    GpuJpeg_ClassInfo.tp_init      = GpuJpeg_init;
    GpuJpeg_ClassInfo.tp_new = PyType_GenericNew;

    if(PyType_Ready(&GpuJpeg_ClassInfo) < 0) 
        return NULL;

    pReturn = PyModule_Create(&ModuleInfo);
    if(pReturn == NULL)
        return NULL;

    Py_INCREF(&ModuleInfo);

    Py_INCREF(&GpuJpeg_ClassInfo);
    if (PyModule_AddObject(pReturn, "GpuJpeg", (PyObject*)&GpuJpeg_ClassInfo) < 0) {
        Py_DECREF(&GpuJpeg_ClassInfo);
        Py_DECREF(pReturn);
        return NULL;
    }

    import_array();
    return pReturn;
}
