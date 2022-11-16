#include "include/lvtinydom.h"

/// returns position pointer
ldomXPointer LvTocItem::getXPointer()
{
    if (_position.isNull() && !_path.empty()) {
        _position = _doc->createXPointer(_path);
        if (_position.isNull()) {
            CRLog::trace("TOC node is not found for path %s", LCSTR(_path));
        } else {
            CRLog::trace("TOC node is found for path %s", LCSTR(_path));
        }
    }
    return _position;
}

/// returns position path
lString16 LvTocItem::getPath()
{
    if (_path.empty() && !_position.isNull()) {
        _path = _position.toString();
    }
    return _path;
}

/// returns Y position
int LvTocItem::getY()
{
    return getXPointer().toPoint().y;
}
