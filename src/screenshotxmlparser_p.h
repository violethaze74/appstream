#ifndef SCREENSHOT_XML_PARSER_H
#include <qxmlstream.h>
#include <QUrl>

#include "image.h"
#include "screenshot.h"

namespace {
    inline Asmara::Image parseImageXml(QXmlStreamReader* reader, bool* failure) {
        int level = 0;
        Q_ASSERT(reader->name() == QLatin1String("image"));
        QString type;
        if(reader->attributes().hasAttribute(QLatin1String("type"))) {
            type = reader->attributes().value(QLatin1String("type")).toString();
        }
        int width = -1;
        if(reader->attributes().hasAttribute(QLatin1String("width"))) {
            QString widthString = reader->attributes().value(QLatin1String("width")).toString();
            bool convertSuccess;
            int possibleWidth = widthString.toInt(&convertSuccess);
            if(convertSuccess) {
                width = possibleWidth;
            }
        }
        int height = -1;
        if(reader->attributes().hasAttribute(QLatin1String("height"))) {
            QString heightString = reader->attributes().value(QLatin1String("height")).toString();
            bool convertSuccess;
            int possibleHeight = heightString.toInt(&convertSuccess);
            if(convertSuccess) {
                height = possibleHeight;
            }
        }
        QUrl url = QUrl::fromUserInput(reader->readElementText());

        Asmara::Image image;
        image.setUrl(url);
        image.setKind(Asmara::Image::stringToKind(type));
        image.setWidth(width);
        image.setHeight(height);
        return image;
    }

    inline Asmara::ScreenShot parseScreenShotXml(QXmlStreamReader* reader, bool* failure) {
        int level = 0;
        QList<Asmara::Image> images;
        bool default_ = reader->attributes().value(QLatin1String("type")) == QLatin1String("default");
        QString caption;
        while(!(*failure) && !reader->atEnd()) {
            QXmlStreamReader::TokenType token = reader->readNext();
            if(reader->name() == QLatin1String("screenshot") && reader->tokenType() == QXmlStreamReader::EndElement) {
                break;
            }
            if(token == QXmlStreamReader::StartElement) {
                if(reader->name() == QLatin1String("image")) {
                    images << parseImageXml(reader, failure);
                    Q_ASSERT(reader->tokenType() == QXmlStreamReader::EndElement);
                } else if(reader->name() == QLatin1String("caption")) {
                    caption = reader->readElementText();
                    level++;
                }
            } else if (token == QXmlStreamReader::EndElement) {
                if(reader->name() == QLatin1String("screenshot")) {
                    if(level !=0) {
                        *failure = true;
                    }
                }
                if(reader->name() == QLatin1String("caption")) {
                    level--;
                }
            }
        }
        if(level!=0) {
            *failure = true;
        }
        Asmara::ScreenShot screenShot;
        screenShot.setImages(images);
        screenShot.setDefault(default_);
        screenShot.setCaption(caption);
        return screenShot;
    }

    inline QList<Asmara::ScreenShot> parseScreenShotsXml(QXmlStreamReader* reader) {
        int level = 0;
        bool failure = false;
        QList<Asmara::ScreenShot> screenShots;
        while(!reader->atEnd() && !failure) {
            QXmlStreamReader::TokenType token = reader->readNext();
            if(token == QXmlStreamReader::StartDocument) {
                continue;
            }
            if(token == QXmlStreamReader::StartElement) {
                if(reader->name() == QLatin1String("screenshots")) {
                    level++;
                } else if(reader->name() == QLatin1String("screenshot")) {
                    screenShots << parseScreenShotXml(reader, &failure);
                    Q_ASSERT(reader->tokenType() == QXmlStreamReader::EndElement);
                }
            } else if (token == QXmlStreamReader::EndElement) {
                if(reader->name() == QLatin1String("screenshots")) {
                    level--;
                }
            }
        }
        if(failure || level != 0 || reader->hasError()) {
            return QList<Asmara::ScreenShot>();
        } else {
            return screenShots;
        }
    }
}

#endif // SCREENSHOT_XML_PARSER_H
