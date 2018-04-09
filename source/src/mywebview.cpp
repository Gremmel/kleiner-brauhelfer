#include "mywebview.h"
#include <QDesktopServices>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))

#include <QEventLoop>
#include <QMessageBox>
#include "definitionen.h"

MyWebPage::MyWebPage(QObject* parent) :
    QWebEnginePage(parent),
    mExternal(false)
{
}

void MyWebPage::setLinksExternal(bool external)
{
    mExternal = external;
}

bool MyWebPage::acceptNavigationRequest(const QUrl& url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    Q_UNUSED(isMainFrame)
    if (type == QWebEnginePage::NavigationTypeLinkClicked)
    {
        if (url.isLocalFile() || mExternal)
        {
            QDesktopServices::openUrl(url);
            return false;
        }
    }
    return true;
}

MyWebView::MyWebView(QWidget* parent) :
    QWebEngineView(parent)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    setPage(new MyWebPage());
}

MyWebView::~MyWebView()
{
    delete page();
}

void MyWebView::setLinksExternal(bool external)
{
    ((MyWebPage*)page())->setLinksExternal(external);
}

void MyWebView::printToPdf(const QString& filePath)
{
  #if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    QEventLoop loop;
    connect(page(), SIGNAL(pdfPrintingFinished(const QString&, bool)), &loop, SLOT(quit()));
    page()->printToPdf(filePath, QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, QMarginsF(20, 20, 20, 20)));
    loop.exec();
  #else
    Q_UNUSED(filePath)
    QMessageBox::critical(this, APP_NAME, tr("PDF Erstellung wird von dieser Qt Version nicht unterstüzt."));
  #endif
}

#else

#include <QPrinter>

MyWebView::MyWebView(QWidget* parent) :
    QWebView(parent),
    mExternal(false)
{
    setRenderHint(QPainter::TextAntialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setRenderHint(QPainter::HighQualityAntialiasing, true);

    // Links extern weiterleiten
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this, SIGNAL(linkClicked (const QUrl &)), this, SLOT(slot_urlClicked(const QUrl &)));
}

void MyWebView::setLinksExternal(bool external)
{
    mExternal = external;
}

void MyWebView::printToPdf(const QString& filePath)
{
    QPrinter* printer = new QPrinter(QPrinter::HighResolution);
    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setColorMode(QPrinter::Color);
    printer->setResolution(1200);
    printer->setOutputFileName(filePath);
    QWebView::print(printer);
    delete printer;
}

void MyWebView::slot_urlClicked(const QUrl &url)
{
    if (url.isLocalFile() || mExternal)
        QDesktopServices::openUrl(url);
    else
        setUrl(url);
}

#endif
