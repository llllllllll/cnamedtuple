#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <Python.h>
#include <structmember.h>
#include <frameobject.h>


// strdup with PyMem_New instead of malloc.
// Memory should be PyMem_Del'd not free'd.
char *pymem_strdup(char *str){
    size_t len = strlen(str) + 1;  // Account for null terminator.
    char *r    = PyMem_New(char,len);
    memcpy(r,str,len);
    return r;
}


// The type of the returned types from `namedtuple`.
typedef struct{
    PyHeapTypeObject nt_tp;
    char           **nt_fieldv;
    Py_ssize_t       nt_fieldc;
    PyObject        *nt_reprfmt;  // The format for the repr.
}namedtuple;


static PyObject *namedtuple_meta_getfields(PyObject*,void*);
static PyObject *namedtuple_meta_new(PyTypeObject*,PyObject*,PyObject*);
static void namedtuple_meta_dealloc(PyObject*);


// The `getsets` of the metaclass. Allows for grabbing the `nt_fieldv`
// as a `tuple` of `str`.
PyGetSetDef namedtuple_meta_getsets[] = {
    {.name="_fields",
     .get=namedtuple_meta_getfields,
     .set=NULL,
     .closure=NULL},
    {NULL},
};


#define namedtuple_meta_doc \
    "Metaclass for the types returned by the namedtuple factory function."


// The type of the `namedtuple` type.
// Provides the memory management for the extra data.
PyTypeObject namedtuple_meta = {
    PyVarObject_HEAD_INIT(&PyType_Type,0)
    .tp_name="NamedTupleMeta",
    .tp_dealloc=namedtuple_meta_dealloc,
    .tp_basicsize=sizeof(namedtuple),
    .tp_flags=Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc=namedtuple_meta_doc,
    .tp_getset=namedtuple_meta_getsets,
    .tp_base=&PyType_Type,
    .tp_new=namedtuple_meta_new,
};


// Build a python `tuple` object out the fieldv and fieldc of a type.
// return: `PyObject*` pointing to a new `PyTupleObject` or NULL on failure.
static PyObject *tuple_from_fieldnames(char **fieldv,Py_ssize_t fieldc){
    PyObject  *tuple;
    PyObject  *string;
    Py_ssize_t n;

    if (!(tuple = PyTuple_New(fieldc))){
        return NULL;
    }

    for (n = 0;n < fieldc;++n){
        if (!(string = PyString_FromString(fieldv[n]))){
            Py_DECREF(tuple);
            return NULL;
        }

        PyTuple_SET_ITEM(tuple,n,string);
    }

    return tuple;
}


// Accessor for the `namedtuple` types to get the fields as a python
// `tuple`.
// return: The fields packed into a `tuple` or NULL on failure.
static PyObject *namedtuple_meta_getfields(PyObject *self,void *_){
    return tuple_from_fieldnames(((namedtuple*) self)->nt_fieldv,
                                 ((namedtuple*) self)->nt_fieldc);
}


// Allocate space for and copy the `nt_fieldv` and `nt_fieldc` from the super.
// return: The new `namedtuple` type or NULL on failure.
PyObject *namedtuple_meta_new(PyTypeObject *mcls,
                              PyObject *args,
                              PyObject *kwargs){
    const char * const argnames[] = {"name","bases","dict_",NULL};
    namedtuple *self;
    PyObject   *base;
    PyObject   *_;
    Py_ssize_t  fieldc;
    Py_ssize_t  n;
    static count = 0;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwargs,
                                     "OOO:NamedTupleMeta",
                                     (char**) argnames,
                                     &_,
                                     &base,
                                     &_)){
        return NULL;
    }

    if (!(base = PyTuple_GetItem(base,0))){
        PyErr_SetString(
            PyExc_ValueError,
            "Cannot construct old style classes with NamedTupleMeta");
        return NULL;
    }

    self = (namedtuple*) PyType_Type.tp_new(mcls,args,kwargs);

    // Copy the repr pointer format down
    self->nt_reprfmt = ((namedtuple*) base)->nt_reprfmt;
    Py_INCREF(self->nt_reprfmt);

    fieldc = ((namedtuple*) base)->nt_fieldc;
    self->nt_fieldc = fieldc;

    self->nt_fieldv = PyMem_New(char*,fieldc);
    for (n = 0;n < fieldc;++n){
        self->nt_fieldv[n] = pymem_strdup(((namedtuple*) base)->nt_fieldv[n]);
    }

    PyType_Modified((PyTypeObject*) self);

    return (PyObject*) self;
}


