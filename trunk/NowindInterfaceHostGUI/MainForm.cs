using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace NowindInterfaceHostGUI
{
  public partial class MainForm : Form
  {
    public NWHostingProcess NWHostingProcess;
    public MainForm()
    {
      InitializeComponent();
      NWHostingProcess = new NWHostingProcess();
      Application.ApplicationExit += new EventHandler(this.OnApplicationExit);
    }

    private void OnApplicationExit(object sender, EventArgs e)
    {
      try
      {
        // Ignore any errors that might occur while closing the application
        NWHostingProcess.killProcessIfRunning();
      }
      catch { }
    }

    private void btnInsert_Click(object sender, EventArgs e)
    {
      OpenFileDialog openFileDialog = new OpenFileDialog();
      openFileDialog.InitialDirectory = "c:\\" ;
      openFileDialog.Filter = "disk images (*.dsk)|*.dsk|All files (*.*)|*.*" ;
      openFileDialog.FilterIndex = 2;
      openFileDialog.RestoreDirectory = true;

      if (openFileDialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
      {
        labelDisk1.Text = openFileDialog.FileName;
        Settings.Disk = openFileDialog.FileName;
      }      
    }

    /*
     * todo: 
     * - support drag/drop of disk to window
     * x kill nwhostapp (my own) if already running and when window is closed
     */

    private void btnRestart_Click(object sender, EventArgs e)
    {
      NWHostingProcess.startHosting(Settings.Disk);
    }

    private void chkboxDebug_CheckedChanged(object sender, EventArgs e)
    {
      Settings.Debug = chkboxDebug.Checked;
    }
  }
}
