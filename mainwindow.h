#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define MAX_RECENT 20
#define TABCHARS    4

#include <QMainWindow>
#include <QThread>
#include <QTimer>
#include <QMovie>
#include <QAction>
#include <QAbstractButton>
#include "searchdialog.h"
#include "customtextedit.h"
#include "worker.h"
#include "downloadmanager.h"

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
    void lockTextEditor();
    bool showCustomMessage(QString asTitle, QString asText, QString asCustomButtonText);
    void showTimedStatusMessage(QString asMessage, int aiTimeMsecs);
    ~MainWindow();

signals:
    void main_signal_setTextEdit(QPlainTextEdit *textdit);
    void main_signal_loadFile(QFile *file);
    void main_signal_refreshHighlight();
    void main_signal_tailFile(QFile *file);
    void main_signal_setCurrentFileSize(int aiFileSize);

private slots:
    bool on_actionSave_As_triggered();
    bool on_actionSave_triggered();
    void main_slot_appendText(QString asText);
    void main_slot_currentLineChanged();
    void main_slot_dontShowAgain(bool abValue);
    void main_slot_getTextEditText();
    void main_slot_gr1();
    void main_slot_gr2();
    void main_slot_gr3();
    void main_slot_gr4();
    void main_slot_insertText(QString asText);
    void main_slot_loadFileFromAction(QAction *aobAction);
    void main_slot_openFileLocation();
    void main_slot_pasteGr1();
    void main_slot_pasteGr2();
    void main_slot_pasteGr3();
    void main_slot_pasteGr4();
    void main_slot_processDropEvent(QDropEvent *event);
    void main_slot_resetCursor();
    void main_slot_resetStatusBarText();
    void main_slot_showHideMenuBar();
    void main_slot_tabChanged(int aIndex);
    void main_slot_tabMoved(int from, int to);
    void main_slot_tailFile();
    void main_slot_textChanged();
    void on_actionAbout_QNote_triggered();
    void on_actionAbout_QT_triggered();
    void on_actionAlways_on_top_triggered(bool checked);
    void on_actionAuto_Reload_delay_triggered();
    void on_actionAuto_Reload_tail_f_toggled(bool arg1);
    void on_actionClose_Tab_triggered();
    void on_actionErase_and_save_triggered();
    void on_actionFind_Replace_triggered();
    void on_actionFont_triggered();
    void on_actionGo_to_line_triggered();
    void on_actionLoad_theme_triggered();
    void on_actionNew_Tab_triggered();
    void on_actionOpen_triggered();
    void on_actionReload_file_triggered();
    void on_actionReset_alerts_triggered();
    void on_actionSet_Maximun_file_size_triggered();
    void on_actionSystem_theme_triggered();
    void on_actionTo_UPERCASE_triggered();
    void on_actionTo_lowercase_triggered();
    void on_actionWord_wrap_toggled(bool arg1);
    void on_statusBar_linkActivated(const QString &link);
    void on_tabWidget_tabCloseRequested(int index);
    void on_actionErase_and_save_2_triggered();

private:
    bool checkFileExist(QString asFileName);
    bool loadConfig();
    bool saveConfig();
    bool saveFile(QString asFileName, QString asText);
    bool saveFile(QString asFileName);
    //int checkFileSize(QString asFileName);
    void checkIfUnsaved(int index);
    void closeTab(int index);
    void loadFile(QString asFileName);
    void setCurrentTabNameFromFile(QString asFileName);
    void addRecentFiles();
    void disableAutoReload();
    QStringList removeDuplicates(QStringList aobList);

    Ui::MainWindow *ui;             //Interfaz de usuario

    float gfMaxFileSize;

    int giCurrentFileIndex;         //indice para el archivo actual que se abrirá
    int giCurrentTabIndex;          //indice para la pestaña actual
    int giDefaultDirCounter;        //Contador de archivos abiertos mediante Open, para aegurar que la proxima vez que se abra un archivo, se cargue el directorio del ultimo abierto.
    int giOpenWithFlag;
    int giSavedFontPointSize;
    int giSavedFontStyle;
    int giTimerDelay;               //Tiempo de espera en milisegundos para la recarga automática
    int giTotalTabs;                //Total de pestañas abiertas

    bool gbShowEraseAndSaveMessageBox;  //Bandera que indica se se muestra o no el QMessageBox de Borrado, guardado y recarga.
    bool gbIsOpenedFile;            //Bandera que indica si se está abriendo un archivo
    bool gbIsReloadFile;            //Bandera que indica si un archivo se ha recargado
    bool gbIsAutoreloadEnabled;     //Bandera que indica si la recarga automatica esta activa
    bool gbSaveCancelled;           //Bandera que indica si se canceló el guardado del archivo

    QHash<int, QString> gobFilePathsHash;    //Mapa que almacena índice del tab y ruta de archivo
    QHash<int, bool> gobIsModifiedTextHash;     //Mapa que almacena índice del tab, y si el archivo correspondiente ha sido modificado
    QHash<int, bool> gobIsReloadingHash;     //Mapa que almacena índice del tab, y si el archivo se encuentra en recarga automática

    QString gsDefaultDir;           //Cadena con la ruta del directorio por defecto a mostrar al abrir un archivo.
    QString gsGr1;
    QString gsGr2;
    QString gsGr3;
    QString gsGr4;
    QString gsGr5;
    QString gsSavedFont;
    QString gsStatusBarTemporalText;
    QString gsThemeFile;            //Ruta del archivo del tema usado (style.qss)
    QString gsStatusBarColor;

    QStringList gobFileNames;       //Lista de archivos arrastrados o abiertos.
    QStringList gobRecentFiles;     //Lista de archivos recientes.

    SearchDialog *gobSearchDialog;  //Diálogo de buscar y reemplazar
    CustomTextEdit *gobCurrentPlainTextEdit;    //Objeto que almacena el QPlainTextEdit actual
    QMovie *gobMovie;
    QThread *workerThread;          //Hilo separado del hilo principal
    QTimer *gobTimer;               //Temporizador para recarga automática de archivos
    Worker *worker;                 //
    DownloadManager *gobDownloadManager;



protected:
    virtual void dropEvent(QDropEvent *event);
};

#endif // MAINWINDOW_H