// Cleans up the memory used by the namedtuple types.
static void namedtuple_meta_dealloc(PyObject *self){
    Py_ssize_t n;
    Py_ssize_t fieldc = ((namedtuple*) self)->nt_fieldc;
    char     **fieldv = ((namedtuple*) self)->nt_fieldv;

    Py_XDECREF(((namedtuple*) self)->nt_reprfmt);

    for (n = 0;n < fieldc;++n){
        // Free all of the fieldnames.
        PyMem_Del(fieldv[n]);
    }

    // Free the array of fieldnames.
    PyMem_Del(((namedtuple*) self)->nt_fieldv);
    PyObject_GC_Del(self);
}


// Custom `__new__` for the namedtuple that reflects the argument information
// off of the type.
// return: The newly constructed namedtuple instance or NULL on failure.
static PyObject *namedtuple_new(PyTypeObject *cls,
                                PyObject *args,
                                PyObject *kwargs){
    PyObject  *self;
    char     **fieldv;
    Py_ssize_t fieldc;
    char      *keyword;
    Py_ssize_t n;
    Py_ssize_t pos;
    Py_ssize_t nargs;
    Py_ssize_t nkwargs;
    PyObject  *current_arg;
    int        match;
    PyObject  *key;
    PyObject  *value;

    nargs = PyTuple_GET_SIZE(args);
    nkwargs = (kwargs) ? PyDict_Size(kwargs) : 0;
    if (nargs + nkwargs > BUFSIZ){
        PyErr_Format(PyExc_TypeError,
                     "Cannot pass more than %d arguments to %s",
                     BUFSIZ,
                     ((PyTypeObject*) cls)->tp_name);
        return NULL;
    }


    fieldc = ((namedtuple*) cls)->nt_fieldc;
    fieldv = ((namedtuple*) cls)->nt_fieldv;

    if (!(self = ((PyTypeObject*) cls)->tp_alloc((PyTypeObject*) cls,fieldc))){
        return NULL;
    }

    // Custom argument parsing because of dynamic construction.
    if (nargs + nkwargs > fieldc) {
        PyErr_Format(PyExc_TypeError,
                     "%s takes at most %zd argument%s (%zd given)",
                     ((PyTypeObject*) cls)->tp_name,
                     fieldc,
                     (fieldc == 1) ? "" : "s",
                     nargs + nkwargs);
        PyTuple_Type.tp_dealloc(self);
        return NULL;
    }

    for (n = 0;n < fieldc;n++){
        keyword = fieldv[n];
        current_arg = NULL;
        if (nkwargs){
            current_arg = PyDict_GetItemString(kwargs,keyword);
        }
        if (current_arg){
            --nkwargs;
            if (n < nargs){
                // Arg present in tuple and in dict.
                PyErr_Format(PyExc_TypeError,
                             "Argument given by name ('%s') and position (%zd)",
                             keyword,
                             n + 1);
                PyTuple_Type.tp_dealloc(self);
                return NULL;
            }
        }else if (nkwargs && PyErr_Occurred()){
            PyTuple_Type.tp_dealloc(self);
            return NULL;
        }else if (n < nargs){
            // This reference is stolen when we store it in self.
            Py_INCREF(current_arg);
            current_arg = PyTuple_GET_ITEM(args,n);
        }

        if (current_arg){
            Py_INCREF(current_arg);
            PyTuple_SET_ITEM((PyTupleObject*) self,n,current_arg);
            continue;
        }

        if (n < fieldc){
            PyErr_Format(PyExc_TypeError,
                         "Required argument '%s' (pos %zd) not found",
                         keyword,
                         n + 1);
            PyTuple_Type.tp_dealloc(self);
            return NULL;
        }
    }

    // Check for extra kwargs.
    if (nkwargs > 0) {
        while (PyDict_Next(kwargs,&pos,&key,&value)){
            if (!PyString_Check(key)){
                PyErr_SetString(PyExc_TypeError,
                                "keywords must be strings");
                PyTuple_Type.tp_dealloc(self);
                return NULL;
            }
            keyword = PyString_AsString(key);
            for (n = 0;n < fieldc;++n){
                if (!strcmp(keyword,fieldv[n])) {
                    match = 1;
                    break;
                }
            }
            if (!match){
                PyErr_Format(PyExc_TypeError,
                             "'%s' is an invalid keyword argument for this "
                             "function",
                             keyword);
                PyTuple_Type.tp_dealloc(self);
                return NULL;
            }
        }
    }

    return self;
}


