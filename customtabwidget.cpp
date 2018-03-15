#include "customtabwidget.h"
#include <QDebug>
#include <QTabBar>
#include <QTableWidget>

CustomTabWidget::CustomTabWidget(QWidget *parent):QTabWidget(parent)
{
    connect(this->tabBar(),SIGNAL(tabMoved(int,int)),this,SLOT(ctw_slot_reorderTabs(int,int)));
}

void CustomTabWidget::ctw_slot_reorderTabs(int from, int to)
{
    emit(ctw_signal_tabMoved(from, to));
}
