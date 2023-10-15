/****************************************************************************
** Meta object code from reading C++ file 'MT_2_N52.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../MT_2_N52.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MT_2_N52.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MT_2_N52_t {
    QByteArrayData data[16];
    char stringdata0[206];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MT_2_N52_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MT_2_N52_t qt_meta_stringdata_MT_2_N52 = {
    {
QT_MOC_LITERAL(0, 0, 8), // "MT_2_N52"
QT_MOC_LITERAL(1, 9, 7), // "measure"
QT_MOC_LITERAL(2, 17, 0), // ""
QT_MOC_LITERAL(3, 18, 8), // "show_ROI"
QT_MOC_LITERAL(4, 27, 9), // "get_stack"
QT_MOC_LITERAL(5, 37, 9), // "Focus_box"
QT_MOC_LITERAL(6, 47, 5), // "value"
QT_MOC_LITERAL(7, 53, 18), // "MagnetPosition_box"
QT_MOC_LITERAL(8, 72, 1), // "F"
QT_MOC_LITERAL(9, 74, 10), // "Send_pulse"
QT_MOC_LITERAL(10, 85, 33), // "on_MagnetPositionBox_valueCha..."
QT_MOC_LITERAL(11, 119, 4), // "arg1"
QT_MOC_LITERAL(12, 124, 24), // "on_SliderMP_valueChanged"
QT_MOC_LITERAL(13, 149, 27), // "on_FocusSlider_valueChanged"
QT_MOC_LITERAL(14, 177, 24), // "on_FocusBox_valueChanged"
QT_MOC_LITERAL(15, 202, 3) // "arg"

    },
    "MT_2_N52\0measure\0\0show_ROI\0get_stack\0"
    "Focus_box\0value\0MagnetPosition_box\0F\0"
    "Send_pulse\0on_MagnetPositionBox_valueChanged\0"
    "arg1\0on_SliderMP_valueChanged\0"
    "on_FocusSlider_valueChanged\0"
    "on_FocusBox_valueChanged\0arg"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MT_2_N52[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x0a /* Public */,
       3,    0,   65,    2, 0x0a /* Public */,
       4,    0,   66,    2, 0x0a /* Public */,
       5,    1,   67,    2, 0x0a /* Public */,
       7,    1,   70,    2, 0x0a /* Public */,
       9,    0,   73,    2, 0x0a /* Public */,
      10,    1,   74,    2, 0x0a /* Public */,
      12,    1,   77,    2, 0x0a /* Public */,
      13,    1,   80,    2, 0x0a /* Public */,
      14,    1,   83,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Double,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   11,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::Double,   15,

       0        // eod
};

void MT_2_N52::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MT_2_N52 *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->measure(); break;
        case 1: _t->show_ROI(); break;
        case 2: _t->get_stack(); break;
        case 3: _t->Focus_box((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->MagnetPosition_box((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 5: _t->Send_pulse(); break;
        case 6: _t->on_MagnetPositionBox_valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->on_SliderMP_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->on_FocusSlider_valueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->on_FocusBox_valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MT_2_N52::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_MT_2_N52.data,
    qt_meta_data_MT_2_N52,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MT_2_N52::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MT_2_N52::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MT_2_N52.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MT_2_N52::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
