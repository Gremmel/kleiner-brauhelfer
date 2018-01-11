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
    void setLinksExternal(bool external);
    bool acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool) Q_DECL_OVERRIDE;
private:
    bool mExternal;
};

class MyWebView : public QWebEngineView
{
    Q_OBJECT

public:
    MyWebView(QWidget* parent = Q_NULLPTR);
    ~MyWebView();
    void setLinksExternal(bool external);
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
    void setLinksExternal(bool external);
    void printToPdf(const QString& filePath);
private slots:
    void slot_urlClicked(const QUrl &url);
private:
    bool mExternal;
};

#endif

#endif // MYWEBVIEW_H
