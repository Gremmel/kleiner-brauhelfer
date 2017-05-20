#include "mywebview.h"
#include <QDesktopServices>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 5, 0))

template<typename Arg, typename R, typename C>
struct InvokeWrapper {
    R *receiver;
    void (C::*memberFun)(Arg);
    void operator()(Arg result) {
        (receiver->*memberFun)(result);
    }
};

template<typename Arg, typename R, typename C>
InvokeWrapper<Arg, R, C> invoke(R *receiver, void (C::*memberFun)(Arg))
{
    InvokeWrapper<Arg, R, C> wrapper = {receiver, memberFun};
    return wrapper;
}

MyWebView::MyWebView(QWidget* parent) :
    QWebEngineView(parent),
    currentPrinter(nullptr)
{
}

void MyWebView::setTextSizeMultiplier(qreal factor)
{
    // Zoom Faktor wird bei printToPdf() nicht übernommen
    page()->setZoomFactor(factor);
}

void MyWebView::print(QPrinter* printer)
{
    if (printer->outputFormat() == QPrinter::PdfFormat)
    {
        page()->printToPdf(printer->outputFileName());
        delete printer;
    }
    else
    {
        if (currentPrinter == nullptr)
        {
            currentPrinter = printer;
            page()->print(printer, invoke(this, &MyWebView::slotHandlePagePrinted));
        }
    }
}

void MyWebView::slotHandlePagePrinted(bool result)
{
    Q_UNUSED(result);
    delete currentPrinter;
    currentPrinter = nullptr;
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

void MyWebView::print(QPrinter* printer)
{
    QWebView::print(printer);
    delete printer;
}

void MyWebView::slot_urlClicked(const QUrl &url)
{
    QDesktopServices::openUrl(url);
}

#endif
