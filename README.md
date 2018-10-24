# QNote
Lightweight yet powerful and fast QT Text editor

![QNote_GUI](https://sites.google.com/site/gtronick/QNote_1.7.4.png)
*Preview in Plasma Desktop*

# HOW TO INSTALL:

**Windows:**
Download from here: https://github.com/GTRONICK/QNote/releases/download/v1.7.4/QNote_Windows.zip  
Just decompress the zip file and run QNote.exe or, if you have a previous QNote version, just download the QNote.exe file and replace the existing one.

**Linux:**
Download from here: https://github.com/GTRONICK/QNote/releases/download/v1.7.4/QNote_Linux.zip  
Just decompress the zip file and run 

    sh QNote.sh 

**Note:** If you have a QT based system, like KDE (Plasma) or LXQT, just try to run the binary file, not the .sh.

# HOW TO BUILD FROM SOURCE:

It is strongly recommended to use QtCreator with Qt5.11.2 to compile QNote, otherwise you may experiment unexpected behavior. However, you can build it manually with:

    qmake QNote.pro
    make all -j4
    make clean

# HOW TO USE:

**1. Open new document:**   
Go to the File menu, then click on Open File... 

**2. Save a file:**   
Press Ctrl + S, or go to the File menu, then click on Save File. If the file has not been saved before, the "save as" dialog will appear.

**3. Save a File with a new name:**   
Press Ctrl + Shift + S, or go to the File menu, then click on Save As. 

**4. Open a new tab to edit a new or existing file:**   
Press Ctrl + T or go to the File menu, and Click on New Tab.

**5. Close a Tab:**   
Press Ctrl + W to close the current Tab, or press the close button in the desired tab.

**6. Reload a File:**   
To reload the current file, press F5, or go to the Tools menu and click on Reload File.

**7. Auto Reload the current File:**    
If you want to reload the current file automatically after some delay in milliseconds, pres F6, or go to the Tools menu, and select Auto Reload. This will clear the current text but not the file.

**8. To Search or replace text:**   
Press Ctrl + F, or go to the Tools menu and click on Find & Replace.

**9. To navigate to a specific line:**    
Press Ctrl + G, or go to the Tools menu, and click on Go to Line... Then, type the line you want to go to.

**10. Activate the Word Wrap:** (Active by default)     
Press Alt + W or go to the Settings menu, and check Word Wrap. This will hide the horizontal scrollbar and will wrap the text if it exceeds the horizontal page limit.

**11. Load a Theme:**   
You can load a Style Sheet for this application and make it look as cool as you want!, just go to the Settings menu, and click on Load Theme. Then select a valid Style Sheet.
Go to https://github.com/GTRONICK/QSS for some cool themes!.

**12. Revert to the system theme:**
Go to the Settings menu and click on System Theme, to revert to the default theme. 

**13. Set the default reload delay:**   
You can adjust the reload delay, to make the file reload faster, or slower. Just go to the Settings menu, and click on Auto Reload Delay. Then, specify the desired Delay, and press OK. The 
minimum value is 20 ms.

**14. Adjust the text Font:**   
Go to the Settings menu, and click on Font. The font dialog will appear, and the new font can now be selected. This will remain until you change it again. 

**15. Set the application Always on Top:**    
Go to the Settings menu and check the Always on Top check box. This change will apply immediately. This could not work as spected in Windows, for now.

**16. Erase And Save:**
Press F8 or go to Tools -> Erase And Save. This will delete the file contents and save it as an empty file.

**17. Erase, save and Auto Reload:**
Press F7, or go to Tools -> Erase, save and Auto Reload. This will delete the file contents, and activate the auto reload feature.

## ADVANCED USAGE:

**16. Group control:**		
This feature allow you to assign up to 4 buffers to hold text. Press Ctrl + <1-4> to assign the desired group, for example:

Ctrl +1, will assign the current selected text, to the buffer 1; If no text is selected, the buffer will be cleared.

To paste a buffer content at the current cursor position or selection, press F <1-4>.

Think in this as a multi-clipboard.

**17. Open the file's containing folder:**		
Click on the status bar, at the file path location to open the containing folder, or press F9.

## IN THIS VERSION:

  1. General bug fixes.
  2. Reduced code size.
  3. Improved recent files ordering.
	
QNote is now compiled with QT5.11.

## HOW TO CONTRIBUTE:

Create a new issue if you find some bug, or have any suggestion.