// docstring for `_make` method of `namedtuple`s.
#define namedtuple_make_doc "Make a new instance from a sequence or iterable."


// Namedtuple class method for creating new instances from an iterable.
// return `PyObject*` representing the new instance, or NULL to signal an error.
static PyObject *namedtuple_make(PyObject *cls,
                                 PyObject *args,
                                 PyObject *kwargs){
    const char * const argnames[] = {"iterable",NULL};
    PyObject *iterable;
    PyObject *tupleargs;
    PyObject *ret;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwargs,
                                     "O:_make",
                                     (char**) argnames,
                                     &iterable)){
        return NULL;
    }

    if (!(tupleargs = PyTuple_Pack(1,iterable))){
        return NULL;
    }
    ret = PyTuple_Type.tp_new((PyTypeObject*) cls,tupleargs,NULL);
    Py_DECREF(tupleargs);
    return ret;
}


// docstring for `_replace` method of `namedtuple`s.
#define namedtuple_replace_doc \
    "Return a new instance, replacing specified fields with new values."

// return:  new instance of `type(self)` with the kwargs
// swapped out. On failure, returns NULL.
static PyObject *namedtuple_replace(PyObject *self,
                                    PyObject *args,
                                    PyObject *kwargs){
    PyObject  *items;
    PyObject  *item;
    PyObject  *ret;
    PyObject  *tstr;
    PyObject  *tkeys;
    PyObject  *ttuple;
    PyObject  *errstr;
    Py_ssize_t n;
    Py_ssize_t fieldc = ((namedtuple*) self->ob_type)->nt_fieldc;
    char     **fieldv = ((namedtuple*) self->ob_type)->nt_fieldv;


    if (PyTuple_GET_SIZE(args)){
        // No positional arguments allowed.
        PyErr_Format(PyExc_TypeError,
                     "_replace takes no arguments (%zd given)",
                     PyTuple_GET_SIZE(args));
        return NULL;
    }

    if (!kwargs){
        // Fast path if nothing needs to be replaced, just copy.
        return PyObject_CallMethod(self,"_make","(N)",self);
    }

    if (!(items = PyTuple_New(fieldc))){
        return NULL;
    }

    for (n = 0;n < fieldc;++n){
        if (!(item = PyDict_GetItemString(kwargs,fieldv[n]))){
            item = PyTuple_GET_ITEM(self,n);
        }else{
            PyDict_DelItemString(kwargs,fieldv[n]);
        }

        Py_INCREF(item);
        PyTuple_SET_ITEM(items,n,item);
    }

    if (PyDict_Size(kwargs)){
        tstr = PyString_FromString("Got unexpected field names: %r");
        tkeys = PyDict_Keys(kwargs);
        ttuple = PyTuple_Pack(1,tkeys);
        Py_DECREF(tstr);
        Py_DECREF(tkeys);
        errstr = PyString_Format(tstr,ttuple);
        Py_DECREF(ttuple);
        PyErr_SetObject(PyExc_ValueError,errstr);
        return NULL;
    }

    ret = PyObject_CallMethod(self,"_make","(O)",items);
    Py_DECREF(items);
    return ret;
}


// The accessor function for lookup up a field by name on a named
// tuple. The `closure` is the index of the name cast to a `void*`.
static PyObject *namedtuple_getname(PyObject *self,void *idx){
    return PyTuple_GET_ITEM(self,(Py_ssize_t) idx);
}


// The accessor function to lookup the fields on the type as a python tuple.
// The `closure` is ignored.
static PyObject *namedtuple_getfields(PyObject *self,void *_){
    return tuple_from_fieldnames(((namedtuple*) self->ob_type)->nt_fieldv,
                                 ((namedtuple*) self->ob_type)->nt_fieldc);
}




// Constructor for asdict. Set this with `_register_asdict` function.
PyObject *closed_asdict = NULL;


#define namedtuple_asdict_doc \
    "Return a new OrderedDict which maps field names to their values."


