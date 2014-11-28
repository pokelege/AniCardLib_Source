/****************************************************************************
** Meta object code from reading C++ file 'Editor.h'
**
** Created: Fri Nov 28 11:58:19 2014
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "Editor.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Editor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Editor[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       8,    7,    7,    7, 0x08,
      30,   21,    7,    7, 0x08,
      46,   21,    7,    7, 0x08,
      63,    7,    7,    7, 0x08,
      73,    7,    7,    7, 0x08,
      85,    7,    7,    7, 0x08,
      96,    7,    7,    7, 0x08,
     108,    7,    7,    7, 0x08,
     121,    7,    7,    7, 0x08,
     135,    7,    7,    7, 0x08,
     142,    7,    7,    7, 0x08,
     155,  149,    7,    7, 0x08,
     178,    7,    7,    7, 0x08,
     194,    7,    7,    7, 0x08,
     213,    7,    7,    7, 0x08,
     233,    7,    7,    7, 0x08,
     258,  251,    7,    7, 0x08,
     279,  251,    7,    7, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_Editor[] = {
    "Editor\0\0initialize()\0selected\0"
    "selectCard(int)\0selectModel(int)\0"
    "addCard()\0swapCards()\0addModel()\0"
    "linkModel()\0addTexture()\0linkTexture()\0"
    "save()\0load()\0index\0frameIndexChanged(int)\0"
    "addFrameRange()\0changeParamsNext()\0"
    "changeParamsStart()\0changeParamsEnd()\0"
    "isTrue\0changeCardMode(bool)\0"
    "changeModelMode(bool)\0"
};

void Editor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Editor *_t = static_cast<Editor *>(_o);
        switch (_id) {
        case 0: _t->initialize(); break;
        case 1: _t->selectCard((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->selectModel((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->addCard(); break;
        case 4: _t->swapCards(); break;
        case 5: _t->addModel(); break;
        case 6: _t->linkModel(); break;
        case 7: _t->addTexture(); break;
        case 8: _t->linkTexture(); break;
        case 9: _t->save(); break;
        case 10: _t->load(); break;
        case 11: _t->frameIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 12: _t->addFrameRange(); break;
        case 13: _t->changeParamsNext(); break;
        case 14: _t->changeParamsStart(); break;
        case 15: _t->changeParamsEnd(); break;
        case 16: _t->changeCardMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 17: _t->changeModelMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData Editor::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Editor::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Editor,
      qt_meta_data_Editor, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Editor::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Editor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Editor::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Editor))
        return static_cast<void*>(const_cast< Editor*>(this));
    return QWidget::qt_metacast(_clname);
}

int Editor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
