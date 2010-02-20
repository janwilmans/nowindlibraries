#!/usr/bin/python
"""UsbHost v1.0 (C) 2009 by Team Nowind
"""

import sys
import wx
import os.path

import pynowind

class FileDrop(wx.FileDropTarget):
    def __init__(self, window):
        wx.FileDropTarget.__init__(self)
        self.window = window

    def OnDropFiles(self, x, y, filenames):

        for name in filenames:
            try:
                file = open(name, 'r')
                text = file.read()
                self.window.WriteText(text)
                file.close()
            except IOError, error:
                dlg = wx.MessageDialog(None, 'Error opening file\n' + str(error))
                dlg.ShowModal()
            except UnicodeDecodeError, error:
                dlg = wx.MessageDialog(None, 'Cannot open non ascii files\n' + str(error))
                dlg.ShowModal()



class TopPanel(wx.Panel):
    def __init__(self, parent, id):
        wx.Panel.__init__(self, parent, id, style=wx.BORDER_SUNKEN)

        button1 = wx.Button(self, -1, 'Start', (10, 10))
        button2 = wx.Button(self, -1, 'Exit', (10, 60))

        self.Bind(wx.EVT_BUTTON, self.OnStart, id=button1.GetId())
        self.Bind(wx.EVT_BUTTON, self.OnExit, id=button2.GetId())

    def OnStart(self, event):
       mainPanel = self.GetParent().GetParent()
       mainPanel.logBox.write("Start...")
       

    def OnExit(self, event):
        os._exit(1)


class BottomPanel(wx.Panel):
    def __init__(self, parent, id):
        wx.Panel.__init__(self, parent, id, style=wx.BORDER_NONE)
        #self.text = wx.StaticText(self, -1, '0', (40, 60))
        
        self.logBox = wx.TextCtrl(self, style=wx.TE_MULTILINE)
        #self.logBox.IsEditable = 0
        
        hbox = wx.BoxSizer(wx.VERTICAL)
        hbox.Add(self.logBox, 1, wx.EXPAND | wx.ALL, 0)
        self.SetSizer(hbox)
        
        dt = FileDrop(self.logBox)
        self.logBox.SetDropTarget(dt)

    
class MainWindow(wx.Frame):
    def __init__(self, filename='noname.txt'):
        super(MainWindow, self).__init__(None, size=(600,400))
        self.filename = filename
        self.dirname = '.'
        self.CreateInteriorWindowComponents()
        self.CreateExteriorWindowComponents()

    def CreateInteriorWindowComponents(self):

        panel = wx.Panel(self, -1)
        self.topPanel = TopPanel(panel, -1)
        self.bottomPanel = BottomPanel(panel, -1)
        self.logBox = self.bottomPanel.logBox

        hbox = wx.BoxSizer(wx.VERTICAL)
        hbox.Add(self.topPanel, 2, wx.EXPAND | wx.ALL, 5)
        hbox.Add(self.bottomPanel, 1, wx.EXPAND | wx.ALL, 5)
        panel.SetSizer(hbox)

    def CreateExteriorWindowComponents(self):
        ''' Create "exterior" window components, such as menu and status
            bar. '''
        #self.CreateMenu()
        self.CreateStatusBar()
        self.SetTitle()

    def CreateMenu(self):
        fileMenu = wx.Menu()
        for id, label, helpText, handler in \
            [(wx.ID_ABOUT, '&About', 'Information about this program',
                self.OnAbout),
             (wx.ID_OPEN, '&Open', 'Open a new file', self.OnOpen),
             (wx.ID_SAVE, '&Save', 'Save the current file', self.OnSave),
             (wx.ID_SAVEAS, 'Save &As', 'Save the file under a different name',
                self.OnSaveAs),
             (None, None, None, None),
             (wx.ID_EXIT, 'E&xit', 'Terminate the program', self.OnExit)]:
            if id == None:
                fileMenu.AppendSeparator()
            else:
                item = fileMenu.Append(id, label, helpText)
                self.Bind(wx.EVT_MENU, handler, item)

        menuBar = wx.MenuBar()
        menuBar.Append(fileMenu, '&File') # Add the fileMenu to the MenuBar
        self.SetMenuBar(menuBar)  # Add the menuBar to the Frame

    def SetTitle(self):
        # MainWindow.SetTitle overrides wx.Frame.SetTitle, so we have to
        # call it using super:
        super(MainWindow, self).SetTitle('Editor %s'%self.filename)


    # Helper methods:

    def defaultFileDialogOptions(self):
        ''' Return a dictionary with file dialog options that can be
            used in both the save file dialog as well as in the open
            file dialog. '''
        return dict(message='Choose a file', defaultDir=self.dirname,
                    wildcard='*.*')

    def askUserForFilename(self, **dialogOptions):
        dialog = wx.FileDialog(self, **dialogOptions)
        if dialog.ShowModal() == wx.ID_OK:
            userProvidedFilename = True
            self.filename = dialog.GetFilename()
            self.dirname = dialog.GetDirectory()
            self.SetTitle() # Update the window title with the new filename
        else:
            userProvidedFilename = False
        dialog.Destroy()
        return userProvidedFilename

    # Event handlers:

    def OnAbout(self, event):
        dialog = wx.MessageDialog(self, 'Nowind Interface Host GUI v1.0 by Team Nowind (C) 2009\n',
            'About Nowind Interface Host GUI', wx.OK)
        dialog.ShowModal()
        dialog.Destroy()

    def OnExit(self, event):
        self.Close()  # Close the main window.

    def OnSave(self, event):
        textfile = open(os.path.join(self.dirname, self.filename), 'w')
        textfile.write(self.control.GetValue())
        textfile.close()

    def OnOpen(self, event):
        if self.askUserForFilename(style=wx.OPEN,
                                   **self.defaultFileDialogOptions()):
            textfile = open(os.path.join(self.dirname, self.filename), 'r')
            self.control.SetValue(textfile.read())
            textfile.close()

    def OnSaveAs(self, event):
        if self.askUserForFilename(defaultFile=self.filename, style=wx.SAVE,
                                   **self.defaultFileDialogOptions()):
            self.OnSave(event)


# redirect=0 causes exceptions to be printed on the commandline (as usual)
app = wx.App(redirect=0)
frame = MainWindow()
# frame.Show()
# app.MainLoop()
  
pynowind.start()