// Converts self into a dict.
// return: A new dict or NULL in case of error.
PyObject *namedtuple_asdict(PyObject *self,PyObject *_){
    Py_ssize_t  n;
    namedtuple *tp     = (namedtuple*) self->ob_type;
    Py_ssize_t  fieldc = tp->nt_fieldc;
    char      **fieldv = tp->nt_fieldv;
    PyObject   *targ;
    PyObject   *temp;

    if (!(targ = PyTuple_New(fieldc))){
        return NULL;
    }

    for (n = 0;n < fieldc;++n){
        if (!(temp = Py_BuildValue("(sO)",fieldv[n],PyTuple_GET_ITEM(self,n)))){
            Py_DECREF(targ);
            return NULL;
        }

        PyTuple_SET_ITEM(targ,n,temp);
    }

    temp = PyObject_CallFunctionObjArgs(closed_asdict,targ,NULL);
    Py_DECREF(targ);
    return temp;
}


// docstring for `__getnewargs__` method of `namedtuple` objects.
#define namedtuple_getnewargs_doc \
    "Return self as a plain tuple. Used by copy and pickle."


// Pickle and copy protocol.
// return: self as a plain tuple or NULL in case of error.
PyObject *namedtuple_getnewargs(PyObject *self,PyObject *_){
    return PyObject_CallFunctionObjArgs((PyObject*) &PyTuple_Type,self,NULL);
}


// docstring for `__getstate__` method of `namedtuple` objects.
#define namedtuple_getstate_doc \
    "Exclude the OrderedDict from pickling."


// Pass function for the pickle and copy protocol.
// return: Py_None.
PyObject *namedtuple_getstate(PyObject *self,PyObject *_){
    Py_INCREF(Py_None);
    return Py_None;
}


// docstring for the `__reduce_ex__` method of `namedtuple` objects.
#define namedtuple_reduce_ex_doc "Pickle protocol."


// Pickle protocol for extension types.
// return: A tuple `(type(self),tuple(self))` or NULL in case of an error.
PyObject *namedtuple_reduce_ex(PyObject *self,PyObject *_){
    PyObject *astuple = PyObject_CallFunctionObjArgs((PyObject*) &PyTuple_Type,
                                                     self,
                                                     NULL);

    if (!astuple){
        return NULL;
    }

    return PyTuple_Pack(2,self->ob_type,astuple);
}


// The `__repr__` for `namedtuple` objects.
// return: A str in the format `{typename}({f_1}={v_1}, ..., {f_n}={v_n})`
// or NULL in case of an exception.
PyObject *namedtuple_repr(PyObject *self,PyObject *_){
    return PyString_Format(((namedtuple*) self->ob_type)->nt_reprfmt,self);
}


// Populates the `nt_fieldv` of `cls`.
// return: zero on success, nonzero on failure.
static int namedtuple_factory_populate_str(namedtuple *cls,
                                           PyObject *field_names){
    Py_ssize_t len;
    Py_ssize_t fieldc = 0;
    char      *field_names_asstring;
    char      *field;
    char      *ptr;
    char      *field_ptrs[BUFSIZ];

    len = PyString_GET_SIZE(field_names);
    field_names_asstring = field = strndup(PyString_AS_STRING(field_names),len);
    while(field){
        ptr = strsep(&field,", ");
        if (!*ptr){
            continue;
        }

        field_ptrs[fieldc++] = pymem_strdup(ptr);

        if (fieldc == BUFSIZ){
            while (fieldc--){
                PyMem_Del(field_ptrs[fieldc]);
            }
            free(field_names_asstring);
            PyErr_Format(PyExc_ValueError,
                         "Cannot create a namedtuple with more than %d fields",
                         BUFSIZ);
            return 1;
        }
    }
    free(field_names_asstring);

    cls->nt_fieldv = PyMem_New(char*,fieldc);
    memcpy(cls->nt_fieldv,field_ptrs,fieldc * sizeof(char*));
    cls->nt_fieldc = fieldc;

    return 0;
}


