/****************************************************************************
** Meta object code from reading C++ file 'ft2000bit.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../ft2000bit.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ft2000bit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_FT2000BIT_t {
    QByteArrayData data[3];
    char stringdata0[24];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FT2000BIT_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FT2000BIT_t qt_meta_stringdata_FT2000BIT = {
    {
QT_MOC_LITERAL(0, 0, 9), // "FT2000BIT"
QT_MOC_LITERAL(1, 10, 12), // "sig_FT200bit"
QT_MOC_LITERAL(2, 23, 0) // ""

    },
    "FT2000BIT\0sig_FT200bit\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FT2000BIT[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,

       0        // eod
};

void FT2000BIT::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        FT2000BIT *_t = static_cast<FT2000BIT *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sig_FT200bit(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (FT2000BIT::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&FT2000BIT::sig_FT200bit)) {
                *result = 0;
                return;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject FT2000BIT::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_FT2000BIT.data,
      qt_meta_data_FT2000BIT,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *FT2000BIT::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FT2000BIT::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_FT2000BIT.stringdata0))
        return static_cast<void*>(const_cast< FT2000BIT*>(this));
    return QThread::qt_metacast(_clname);
}

int FT2000BIT::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void FT2000BIT::sig_FT200bit()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
