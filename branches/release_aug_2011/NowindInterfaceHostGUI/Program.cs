using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.IO;

namespace NowindInterfaceHostGUI
{
  static class Program
  {
    /// <summary>
    /// The main entry point for the application.
    /// </summary>
    [STAThread]
    static void Main()
    {
      Application.EnableVisualStyles();
      Application.SetCompatibleTextRenderingDefault(false);

      Settings.ApplicationPath = Path.GetDirectoryName(Application.ExecutablePath);
      Application.Run(new MainForm());
    }
  }
}