// Build the `nt_fieldv` of `cls` out of a sequence.
// return: zero on success, nonzero on failure.
static int namedtuple_factory_populate_seq(namedtuple *cls,
                                           PyObject *field_names){
    Py_ssize_t fieldc;
    Py_ssize_t n;
    char     **fieldv;
    PyObject **fastitems;
    PyObject  *asstr;
    PyObject  *items;

    if (!(items = PySequence_Fast(field_names,
                                  "field_names must be a sequence"))){
        return 1;
    }

    fieldc = PySequence_Fast_GET_SIZE(items);
    cls->nt_fieldv = fieldv = PyMem_New(char*,fieldc);
    cls->nt_fieldc = fieldc;

    fastitems = PySequence_Fast_ITEMS(items);
    for (n = 0;n < fieldc;n++){
        if (!(asstr = PyObject_Str(fastitems[n]))){
            while (n--){
                PyMem_Del(fieldv[n]);
            }
            PyMem_Del(fieldv);
            return 1;
        }

        fieldv[n] = pymem_strdup(PyString_AS_STRING(asstr));
        Py_DECREF(asstr);
    }
    return 0;
}


#define py_keywordc 31
const char * const py_keywordv[py_keywordc] = {
    "and",
    "as",
    "assert",
    "break",
    "class",
    "continue",
    "def",
    "del",
    "elif",
    "else",
    "except",
    "exec",
    "finally",
    "for",
    "from",
    "global",
    "if",
    "import",
    "in",
    "is",
    "lambda",
    "not",
    "or",
    "pass",
    "print",
    "raise",
    "return",
    "try",
    "while",
    "with",
    "yield",
};


// A type to indicate the results of `namedtuple_factory_checkfield`
typedef enum{
    CHECKFIELD_VALID      = 0,
    CHECKFIELD_EMPTY      = 1,
    CHECKFIELD_DIGIT      = 2,
    CHECKFIELD_UNDERSCORE = 3,
    CHECKFIELD_NONALNUM   = 4,
    CHECKFIELD_KEYWORD    = 5,
}nt_checkfield;


// Checks a single field.
// return: The issue with the field.
static nt_checkfield namedtuple_factory_checkfield(char *field){
    char  *c;
    size_t n;

    if (!*field){
        // Empty name.
        return CHECKFIELD_EMPTY;
    }

    if (isdigit(*field)){
        // Cannot start with digit.
        return CHECKFIELD_DIGIT;
    }else if (*field == '_'){
        // Cannot start with '_'.
        return CHECKFIELD_UNDERSCORE;
    }

    for (c = field;*c;++c){
        if (!(isalnum(*c) || *c == '_')){
            // Must be all alphanumeric characters or underscores.
            return CHECKFIELD_NONALNUM;
        }
    }

    for (n = 0;n < py_keywordc;++n){
        if (!strcmp(field,py_keywordv[n])){
            // Cannot be a keyword.
            return CHECKFIELD_KEYWORD;
        }
    }

    return CHECKFIELD_VALID;
}


// Apply the renames to the `field_names` for `cls`.
// return: zero on success, non-zero on failure.
static int namedtuple_factory_rename(namedtuple *cls){
    PyObject    *seen;
    PyObject    *asstr;
    char       **fieldv;
    char        *field;
    Py_ssize_t   fieldc;
    Py_ssize_t   n;
    bool         rename;

    if (!(seen = PySet_New(NULL))){
        return 1;
    }

    fieldv = cls->nt_fieldv;
    fieldc = cls->nt_fieldc;

    // Iterate over the fields, applying any renames if needed.
    for (n = 0;n < fieldc;++n){
        rename = false;
        field = fieldv[n];
        asstr = NULL;

        if (namedtuple_factory_checkfield(field) != CHECKFIELD_VALID){
            // Invalid name for some reason.
            rename = true;
        }else{
            if (!(asstr = PyString_FromString(field))){
                Py_DECREF(seen);
                return 1;
            }

            // Check if the name is in the set of seen names.
            switch(PySet_Contains(seen,asstr)){
            case 1:
                rename = true;
                break;
            case -1:
                Py_DECREF(asstr);
                Py_DECREF(seen);
                return 1;
            }
        }

        if (rename){
            Py_XDECREF(asstr);
            PyMem_Del(field);

            if (!(asstr = PyString_FromFormat("_%zu",n))){
                Py_DECREF(seen);
                return 1;
            }
            fieldv[n] = pymem_strdup(PyString_AS_STRING(asstr));
        }

        // Add the name to the set of seen names.
        if (PySet_Add(seen,asstr)){
            return NULL;
        }
        Py_DECREF(asstr);
    }

    Py_DECREF(seen);  // This will decref all the strings it holds.
    return 0;
}


