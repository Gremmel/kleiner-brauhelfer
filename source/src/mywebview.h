#ifndef MYWEBVIEW_H
#define MYWEBVIEW_H

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))

// Since Qt5.5 use WebEngine

#include <QWebEngineView>
#include <QPrinter>

class MyWebView : public QWebEngineView
{
    Q_OBJECT

public:
    MyWebView(QWidget* parent = Q_NULLPTR);
    void setTextSizeMultiplier(qreal factor);

public slots:
    void print(QPrinter* printer);

private slots:
    void slotHandlePagePrinted(bool result);

private:
    QPrinter* currentPrinter;
};

#else

// Before Qt5.5 use WebKit

#include <QWebView>
#include <QPrinter>

class MyWebView : public QWebView
{
    Q_OBJECT

public:
    MyWebView(QWidget* parent = 0);

public slots:
    void print(QPrinter* printer);

private slots:
    void slot_urlClicked(const QUrl &url);
};

#endif

#endif // MYWEBVIEW_H
