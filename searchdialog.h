#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QPlainTextEdit>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = 0);
    void focusOnSearchInputText();
    ~SearchDialog();

signals:
    void search_signal_resetCursor();
    void search_signal_getTextEditText();
    void search_signal_disableHighLight();
    void search_signal_enableHighLight();

private slots:
    void on_searchDialog_searchButton_clicked();
    void on_searchDialog_replaceButton_clicked();
    void on_searchDialog_replaceAllButton_clicked();
    void search_slot_setTextEdit(QPlainTextEdit *textEdit);
    void on_gobSwapTextButton_clicked();

private:
    Ui::SearchDialog *ui;
    int giLine;
    int giLogCursorPos;
    int giOcurrencesFound;
    bool gbReplaceAllClicked;
    bool gbReplaceClicked;
    bool gbSearchClicked;
    QString gsFoundText;
    QPlainTextEdit *gobTextEdit;
};

#endif // SEARCHDIALOG_H