// Validates the `typename` and field names.
// return: zero on success, nonzero on failure.
static int namedtuple_factory_validate(PyObject *typename,
                                       char **fieldv,
                                       Py_ssize_t fieldc,
                                       bool rename){
    Py_ssize_t n;
    PyObject  *seen;
    PyObject  *asstr;
    char      *field;
    const char * const nonalnum_fmt =
        "Type names and field names can only contain alphanumeric characters "
        "and underscores: %s";
    const char * const keyword_fmt =
        "Type names and field names cannot be a keyword: %s";
    const char * const digit_fmt = "Type names and field names cannot start "
        "with a number: %s";
    const char * const underscore_fmt =
        "Field names cannot start with an underscore: %s";
    const char * const empty_fmt = "Type names and field names cannot be empty";
    const char * const seen_fmt = "Encountered duplicate field name: %s";

    switch(namedtuple_factory_checkfield(PyString_AS_STRING(typename))){
    case CHECKFIELD_UNDERSCORE:
    case CHECKFIELD_VALID:
        break;
    case CHECKFIELD_NONALNUM:
        PyErr_Format(PyExc_ValueError,
                     nonalnum_fmt,
                     PyString_AS_STRING(typename));
        return 1;
    case CHECKFIELD_KEYWORD:
        PyErr_Format(PyExc_ValueError,
                     keyword_fmt,
                     PyString_AS_STRING(typename));
        return 1;
    case CHECKFIELD_DIGIT:
        PyErr_Format(PyExc_ValueError,
                     digit_fmt,
                     PyString_AS_STRING(typename));
        return 1;
    case CHECKFIELD_EMPTY:
        PyErr_SetString(PyExc_ValueError,empty_fmt);
        return 1;
    }

    if (!(seen = PySet_New(NULL))){
        return 1;
    }

    for (n = 0;n < fieldc;++n){
        field = fieldv[n];

        switch(namedtuple_factory_checkfield(field)){
        case CHECKFIELD_VALID:
            break;
        case CHECKFIELD_UNDERSCORE:
            if (!rename){
                PyErr_Format(PyExc_ValueError,
                             underscore_fmt,
                             field);
                return 1;
            }

            break;
        case CHECKFIELD_NONALNUM:
            PyErr_Format(PyExc_ValueError,
                         nonalnum_fmt,
                         field);
            Py_DECREF(seen);
            return 1;
        case CHECKFIELD_KEYWORD:
            PyErr_Format(PyExc_ValueError,
                         keyword_fmt,
                         field);
            Py_DECREF(seen);
            return 1;
        case CHECKFIELD_DIGIT:
            PyErr_Format(PyExc_ValueError,
                         digit_fmt,
                         field);
            Py_DECREF(seen);
            return 1;
        case CHECKFIELD_EMPTY:
            PyErr_SetString(PyExc_ValueError,empty_fmt);
            return 1;
        }

        if (!(asstr = PyString_FromString(field))){
            Py_DECREF(seen);
            return 1;
        };

        switch(PySet_Contains(seen,asstr)){
        case 1:
            PyErr_Format(PyExc_ValueError,
                         seen_fmt,
                         field);
            Py_DECREF(asstr);
            Py_DECREF(seen);
            return 1;
        case -1:
            Py_DECREF(asstr);
            Py_DECREF(seen);
            return 1;
        }

        if (PySet_Add(seen,asstr)){
            Py_DECREF(seen);
            return 1;
        }

        // Adding it to the set will incref it to keep it alive.
        Py_DECREF(asstr);
    }

    // Will traverse the set to del all the asstr's it is holding.
    Py_DECREF(seen);
    return 0;
}


// Cache the repr format string.
// return: zero on success, nonzero on failure.
int namedtuple_factory_cache_repr_fmt(namedtuple *nt,PyObject *typename){
    Py_ssize_t n;
    Py_ssize_t fieldc = nt->nt_fieldc;
    char     **fieldv = nt->nt_fieldv;
    PyObject  *buf;
    PyObject  *tmp;

    if (!(buf = PyString_FromFormat("%s(",PyString_AS_STRING(typename)))){
        return 1;
    }

    for (n = 0;n < fieldc;++n){
        if (!(tmp = PyString_FromFormat("%s=%%r%s",
                                        fieldv[n],
                                        (n == fieldc - 1) ? ")" : ", "))){
            Py_DECREF(buf);
            return 1;
        }
        PyString_ConcatAndDel(&buf,tmp);
    }

    nt->nt_reprfmt = buf;
    return 0;
}


