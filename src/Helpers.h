#ifndef Helpers_h_
#define Helpers_h_

#include <QUrl>
#include "UrlItem.h"

class QString;
class QGraphicsWidget;

void notification(const QString& text, QGraphicsWidget* parent);
QUrl urlFromUserInput(const QString& string);
void internalizeUrlList(UrlList& list, const QString& fileName);
void externalizeUrlList(const UrlList& list, const QString& fileName);

#endif
