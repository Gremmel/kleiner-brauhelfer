#ifndef MYWEBVIEW_H
#define MYWEBVIEW_H

#include <QtGlobal>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))

// Since Qt5.5 use WebEngine

#include <QWebEngineView>
#include <QWebEnginePage>

class MyWebPage : public QWebEnginePage
{
    Q_OBJECT

public:
    MyWebPage(QObject* parent = Q_NULLPTR);
    bool acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool) Q_DECL_OVERRIDE;
};

class MyWebView : public QWebEngineView
{
    Q_OBJECT

public:
    MyWebView(QWidget* parent = Q_NULLPTR);
    ~MyWebView();
    void printToPdf(const QString& filePath);
};

#else

// Before Qt5.5 use WebKit

#include <QWebView>

class MyWebView : public QWebView
{
    Q_OBJECT

public:
    MyWebView(QWidget* parent = 0);
    void printToPdf(const QString& filePath);

private slots:
    void slot_urlClicked(const QUrl &url);
};

#endif

#endif // MYWEBVIEW_H