// doctstring for `namedtuple` instances.
#define namedtuple_doc "A subclass of tuple with named attribute lookups."


PyMethodDef namedtuple_methods[] = {
    {.ml_name="_make",
     .ml_meth=namedtuple_make,
     .ml_flags=METH_CLASS | METH_KEYWORDS,
     .ml_doc=namedtuple_make_doc},
    {.ml_name="_replace",
     .ml_meth=namedtuple_replace,
     .ml_flags=METH_KEYWORDS,
     .ml_doc=namedtuple_replace_doc},
    {.ml_name="_asdict",
     .ml_meth=namedtuple_asdict,
     .ml_flags=METH_NOARGS,
     .ml_doc=namedtuple_asdict_doc},
    {.ml_name="__getnewargs__",
     .ml_meth=namedtuple_getnewargs,
     .ml_flags=METH_NOARGS,
     .ml_doc=namedtuple_getnewargs_doc},
    {.ml_name="__getstate__",
     .ml_meth=namedtuple_getstate,
     .ml_flags=METH_NOARGS,
     .ml_doc=namedtuple_getstate_doc},
    {.ml_name="__reduce_ex__",
     .ml_meth=namedtuple_reduce_ex,
     .ml_flags=METH_O,
     .ml_doc=namedtuple_reduce_ex_doc},
    {NULL},
};


// docstring for `namedtuple` function.
#define namedtuple_factory_doc \
    "Returns a new subclass of tuple with named fields."


