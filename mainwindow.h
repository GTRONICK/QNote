#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define MAX_SIZE 20000000.0

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QMovie>
#include <QAction>
#include "searchdialog.h"
#include "customtextedit.h"
#include "worker.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void closeEvent(QCloseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void setFileNameFromCommandLine(QStringList asFileNames);
    void setStatusBarTextAsLink(QString asText);
    ~MainWindow();

signals:
    void main_signal_setTextEdit(QPlainTextEdit *textdit);
    void main_signal_loadFile(QFile *file);
    void main_signal_refreshHighlight();

private slots:
    void on_actionOpen_triggered();
    bool on_actionSave_As_triggered();
    bool on_actionSave_triggered();
    void on_actionAbout_QNote_triggered();
    void on_actionAbout_QT_triggered();
    void on_tabWidget_tabCloseRequested(int index);
    void on_actionNew_Tab_triggered();
    void main_slot_tabChanged(int aIndex);
    void main_slot_tabMoved(int from, int to);
    void main_slot_textChanged();
    void on_actionClose_Tab_triggered();
    void main_slot_getTextEditText();
    void main_slot_resetCursor();
    void on_actionFind_Replace_triggered();
    void main_slot_appendText(QString asText);
    void main_slot_processDropEvent(QDropEvent *event);
    void on_actionReload_file_triggered();
    void on_actionWord_wrap_toggled(bool arg1);
    void on_actionGo_to_line_triggered();
    void on_actionLoad_theme_triggered();
    void on_actionSystem_theme_triggered();
    void on_actionAuto_Reload_tail_f_toggled(bool arg1);
    void on_actionAuto_Reload_delay_triggered();
    void main_slot_currentLineChanged();
    void on_actionFont_triggered();
    void on_actionAlways_on_top_triggered(bool checked);
    void main_slot_showHideMenuBar();

    void main_slot_gr1();
    void main_slot_gr2();
    void main_slot_gr3();
    void main_slot_gr4();
    void main_slot_gr5();

    void main_slot_pasteGr1();
    void main_slot_pasteGr2();
    void main_slot_pasteGr3();
    void main_slot_pasteGr4();
    void main_slot_pasteGr5();

    void main_slot_resetStatusBarText();
    void on_statusBar_linkActivated(const QString &link);
    void main_slot_loadFileFromAction(QAction *aobAction);

private:
    void addRecentFiles();
    void setCurrentTabNameFromFile(QString asFileName);
    void checkIfUnsaved(int index);
    bool checkFileExist(QString asFileName);
    void closeTab(int index);
    void loadFile(QString asFileName);
    bool saveFile(QString asFileName, QString asText);
    bool saveConfig();
    bool loadConfig();

    Ui::MainWindow *ui;             //Interfaz de usuario
    QStringList gobFileNames;       //Lista de archivos arrastrados o abiertos
    int giCurrentFileIndex;         //indice para el archivo actual que se abrirá
    int giCurrentTabIndex;          //indice para la pestaña actual
    int giTotalTabs;                //Total de pestañas abiertas
    int giTabCharacters;            //Number of tab spaces
    int giTimerDelay;               //Tiempo de espera en milisegundos para la recarga automática
    int giRecentFilePos;            //Contador de posicion del archivo actual
    int giRecentAux;
    int giOpenWithFlag;
    bool gbIsOpenedFile;            //Bandera que indica si se está abriendo un archivo
    bool gbIsReloadFile;            //Bandera que indica si un archivo se ha recargado
    bool gbIsAutoreloadEnabled;     //Bandera que indica si la recarga automatica esta activa
    bool gbSaveCancelled;           //Bandera que indica si se canceló el guardado del archivo
    QHash<int, QString> gobHash;    //Mapa que almacena índice del tab y ruta de archivo
    QHash<int, bool> gobIsModifiedTextHash;     //Mapa que almacena índice del tab, y si el archivo correspondiente ha sido modificado
    SearchDialog *gobSearchDialog;  //Diálogo de buscar y reemplazar
    Worker *worker;                 //Objeto para procesar tareas en un hilo separado
    QThread *workerThread;          //Hilo separado del hilo principal
    CustomTextEdit *gobCurrentPlainTextEdit;    //Objeto que almacena el QPlainTextEdit actual
    QFile *gobFile;                 //Objeto que almacena temporalemente un objeto de archivo
    QString gsThemeFile;            //Ruta del archivo del tema usado (style.qss)
    QTimer *gobTimer;               //Temporizador para recarga automática de archivos
    QMovie *gobMovie = new QMovie("://reloading.gif");
    QString gsDefaultDir;
    QString gsSavedFont;
    int giSavedFontStyle;
    int giSavedFontPointSize;
    int giDefaultDirCounter;
    QString gsStatusBarTemporalText;
    QString gsGr1;
    QString gsGr2;
    QString gsGr3;
    QString gsGr4;
    QString gsGr5;
    QStringList gobRecentFiles;

protected:
    virtual void dropEvent(QDropEvent *event);
};

#endif // MAINWINDOW_H
