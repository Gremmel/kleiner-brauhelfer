#include "mywebview.h"
#include <QDesktopServices>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))

MyWebView::MyWebView(QWidget* parent) :
    QWebEngineView(parent)
{
}

void MyWebView::setTextSizeMultiplier(qreal factor)
{
    // Zoom Faktor wird bei printToPdf() nicht übernommen
    page()->setZoomFactor(factor);
}

void MyWebView::print(QPrinter * printer) const
{
    page()->printToPdf(printer->outputFileName());
}

#else

MyWebView::MyWebView(QWidget* parent) :
    QWebView(parent)
{
    setRenderHint(QPainter::TextAntialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setRenderHint(QPainter::HighQualityAntialiasing, true);

    // Links extern weiterleiten
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this, SIGNAL(linkClicked (const QUrl &)), this, SLOT(slot_urlClicked(const QUrl &)));
}

void MyWebView::slot_urlClicked(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

#endif
