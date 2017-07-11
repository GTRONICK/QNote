#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QFile>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = 0);

signals:
    void worker_signal_appendText(QString line);

public slots:
    void worker_slot_loadFile(QFile *file);
};

#endif // WORKER_H
