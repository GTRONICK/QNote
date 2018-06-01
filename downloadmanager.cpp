#include "downloadmanager.h"
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

DownloadManager::DownloadManager(QObject *parent) : QObject(parent)
{
    connect(&manager, SIGNAL(finished(QNetworkReply*)),SLOT(downloadFinished(QNetworkReply*)));
}

void DownloadManager::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (X11; U; Linux x86_64; en-US; rv:1.8.1.12) Gecko/20080214 Firefox/2.0.0.12");
    QNetworkReply *reply = manager.get(request);

#if QT_CONFIG(ssl)
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),this,SLOT(sslErrors(QList<QSslError>)));
#endif

}

QString DownloadManager::saveFileName(const QUrl &url)
{
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();

    return basename;
}

bool DownloadManager::saveToDisk(const QString &filename, QIODevice *data)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open %s for writing: %s\n" << qPrintable(filename) << qPrintable(file.errorString());
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

bool DownloadManager::isHttpRedirect(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadManager::execute(QString lsLink)
{
    QUrl url = QUrl::fromEncoded(lsLink.toLocal8Bit());
    doDownload(url);
}

void DownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#if QT_CONFIG(ssl)
    for (const QSslError &error : sslErrors)
        qDebug() << "SSL error: %s\n" << qPrintable(error.errorString());
#else
    Q_UNUSED(sslErrors);
#endif
}

void DownloadManager::downloadFinished(QNetworkReply *reply)
{
    QUrl url = reply->url();
    if (reply->error()) {
        qDebug() << "Failed" << " " << reply->errorString();
    } else {
        if (isHttpRedirect(reply)) {
            qDebug() << "Redirected";
        } else {
            QString filename = saveFileName(url);
            if (saveToDisk(filename, reply)) {
                qDebug() << "File downloaded";
            }
        }
    }

    reply->deleteLater();
}
