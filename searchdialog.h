#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QList>
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
    void setSearchText(QString asText);
    void resetSearch();
    int adjustExtraSelections(QPlainTextEdit *aobTextEdit, QString asTextToSearch);
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
    void on_caseSentive_checkBox_stateChanged(int arg1);
    void on_wholeWords_checkBox_stateChanged(int arg1);
    void on_backward_checkBox_stateChanged(int arg1);

private:
    Ui::SearchDialog *ui;
    int giLine;
    int giCurrentOcurrence;
    int giOcurrencesFound;
    int giSearchFlag;
    QTextDocument::FindFlags giSearchCondition;
    bool gbReplaceAllClicked;
    bool gbReplaceClicked;
    bool gbSearchClicked;
    QString gsFoundText;
    QPlainTextEdit *gobTextEdit;
};

#endif // SEARCHDIALOG_H
