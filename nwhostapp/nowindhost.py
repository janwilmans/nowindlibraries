#!/usr/bin/python
"""UsbHost v1.0 (C) 2009 by Team Nowind
"""

import sys
import wx
import os.path
import nowind

class FileDrop(wx.FileDropTarget):
    def __init__(self, window):
        wx.FileDropTarget.__init__(self)
        self.window = window

    def OnDropFiles(self, x, y, filenames):

        for name in filenames:
            try:
                print "file dropped..."
                #file = open(name, 'r')
                #text = file.read()
                self.window.SetValue(name)

                #file.close()
            except IOError, error:
                dlg = wx.MessageDialog(None, 'Error opening file\n' + str(error))
                dlg.ShowModal()
            except UnicodeDecodeError, error:
                dlg = wx.MessageDialog(None, 'Cannot open non ascii files\n' + str(error))
                dlg.ShowModal()



class TopPanel(wx.Panel):

    dialog = 0
    def __init__(self, parent, id):
        wx.Panel.__init__(self, parent, id, style=wx.BORDER_SUNKEN)

        self.textbox = wx.TextCtrl(self, -1, "", (10, 10), size=(230,30))
        button1 = wx.Button(self, -1, 'Browse', (245, 10), size=(80,30))
        button2 = wx.Button(self, -1, 'Exit', (10, 45))

        dt = FileDrop(self.textbox)
        self.SetDropTarget(dt)

        self.dialog = wx.FileDialog ( None, style = wx.OPEN )

        self.Bind(wx.EVT_BUTTON, self.ShowFileDialog, id=button1.GetId())
        self.Bind(wx.EVT_BUTTON, self.OnExit, id=button2.GetId())

    def OnExit(self, event):
        nowind.insertDisk(self.textbox.GetValue())
        nowind.hostImage()
        os._exit(1)

    def ShowFileDialog(self, event):
        self.dialog.Show()
        if self.dialog.ShowModal() == wx.ID_OK:
            self.textbox.SetValue(self.dialog.GetFilename())
    
class MainWindow(wx.Frame):
    def __init__(self, filename='noname.txt'):
        super(MainWindow, self).__init__(None, size=(350,100))
        self.filename = filename
        self.dirname = '.'
        self.CreateInteriorWindowComponents()
        self.CreateExteriorWindowComponents()

    def CreateInteriorWindowComponents(self):

        panel = wx.Panel(self, -1)
        self.topPanel = TopPanel(panel, -1)

        hbox = wx.BoxSizer(wx.VERTICAL)
        hbox.Add(self.topPanel, 2, wx.EXPAND | wx.ALL, 5)
        panel.SetSizer(hbox)

    def CreateExteriorWindowComponents(self):
        ''' Create "exterior" window components, such as menu and status
            bar. '''
        self.SetTitle('Nowind Host')
  

# redirect=0 causes exceptions to be printed on the commandline (as usual)
app = wx.App(redirect=0)
frame = MainWindow()
frame.Show()

nowind.init()
app.MainLoop()
   