// namedtuple factory function.
// return: A new namedtuple type or NULL on failure.
static PyObject *namedtuple_factory(PyObject *self,
                                    PyObject *args,
                                    PyObject *kwargs){
    const char *const argnames[] =
        {"typename","field_names","verbose","rename",NULL};
    char            **fieldv;
    Py_ssize_t        fieldc;
    Py_ssize_t        n;
    PyObject         *typename    = NULL;
    PyObject         *field_names = NULL;
    PyObject         *rename      = NULL;
    PyObject         *verbose;  // ignored
    bool              rename_as_bool;
    namedtuple       *newtype;
    PyTypeObject     *astype;
    PyHeapTypeObject *asheap;
    PyGetSetDef      *properties;
    PyFrameObject    *frame;
    PyObject         *module_name;

    if (!PyArg_ParseTupleAndKeywords(args,
                                     kwargs,
                                     "OO|OO:namedtuple",
                                     (char**) argnames,
                                     &typename,
                                     &field_names,
                                     &verbose,  // ignored
                                     &rename)){
        return NULL;
    }

    if (!(typename = PyObject_Str(typename))){
        // Typename cannot be converted to `str`.
        return NULL;
    }

    newtype = PyObject_GC_NewVar(namedtuple,&namedtuple_meta,0);
    newtype->nt_reprfmt = NULL;

    // Dispatch the `namedtuple_populate_fieldnames_*` function based
    // on the type of `field_names`.
    if (((PyObject_IsInstance(field_names,(PyObject*) &PyString_Type)) ?
         namedtuple_factory_populate_str
         : namedtuple_factory_populate_seq)(newtype,field_names)){

        Py_DECREF(typename);
        namedtuple_meta_dealloc((PyObject*) newtype);
        return NULL;
    }

    rename_as_bool = rename && PyObject_IsTrue(rename);
    if (rename_as_bool){
        if (namedtuple_factory_rename(newtype)){
            Py_DECREF(typename);
            namedtuple_meta_dealloc((PyObject*) newtype);
            return NULL;
        }
    }

    if (namedtuple_factory_validate(typename,
                                    newtype->nt_fieldv,
                                    newtype->nt_fieldc,
                                    rename_as_bool)){
            Py_DECREF(typename);
            namedtuple_meta_dealloc((PyObject*) newtype);
            return NULL;
    }

    // Cache the repr format string.
    if (namedtuple_factory_cache_repr_fmt(newtype,typename)){
        namedtuple_meta_dealloc((PyObject*) newtype);
        return NULL;
    }

    // Clear the type field.
    memset(&newtype->nt_tp,0,sizeof(PyHeapTypeObject));
    // Give us back the reference that was zeroed out above.
    ((PyObject*) newtype)->ob_refcnt = 1;

    asheap = (PyHeapTypeObject*) newtype;
    // Set the heaptype attributes.
    Py_INCREF(typename);
    asheap->ht_name       = typename;
    asheap->ht_slots      = NULL;

    astype = (PyTypeObject*) newtype;
    // Set the type attributes.
    astype->ob_type       = &namedtuple_meta;  // instance of `namedtuple_meta`
    astype->tp_dictoffset = PyTuple_Type.tp_dictoffset;
    astype->tp_methods    = namedtuple_methods;
    astype->tp_base       = &PyTuple_Type;  // subclass of `tuple`.
    astype->tp_new        = namedtuple_new;
    astype->tp_dealloc    = PyTuple_Type.tp_dealloc;
    astype->tp_traverse   = PyTuple_Type.tp_traverse;
    astype->tp_name       = PyString_AS_STRING(typename);
    astype->tp_repr       = namedtuple_repr;
    astype->tp_doc        = namedtuple_doc;
    astype->tp_itemsize   = PyTuple_Type.tp_itemsize;
    astype->tp_basicsize  = PyTuple_Type.tp_basicsize;
    astype->tp_flags      = (Py_TPFLAGS_DEFAULT
                             | Py_TPFLAGS_BASETYPE
                             | Py_TPFLAGS_HEAPTYPE
                             | Py_TPFLAGS_HAVE_GC);

    // Cache on the stack.
    fieldc = newtype->nt_fieldc;
    fieldv = newtype->nt_fieldv;

    // fieldc + 2 `PyGetSetDef`s because of the `_fields` property and the
    // NULL terminator.
    properties = PyMem_New(PyGetSetDef,fieldc + 2);

    // Iterate through the fieldnames to create the accessors.
    for (n = 0;n < fieldc;++n){
        properties[n].name    = fieldv[n];
        properties[n].get     = namedtuple_getname;
        properties[n].set     = NULL;  // Immutable.
        // Close over the index for this name to make lookups warp speed.
        properties[n].closure = (void*) n;
    }
    properties[n].name    = "_fields";
    properties[n].get     = namedtuple_getfields;
    properties[n].set     = NULL;
    properties[n].closure = NULL;
    // Zero the last `PyGetSetDef` slot to mark the end of the properties.
    memset(&properties[++n],0,sizeof(PyGetSetDef));
    astype->tp_getset = properties;

    if (PyType_Ready(astype)){
        namedtuple_meta_dealloc((PyObject*) newtype);
        return NULL;
    }

    if ((frame = PyEval_GetFrame())){
        if ((module_name = PyDict_GetItemString(frame->f_globals,"__name__"))){
            PyObject_SetAttrString((PyObject*) newtype,
                                   "__module__",
                                   module_name);
        }
    }

    return (PyObject*) newtype;
}


#define register_asdict_doc "Register the constructor for the asdict method."


// Register the dict constructor with the module.
// return: None on success, NULL on failure.
PyObject *register_asdict(PyObject *self,PyObject *asdict){
    if (!PyCallable_Check(asdict)){
        PyErr_SetString(PyExc_ValueError,"asdict must be a callable object");
        return NULL;
    }

    Py_XDECREF(closed_asdict);
    Py_INCREF(asdict);
    closed_asdict = asdict;  // store this for the namedtuple objects.

    Py_INCREF(Py_None);
    return Py_None;
}


// The module level methods for the `namedtuple` module.
PyMethodDef methods[] = {
    {.ml_name="namedtuple",
     .ml_meth=namedtuple_factory,
     .ml_flags=METH_KEYWORDS,
     .ml_doc=namedtuple_factory_doc},
    {.ml_name="_register_asdict",
     .ml_meth=register_asdict,
     .ml_flags=METH_O,
     .ml_doc=register_asdict_doc},
    {NULL},
};


PyMODINIT_FUNC initnamedtuple(){
    PyObject *none;

    if (PyType_Ready(&namedtuple_meta)){
        return;
    }

    Py_InitModule3("namedtuple",
                   methods,
                   "Fast implementation of namedtuple written in C");

    // Default to an asdict of `dict` instead of `OrderedDict`.
    if ((none = register_asdict(Py_None,&PyDict_Type))){
        Py_DECREF(none);
    }
}
