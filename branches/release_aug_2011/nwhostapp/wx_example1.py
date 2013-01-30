import wx
class MainWindow(wx.Frame):
    """ We simply derive a new class of Frame. """
    def __init__(self, parent, id, title):
        wx.Frame.__init__(self, parent, id, title, size=(200,100))
        self.control = wx.TextCtrl(self, 1, style=wx.TE_MULTILINE)
        self.Show(True)
        
app = wx.PySimpleApp()
frame=MainWindow(None, wx.ID_ANY, 'Small editor')
app.MainLoop()