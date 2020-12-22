/****************************************************************************
** Meta object code from reading C++ file 'uidemo01.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../uidemo01.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'uidemo01.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_UIDemo01_t {
    QByteArrayData data[16];
    char stringdata0[265];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_UIDemo01_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_UIDemo01_t qt_meta_stringdata_UIDemo01 = {
    {
QT_MOC_LITERAL(0, 0, 8), // "UIDemo01"
QT_MOC_LITERAL(1, 9, 11), // "setDisLight"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 13), // "setDisContrst"
QT_MOC_LITERAL(4, 36, 8), // "initForm"
QT_MOC_LITERAL(5, 45, 11), // "buttonClick"
QT_MOC_LITERAL(6, 57, 17), // "slot_checkMessage"
QT_MOC_LITERAL(7, 75, 11), // "SERIAL_DATA"
QT_MOC_LITERAL(8, 87, 3), // "msg"
QT_MOC_LITERAL(9, 91, 22), // "on_btnMenu_Min_clicked"
QT_MOC_LITERAL(10, 114, 22), // "on_btnMenu_Max_clicked"
QT_MOC_LITERAL(11, 137, 24), // "on_btnMenu_Close_clicked"
QT_MOC_LITERAL(12, 162, 32), // "on_pushButtonDisContrase_clicked"
QT_MOC_LITERAL(13, 195, 29), // "on_pushButtonDisLight_clicked"
QT_MOC_LITERAL(14, 225, 19), // "slot_RefFT2000HiBit"
QT_MOC_LITERAL(15, 245, 19) // "slot_RefFT2000HiSys"

    },
    "UIDemo01\0setDisLight\0\0setDisContrst\0"
    "initForm\0buttonClick\0slot_checkMessage\0"
    "SERIAL_DATA\0msg\0on_btnMenu_Min_clicked\0"
    "on_btnMenu_Max_clicked\0on_btnMenu_Close_clicked\0"
    "on_pushButtonDisContrase_clicked\0"
    "on_pushButtonDisLight_clicked\0"
    "slot_RefFT2000HiBit\0slot_RefFT2000HiSys"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_UIDemo01[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,
       3,    1,   77,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   80,    2, 0x08 /* Private */,
       5,    0,   81,    2, 0x08 /* Private */,
       6,    1,   82,    2, 0x08 /* Private */,
       9,    0,   85,    2, 0x08 /* Private */,
      10,    0,   86,    2, 0x08 /* Private */,
      11,    0,   87,    2, 0x08 /* Private */,
      12,    0,   88,    2, 0x08 /* Private */,
      13,    0,   89,    2, 0x08 /* Private */,
      14,    0,   90,    2, 0x08 /* Private */,
      15,    0,   91,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7,    8,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void UIDemo01::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        UIDemo01 *_t = static_cast<UIDemo01 *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->setDisLight((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->setDisContrst((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->initForm(); break;
        case 3: _t->buttonClick(); break;
        case 4: _t->slot_checkMessage((*reinterpret_cast< SERIAL_DATA(*)>(_a[1]))); break;
        case 5: _t->on_btnMenu_Min_clicked(); break;
        case 6: _t->on_btnMenu_Max_clicked(); break;
        case 7: _t->on_btnMenu_Close_clicked(); break;
        case 8: _t->on_pushButtonDisContrase_clicked(); break;
        case 9: _t->on_pushButtonDisLight_clicked(); break;
        case 10: _t->slot_RefFT2000HiBit(); break;
        case 11: _t->slot_RefFT2000HiSys(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (UIDemo01::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&UIDemo01::setDisLight)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (UIDemo01::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&UIDemo01::setDisContrst)) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject UIDemo01::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_UIDemo01.data,
      qt_meta_data_UIDemo01,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *UIDemo01::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *UIDemo01::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_UIDemo01.stringdata0))
        return static_cast<void*>(const_cast< UIDemo01*>(this));
    return QDialog::qt_metacast(_clname);
}

int UIDemo01::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void UIDemo01::setDisLight(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void UIDemo01::setDisContrst(int _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
