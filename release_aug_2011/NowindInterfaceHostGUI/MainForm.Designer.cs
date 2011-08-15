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
        System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
        this.labelDisk1 = new System.Windows.Forms.Label();
        this.panelTop = new System.Windows.Forms.Panel();
        this.chkboxDebug = new System.Windows.Forms.CheckBox();
        this.listBox1 = new System.Windows.Forms.ListBox();
        this.panelTop.SuspendLayout();
        this.SuspendLayout();
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
        this.panelTop.AutoSizeMode = System.Windows.Forms.AutoSizeMode.GrowAndShrink;
        this.panelTop.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("panelTop.BackgroundImage")));
        this.panelTop.BackgroundImageLayout = System.Windows.Forms.ImageLayout.None;
        this.panelTop.Controls.Add(this.chkboxDebug);
        this.panelTop.Controls.Add(this.labelDisk1);
        this.panelTop.Dock = System.Windows.Forms.DockStyle.Top;
        this.panelTop.Location = new System.Drawing.Point(0, 0);
        this.panelTop.Margin = new System.Windows.Forms.Padding(0);
        this.panelTop.Name = "panelTop";
        this.panelTop.Size = new System.Drawing.Size(922, 171);
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
        // listBox1
        // 
        this.listBox1.Dock = System.Windows.Forms.DockStyle.Fill;
        this.listBox1.FormattingEnabled = true;
        this.listBox1.Location = new System.Drawing.Point(0, 171);
        this.listBox1.Name = "listBox1";
        this.listBox1.Size = new System.Drawing.Size(922, 238);
        this.listBox1.TabIndex = 4;
        // 
        // MainForm
        // 
        this.AllowDrop = true;
        this.AutoScaleDimensions = new System.Drawing.SizeF(96F, 96F);
        this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Dpi;
        this.BackColor = System.Drawing.SystemColors.ControlLight;
        this.ClientSize = new System.Drawing.Size(922, 410);
        this.Controls.Add(this.listBox1);
        this.Controls.Add(this.panelTop);
        this.MaximumSize = new System.Drawing.Size(930, 1000);
        this.Name = "MainForm";
        this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
        this.Text = "NowindInterfaceHostGUI";
        this.panelTop.ResumeLayout(false);
        this.panelTop.PerformLayout();
        this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.Label labelDisk1;
    private System.Windows.Forms.Panel panelTop;
    private System.Windows.Forms.CheckBox chkboxDebug;
    private System.Windows.Forms.ListBox listBox1;
  }
}

