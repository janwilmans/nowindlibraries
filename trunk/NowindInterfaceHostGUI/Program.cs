using System;
using System.Collections.Generic;
using System.Windows.Forms;

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

      Settings.ApplicationPath = Application.ExecutablePath;
      Application.Run(new MainForm());
    }
  }
}
