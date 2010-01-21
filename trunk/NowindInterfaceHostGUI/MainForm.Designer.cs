namespace NowindInterfaceHostGUI
{
  partial class MainForm
  {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing)
    {
      if (disposing && (components != null))
      {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent()
    {
      this.btnInsert = new System.Windows.Forms.Button();
      this.btnRestart = new System.Windows.Forms.Button();
      this.labelDisk1 = new System.Windows.Forms.Label();
      this.panelTop = new System.Windows.Forms.Panel();
      this.chkboxDebug = new System.Windows.Forms.CheckBox();
      this.panelTop.SuspendLayout();
      this.SuspendLayout();
      // 
      // btnInsert
      // 
      this.btnInsert.Location = new System.Drawing.Point(12, 54);
      this.btnInsert.Margin = new System.Windows.Forms.Padding(0);
      this.btnInsert.Name = "btnInsert";
      this.btnInsert.Size = new System.Drawing.Size(89, 35);
      this.btnInsert.TabIndex = 0;
      this.btnInsert.Text = "&Insert";
      this.btnInsert.UseVisualStyleBackColor = true;
      this.btnInsert.Click += new System.EventHandler(this.btnInsert_Click);
      // 
      // btnRestart
      // 
      this.btnRestart.Location = new System.Drawing.Point(117, 54);
      this.btnRestart.Name = "btnRestart";
      this.btnRestart.Size = new System.Drawing.Size(89, 35);
      this.btnRestart.TabIndex = 1;
      this.btnRestart.Text = "(re)start hosting";
      this.btnRestart.UseVisualStyleBackColor = true;
      this.btnRestart.Click += new System.EventHandler(this.btnRestart_Click);
      // 
      // labelDisk1
      // 
      this.labelDisk1.AutoSize = true;
      this.labelDisk1.Location = new System.Drawing.Point(42, 19);
      this.labelDisk1.Name = "labelDisk1";
      this.labelDisk1.Size = new System.Drawing.Size(0, 13);
      this.labelDisk1.TabIndex = 2;
      // 
      // panelTop
      // 
      this.panelTop.Controls.Add(this.chkboxDebug);
      this.panelTop.Controls.Add(this.labelDisk1);
      this.panelTop.Controls.Add(this.btnRestart);
      this.panelTop.Controls.Add(this.btnInsert);
      this.panelTop.Dock = System.Windows.Forms.DockStyle.Fill;
      this.panelTop.Location = new System.Drawing.Point(0, 0);
      this.panelTop.Margin = new System.Windows.Forms.Padding(0);
      this.panelTop.Name = "panelTop";
      this.panelTop.Size = new System.Drawing.Size(494, 168);
      this.panelTop.TabIndex = 3;
      // 
      // chkboxDebug
      // 
      this.chkboxDebug.AutoSize = true;
      this.chkboxDebug.Location = new System.Drawing.Point(12, 139);
      this.chkboxDebug.Name = "chkboxDebug";
      this.chkboxDebug.Size = new System.Drawing.Size(159, 17);
      this.chkboxDebug.TabIndex = 3;
      this.chkboxDebug.Text = "show console for debug info";
      this.chkboxDebug.UseVisualStyleBackColor = true;
      this.chkboxDebug.CheckedChanged += new System.EventHandler(this.chkboxDebug_CheckedChanged);
      // 
      // MainForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.BackColor = System.Drawing.SystemColors.ControlLight;
      this.ClientSize = new System.Drawing.Size(494, 168);
      this.Controls.Add(this.panelTop);
      this.Name = "MainForm";
      this.Text = "NowindInterfaceHostGUI";
      this.panelTop.ResumeLayout(false);
      this.panelTop.PerformLayout();
      this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.Button btnInsert;
    private System.Windows.Forms.Button btnRestart;
    private System.Windows.Forms.Label labelDisk1;
    private System.Windows.Forms.Panel panelTop;
    private System.Windows.Forms.CheckBox chkboxDebug;
  }
}

