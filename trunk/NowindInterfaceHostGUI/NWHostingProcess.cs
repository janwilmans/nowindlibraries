using System;
using System.Collections.Generic;
using System.Text;
using System.IO;            //File
using System.Diagnostics;
using System.Windows.Forms; //MessageBox

namespace NowindInterfaceHostGUI
{
  public class NWHostingProcess
  {
    private bool HostRunning = false;

    public NWHostingProcess()
    {
      nwhost = new System.Diagnostics.Process();
    }

    private System.Diagnostics.Process nwhost;

    public void killProcessIfRunning()
    {
      if (HostRunning)
      {
        try
        {
          // Ignore any errors that might occur while closing the process
          if (!nwhost.HasExited)
          {
            nwhost.Kill();
          }
        }
        catch { }
        HostRunning = false;
      }
    }

    public void startHosting()
    {
      killProcessIfRunning();
      
      nwhost.StartInfo.FileName = Settings.ApplicationPath + "\\nwhostapp.exe";
      if (!File.Exists(nwhost.StartInfo.FileName))
      {
        MessageBox.Show("Cannot open: " + nwhost.StartInfo.FileName);
        return;
      }
      if (Settings.Disk != "" && !File.Exists(Settings.Disk))
      {
        MessageBox.Show("Cannot open: " + Settings.Disk);
        return;
      }
      nwhost.StartInfo.Arguments = "-a";

      if (Settings.Disk != "")
      {
        nwhost.StartInfo.Arguments += String.Format(" -i {0}", Settings.Disk);
      }

      Debug.WriteLine("[exec] " + nwhost.StartInfo.FileName + " " + nwhost.StartInfo.Arguments);
      nwhost.StartInfo.UseShellExecute = false;
      if (Settings.Debug)
      {
        // debugging
        nwhost.StartInfo.RedirectStandardOutput = false;
        nwhost.StartInfo.CreateNoWindow = false;
      }
      else
      {
        nwhost.StartInfo.RedirectStandardOutput = true;
        nwhost.StartInfo.CreateNoWindow = true;
      }
      nwhost.Start();
      HostRunning = true;

      Settings.MainForm.Invoke((MethodInvoker) delegate()
      {
        Settings.MainForm.Text = "Hosting process started...";
      });

    }

  }
}
