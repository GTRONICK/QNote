#ifndef CUSTOMTABWIDGET_H
#define CUSTOMTABWIDGET_H
#include <QTabWidget>

class CustomTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    CustomTabWidget(QWidget *parent);

public slots:
    void ctw_slot_reorderTabs(int from, int to);

signals:
    void ctw_signal_tabMoved(int from, int to);
};

#endif // CUSTOMTABWIDGET_H
