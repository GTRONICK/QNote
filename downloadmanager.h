#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QtCore>
#include <QNetworkAccessManager>

class DownloadManager : public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager;

public:
    explicit DownloadManager(QObject *parent = nullptr);
    static QString saveFileName(const QUrl &url);
    static bool isHttpRedirect(QNetworkReply *reply);
    bool saveToDisk(const QString &filename, QIODevice *data);
    void doDownload(const QUrl &url);


signals:

public slots:
    void execute(QString lsLink);
    void downloadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);
};

#endif // DOWNLOADMANAGER_H
