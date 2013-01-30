using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace NowindInterfaceHostGUI
{
  public partial class MainForm : Form
  {
    public NWHostingProcess NWHostingProcess;
    public MainForm()
    {
      Settings.MainForm = this;
      InitializeComponent();
      this.Size = new Size(310, 150);

      NWHostingProcess = new NWHostingProcess();
      Application.ApplicationExit += new EventHandler(this.OnApplicationExit);
      this.DragEnter += new DragEventHandler(this.form_DragEnter);      // user starts hovering files above form
      this.DragDrop += new DragEventHandler(this.form_DragDrop);       // user drops files on form
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

    /*
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
    */
  
    /*
     * todo: 
     * x support drag/drop of disk to window
     * x kill nwhostapp (my own) if already running and when window is closed
     * - support log window (and save to logfile)
     */

    private void btnRestart_Click(object sender, EventArgs e)
    {
      NWHostingProcess.startHosting();
    }

    private void chkboxDebug_CheckedChanged(object sender, EventArgs e)
    {
      Settings.Debug = chkboxDebug.Checked;
      NWHostingProcess.startHosting();
    }

    private void form_DragEnter(object sender, DragEventArgs e)
    {
      if (e.Data.GetDataPresent(DataFormats.FileDrop, false) == true)
      {
        e.Effect = DragDropEffects.All;
      }
    }

    private void form_DragDrop(object sender, DragEventArgs e)
    {
      string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);

      if (files.Length > 0)
      {
        Settings.Disk = files[0];
      }
      NWHostingProcess.startHosting();

      foreach (string file in files)
      {
        Debug.WriteLine(file);
      }
    }  
  }
}
