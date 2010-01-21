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
    public NWHostingProcess()
    {
      nwhost = new System.Diagnostics.Process();
    }

    private System.Diagnostics.Process nwhost;

    public void killProcessIfRunning()
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
    }

    public void startHosting(string disk)
    {
      killProcessIfRunning();
      
      nwhost.StartInfo.FileName = Settings.ApplicationPath + "\\nwhostapp.exe";
      if (!File.Exists(nwhost.StartInfo.FileName))
      {
        MessageBox.Show("Cannot open: " + nwhost.StartInfo.FileName);
        return;
      }

      nwhost.StartInfo.Arguments = String.Format("-i {0} -a", disk);
      nwhost.StartInfo.UseShellExecute = false;
      if (Settings.Debug)
      {
        // debugging
      }
      else
      {
        nwhost.StartInfo.RedirectStandardOutput = true;
        nwhost.StartInfo.CreateNoWindow = true;
      }
      nwhost.Start();
    }
  }
}
