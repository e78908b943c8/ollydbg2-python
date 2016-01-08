#include "main.hpp"
#include "toolbox.hpp"
#include "window.hpp"

#include <cstdio>
#include <cstring>
#include <python.h>
#include <structmember.h>

typedef struct {
	PyObject_HEAD;
	ulong eax;
	ulong ecx;
	ulong edx;
	ulong ebx;
	ulong esp;
	ulong ebp;
	ulong esi;
	ulong edi;
} Ctx;

static void
Ctx_dealloc(Ctx* self)
{
	self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
Ctx_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	Ctx *self;

	self = (Ctx *)type->tp_alloc(type, 0);
	if (self != NULL) {
		self->eax = 0;
		self->ecx = 0;
		self->edx = 0;
		self->ebx = 0;
		self->esp = 0;
		self->ebp = 0;
		self->esi = 0;
		self->edi = 0;
	}

	return (PyObject *)self;
}

static int
Ctx_init(Ctx *self, PyObject *args, PyObject *kwds)
{
	PyObject *first = NULL, *last = NULL;

	static char *kwlist[] = { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", NULL };

	if (!PyArg_ParseTuple(args, "KKKKKKKK", 
		&self->eax, &self->ecx, &self->edx, &self->ebx, &self->esp, &self->ebp, &self->esi, &self->edi)) {
		Addtolist(0x31337, RED, NAME_PLUGIN L" Something went wrong while parsing arguments");
		return -1;
	}

	return 0;
}


static PyMemberDef Ctx_members[] = {
	{"eax", T_ULONG, offsetof(Ctx, eax), 0, "eax" },
	{"ecx", T_ULONG, offsetof(Ctx, ecx), 0, "ecx"},
	{"edx", T_ULONG, offsetof(Ctx, edx), 0, "edx"},
	{"ebx", T_ULONG, offsetof(Ctx, ebx), 0, "ebx"},
	{"esp", T_ULONG, offsetof(Ctx, esp), 0, "esp"},
	{"ebp", T_ULONG, offsetof(Ctx, ebp), 0, "ebp"},
	{"esi", T_ULONG, offsetof(Ctx, esi), 0, "esi"},
	{"edi", T_ULONG, offsetof(Ctx, edi), 0, "edi"},
	{ NULL }  /* Sentinel */
};

static PyObject *
Ctx_name(Ctx* self)
{
	static PyObject *format = NULL;
	PyObject *args, *result;

	if (format == NULL) {
		format = PyString_FromString("%s %s");
		if (format == NULL)
			return NULL;
	}

	result = PyString_Format(format, args);
	Py_DECREF(args);

	return result;
}

static PyMethodDef Ctx_methods[] = {
	{ "name", (PyCFunction)Ctx_name, METH_NOARGS,
	"Return the name, combining the first and last name"
	},
	{ NULL }  /* Sentinel */
};

static PyTypeObject CtxType = {
	PyObject_HEAD_INIT(NULL)
	0,                         /*ob_size*/
	"ollyapiex.Ctx",             /*tp_name*/
	sizeof(Ctx),             /*tp_basicsize*/
	0,                         /*tp_itemsize*/
	(destructor)Ctx_dealloc, /*tp_dealloc*/
	0,                         /*tp_print*/
	0,                         /*tp_getattr*/
	0,                         /*tp_setattr*/
	0,                         /*tp_compare*/
	0,                         /*tp_repr*/
	0,                         /*tp_as_number*/
	0,                         /*tp_as_sequence*/
	0,                         /*tp_as_mapping*/
	0,                         /*tp_hash */
	0,                         /*tp_call*/
	0,                         /*tp_str*/
	0,                         /*tp_getattro*/
	0,                         /*tp_setattro*/
	0,                         /*tp_as_buffer*/
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
	"Ctx objects",           /* tp_doc */
	0,		               /* tp_traverse */
	0,		               /* tp_clear */
	0,		               /* tp_richcompare */
	0,		               /* tp_weaklistoffset */
	0,		               /* tp_iter */
	0,		               /* tp_iternext */
	Ctx_methods,             /* tp_methods */
	Ctx_members,             /* tp_members */
	0,                         /* tp_getset */
	0,                         /* tp_base */
	0,                         /* tp_dict */
	0,                         /* tp_descr_get */
	0,                         /* tp_descr_set */
	0,                         /* tp_dictoffset */
	(initproc)Ctx_init,      /* tp_init */
	0,                         /* tp_alloc */
	Ctx_new,                 /* tp_new */
};

static PyMethodDef module_methods[] = {
	{ NULL }  /* Sentinel */
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
        g_hinst = hinstDLL;

    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

static PyObject *g_onexception;

static PyObject *set_onexception(PyObject *dummy, PyObject *args) {
	PyObject *result = NULL;
	PyObject *temp;

	if (PyArg_ParseTuple(args, "O:set_onexception", &temp)) {
		if (!PyCallable_Check(temp)) {
			PyErr_SetString(PyExc_TypeError, "parameter must be callable");
			return NULL;
		}
		Py_XINCREF(temp);
		Py_XDECREF(g_onexception);
		g_onexception = temp;
		Py_INCREF(Py_None);
		result = Py_None;
	}
	return result;
}

static PyMethodDef methods[] = {
	{"set_onexception", set_onexception, METH_VARARGS, "Set exception callback"},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
init_ollyapiex() {
	PyObject* m;

	if (PyType_Ready(&CtxType) < 0)
		return;

	m = Py_InitModule3("ollyapiex", methods,
		"Example module that creates an extension type.");

	if (m == NULL)
		return;

	Py_INCREF(&CtxType);
	PyModule_AddObject(m, "Ctx", (PyObject *)&CtxType);
}


/*
    This routine is required by the OllyDBG plugin engine! 
*/
extc int __cdecl ODBG2_Pluginquery(int ollydbgversion, ulong *features, wchar_t pluginname[SHORTNAME], wchar_t pluginversion[SHORTNAME])
{
    // Yeah, the plugin interface in the v1/v2 are different
    if(ollydbgversion < 201)
        return 0;

    // Set plugin name and version
    wcscpy_s(pluginname, SHORTNAME, L"python-loader");
    wcscpy_s(pluginversion, SHORTNAME, L"v0.1");

    // Initialize the python environment, prepare the hooks
    Py_Initialize();
    PyEval_InitThreads();

	init_ollyapiex();

	Addtolist(0x31337, RED, NAME_PLUGIN L" Plugin fully initialized.");

    return PLUGIN_VERSION;
}

extc void __cdecl ODBG2_Plugindestroy(void)
{
	// Properly ends the python environment
	Py_Finalize();
}

PyObject *makeCtx(t_reg *reg) {
	auto argList1 = Py_BuildValue(
		"KKKKKKKK", reg->r[0], reg->r[1], reg->r[2], reg->r[3], 
		reg->r[4], reg->r[5], reg->r[6], reg->r[7]);
	auto ctx = PyObject_CallObject((PyObject *)&CtxType, argList1);
	Py_DECREF(argList1);
	return ctx;
}

//no good; when this event is received all register states accessible from python aren't updated yet
extc int __cdecl ODBG2_Pluginexception(t_run *prun, const t_disasm *da, t_thread *pthr, t_reg *preg, wchar_t *message)
{
	Addtolist(0x31337, WHITE, NAME_PLUGIN L" ODB2_Pluginexception");
	if (g_onexception)
	{
		auto ctx = makeCtx(preg);
		if (ctx != NULL)
		{
			//unlike PyArg_ParseTuple, the format should not be "O: on_exception"
			//which causes the process to crash
			//Instead it should be plain "O"
			auto argList2 = Py_BuildValue("(O)", ctx);
			auto result = PyObject_CallObject(g_onexception, argList2);
			Py_DECREF(argList2);
			if (result != NULL) // don't care about the result
				Py_DECREF(result);
			Py_DECREF(ctx);
		}
	}
	return PE_IGNORED;
}

/*
    Adds items to OllyDbgs menu system.
*/
extc t_menu * __cdecl ODBG2_Pluginmenu(wchar_t *type)
{
	if(wcscmp(type, PWM_MAIN) == 0)
        return g_MainMenu;

    return NULL;
}

void spawn_window(void)
{
    wchar_t *file_path = (wchar_t*)malloc(sizeof(wchar_t) * 1024);
    OPENFILENAME ofn = {0};

    if(file_path == NULL)
        return;

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwollymain;
    ofn.lpstrFile = file_path;
    // Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
    // use the contents of szFile to initialize itself.
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = 1024 - 1;
    ofn.lpstrFilter = L"Python files\0*.py\0\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if(GetOpenFileName(&ofn) == TRUE)
    {
        /*
        XXX: Seems to not work when instrumenting OllyDBG2 ; 
            I think the reason is:
                When you call ollydbg!Run it calls kernelbase!ContinueDebugEvent
                "Only the thread that created dwProcessId with the CreateProcess function can call ContinueDebugEvent."

                So when, calling ollydbg!Run from another is wrong ; same thing for ollydbg!Checkfordebugevent.
        
        CreateThread(
            NULL,
            0,
            execute_python_script,
            file_path,
            0,
            NULL
        );
        */
        execute_python_script(file_path);
    }
}

int handle_menu(t_table* pTable, wchar_t* pName, ulong index, int nMode)
{
    if(nMode == MENU_VERIFY)
        return MENU_NORMAL;
    else if(nMode == MENU_EXECUTE)
    {
        switch(index)
        {
            case MENU_LOAD_SCRIPT_IDX:
            {
                spawn_window();
                break;
            }

            case MENU_ABOUT_IDX:
            {
                MessageBox(
                    hwollymain,
                    L"python loader",
                    L"About python-loader",
                    MB_OK| MB_ICONINFORMATION
                );

                break;
            }

            case MENU_CMDLINE_IDX:
            {
                if(CreateCommandLineWindow(hwollymain, g_hinst) == FALSE)
                    Addtolist(0x31337, RED, NAME_PLUGIN L" The command-line window can't be created.");

                break;
            }

            default:
                break;
        }

        return MENU_NOREDRAW;
    }
    else
        return MENU_ABSENT;
}

DWORD WINAPI execute_python_script(LPVOID param)
{
    wchar_t *path = (wchar_t*)param;
    Addtolist(0, WHITE, NAME_PLUGIN L" Trying to execute the script located here: '%s'..", path);

    std::wstring pathW(path);
    std::string pathA(widechar_to_multibytes(pathW));

    PyObject* PyFileObject = PyFile_FromString((char*)pathA.c_str(), "r");
    if(PyFileObject == NULL)
    {
        Addtolist(0, RED, NAME_PLUGIN L" Your file doesn't exist.");
        goto clean;
    }

    PyRun_SimpleFile(PyFile_AsFile(PyFileObject), (char*)pathA.c_str());

    Addtolist(0, WHITE, NAME_PLUGIN L" Execution is done!");

clean:
    free(path);
    return 1;
}