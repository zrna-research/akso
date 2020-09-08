/**
 * Copyright (C) 2013 - 2016 Johannes Taelman
 *
 * This file is part of Axoloti.
 *
 * Axoloti is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Axoloti is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Axoloti. If not, see <http://www.gnu.org/licenses/>.
 */
package axoloti.swingui;

import axoloti.Axoloti;
import axoloti.Synonyms;
import axoloti.Version;
import axoloti.abstractui.DocumentWindowList;
import axoloti.connection.CConnection;
import axoloti.connection.IConnection;
import axoloti.job.GlobalJobProcessor;
import axoloti.job.GlobalProgress;
import axoloti.job.IJobContext;
import axoloti.job.IProgressReporter;
import axoloti.job.JobContext;
import axoloti.mvc.AbstractDocumentRoot;
import axoloti.objectlibrary.AxoObjects;
import axoloti.objectlibrary.AxolotiLibrary;
import axoloti.patch.PatchController;
import axoloti.patch.PatchModel;
import axoloti.preferences.Preferences;
import axoloti.preferences.Theme;
import axoloti.swingui.patch.PatchFrame;
import axoloti.swingui.patch.PatchViewSwing;
import axoloti.swingui.patchbank.PatchBank;
import axoloti.swingui.target.TJFrame;
import axoloti.target.TargetModel;
import axoloti.target.TargetRTInfo;
import axoloti.utils.KeyUtils;
import java.awt.EventQueue;
import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.AdjustmentEvent;
import java.awt.event.AdjustmentListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.beans.PropertyChangeEvent;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.logging.Handler;
import java.util.logging.Level;
import java.util.logging.LogRecord;
import java.util.logging.Logger;
import javax.swing.BoundedRangeModel;
import javax.swing.JOptionPane;
import javax.swing.KeyStroke;
import javax.swing.SwingUtilities;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultCaret;
import javax.swing.text.Style;
import javax.swing.text.StyleConstants;
import javax.swing.SwingWorker;

/**
 *
 * @author Johannes Taelman
 */
public class MainFrame extends TJFrame implements ActionListener {

    public static MainFrame mainframe;

    private boolean bGrabFocusOnSevereErrors = true;

    private boolean doAutoScroll = true;

    Style styleParent;
    Style styleSevere;
    Style styleInfo;
    Style styleWarning;

    /**
     * Creates new form MainFrame
     *
     * @param args command line arguments
     */
    public MainFrame(String args[], TargetModel targetModel) {
        super(targetModel);

        initComponents();
        fileMenu.initComponents();

        mainframe = this;

        GlobalProgress.setInstance(getProgressReporter());

        styleParent = jTextPaneLog.addStyle(null, null);
        StyleConstants.setFontFamily(styleParent, Font.MONOSPACED);

        styleSevere = jTextPaneLog.addStyle("severe", styleParent);
        styleInfo = jTextPaneLog.addStyle("info", styleParent);
        styleWarning = jTextPaneLog.addStyle("warning", styleParent);
        jTextPaneLog.setBackground(Theme.getCurrentTheme().Console_Background);
        StyleConstants.setForeground(styleSevere, Theme.getCurrentTheme().Error_Text);
        StyleConstants.setForeground(styleInfo, Theme.getCurrentTheme().Normal_Text);
        StyleConstants.setForeground(styleWarning, Theme.getCurrentTheme().Warning_Text);

        DefaultCaret caret = (DefaultCaret) jTextPaneLog.getCaret();
        caret.setUpdatePolicy(DefaultCaret.NEVER_UPDATE);
        jScrollPaneLog.getVerticalScrollBar().addAdjustmentListener(new AdjustmentListener() {
            private final BoundedRangeModel brm = jScrollPaneLog.getVerticalScrollBar().getModel();

            @Override
            public void adjustmentValueChanged(AdjustmentEvent e) {
                // Invoked when user select and move the cursor of scroll by mouse explicitly.
                if (!brm.getValueIsAdjusting()) {
                    if (doAutoScroll) {
                        brm.setValue(brm.getMaximum());
                    }
                } else {
                    // doAutoScroll will be set to true when user reaches at the bottom of document.
                    doAutoScroll = ((brm.getValue() + brm.getExtent()) == brm.getMaximum());
                }
            }
        });

        jScrollPaneLog.addMouseWheelListener(new MouseWheelListener() {
            private final BoundedRangeModel brm = jScrollPaneLog.getVerticalScrollBar().getModel();

            @Override
            public void mouseWheelMoved(MouseWheelEvent e) {
                // Invoked when user use mouse wheel to scroll
                if (e.getWheelRotation() < 0) {
                    // If user trying to scroll up, doAutoScroll should be false.
                    doAutoScroll = false;
                } else {
                    // doAutoScroll will be set to true when user reaches at the bottom of document.
                    doAutoScroll = ((brm.getValue() + brm.getExtent()) == brm.getMaximum());
                }
            }
        });

        doLayout();
        model.getController().addView(MainFrame.this);
    }

    public void initLogging() {
        Handler logHandler = new Handler() {
                @Override
                public void publish(LogRecord lr) {
                    try {
                        String txt;
                        String excTxt = "";
                        Throwable exc = lr.getThrown();
                        if (exc != null) {
                            ByteArrayOutputStream baos = new ByteArrayOutputStream();
                            PrintStream ps = new PrintStream(baos);
                            exc.printStackTrace(ps);
                            excTxt = exc.toString();
                            excTxt = excTxt + "\n" + baos.toString("utf-8");
                        }
                        if (lr.getMessage() == null) {
                            txt = excTxt;
                        } else {
                            txt = java.text.MessageFormat.format(lr.getMessage(), lr.getParameters());
                            if (excTxt.length() > 0) {
                                txt = txt + "," + excTxt;
                            }
                        }

                            final String java_sucks = txt;
                            SwingUtilities.invokeLater(() -> {
                                    try {
                                        if (lr.getLevel() == Level.SEVERE) {
                                            doAutoScroll = true;
                                            jTextPaneLog.getDocument().insertString(jTextPaneLog.getDocument().getLength(),
                                                                                    java_sucks + "\n", styleSevere);
                                        } else if (lr.getLevel() == Level.WARNING) {
                                            jTextPaneLog.getDocument().insertString(jTextPaneLog.getDocument().getLength(),
                                                                                    java_sucks + "\n", styleWarning);
                                        } else {
                                            jTextPaneLog.getDocument().insertString(jTextPaneLog.getDocument().getLength(),
                                                                                    java_sucks + "\n", styleInfo);
                                        }
                                    } catch (BadLocationException ex) {
                                        ex.printStackTrace();
                                    }
                                });

                    } catch (UnsupportedEncodingException ex) {
                    }
                }

                @Override
                public void flush() {
                    jScrollPaneLog.removeAll();
                }

                @Override
                public void close() throws SecurityException {
                }
            };

        logHandler.setLevel(Level.INFO);

        Logger.getLogger("").addHandler(logHandler);
        Logger.getLogger("").setLevel(Level.INFO);


        Logger.getLogger(MainFrame.class.getName()).log(
            Level.INFO, "Version : {0}", new Object[]{Version.AXOLOTI_VERSION});
        Logger.getLogger(MainFrame.class.getName()).log(
            Level.INFO, "Initializing. Just a moment.", new Object[]{Version.AXOLOTI_VERSION});
        Logger.getLogger(MainFrame.class.getName()).log(
            Level.INFO, "Loading object libraries.", new Object[]{Version.AXOLOTI_VERSION});
    }

    public void initLibrary() {
        SwingWorker<Boolean, Object> sw = new SwingWorker<Boolean, Object>() {
                @Override
                public Boolean doInBackground() {
                    try {
                        for (AxolotiLibrary lib : Preferences.getPreferences().getLibraries()) {
                            if (lib.getEnabled()) {
                                lib.init(false);
                            }
                        }

                        // user library, ask user if they wish to upgrade, or do manuall
                        // this allows them the opportunity to manually backup their files!
                        AxolotiLibrary ulib = Preferences.getPreferences().getLibrary(
                            AxolotiLibrary.USER_LIBRARY_ID);
                        if (ulib != null) {
                            String cb = ulib.getCurrentBranch();
                            if (!cb.equalsIgnoreCase(ulib.getBranch())) {
                                Logger.getLogger(
                                    MainFrame.class.getName()).log(
                                        Level.INFO,
                                        "Current user library does not match correct version {0} -> {1}",
                                        new Object[]{cb, ulib.getBranch()});
                                return true;
                            }
                        }

                        // factory library force and upgrade
                        // Im stashing changes here, just in case, but in reality users should not be altering factory
                        ulib = Preferences.getPreferences().getLibrary(AxolotiLibrary.FACTORY_ID);
                        if (ulib != null) {
                            String cb = ulib.getCurrentBranch();
                            if (!cb.equalsIgnoreCase("experimental")
                                && !cb.equalsIgnoreCase(ulib.getBranch())) {
                                Logger.getLogger(MainFrame.class.getName()).log(
                                    Level.INFO,
                                    "Current factory library does not match correct version, upgrading {0} -> {1}",
                                    new Object[]{cb, ulib.getBranch()});
                                ulib.upgrade();
                            }
                        }

                        for (AxolotiLibrary lib : Preferences.getPreferences().getLibraries()) {
                            if (lib.isAutoSync() && lib.getEnabled()) {
                                lib.sync();
                            }
                        }

                        for (AxolotiLibrary lib : Preferences.getPreferences().getLibraries()) {
                            lib.reportStatus();
                        }

                        Synonyms.instance();

                        IJobContext jobContext = new JobContext();
                        GlobalJobProcessor.getJobProcessor().exec((ctx) -> {
                                AxoObjects.loadAxoObjects(jobContext);
                            });

                        CConnection.getConnection().connect(null);
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    return false;
                }

                @Override
                protected void done() {
                    try {
                        if(get()) {
                            int s = JOptionPane.showConfirmDialog(
                                MainFrame.this,
                                "User Library version mismatch, do you want to upgrade?\n"
                                + "this will stash any changes, and then reapply to new version\n"
                                + "if not, then you will need to manually backup changes, and then sync libraries",
                                "User Library mismatch",
                                JOptionPane.YES_NO_OPTION);
                            AxolotiLibrary ulib = Preferences.getPreferences().getLibrary(
                                AxolotiLibrary.USER_LIBRARY_ID);
                            if (s == JOptionPane.YES_OPTION) {
                                ulib.upgrade();
                            }
                        }
                    } catch (Exception ignore) {
                    }
                }
            };
        sw.execute();
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jPanel2 = new javax.swing.JPanel();
        jPanel3 = new javax.swing.JPanel();
        jLabelIcon = new javax.swing.JLabel();
        jButtonClear = new javax.swing.JButton();
        jPanel1 = new javax.swing.JPanel();
        jCheckBoxConnect = new javax.swing.JCheckBox();
        jLabelFirmwareID = new javax.swing.JLabel();
        jLabelPatch = new javax.swing.JLabel();
        jLabelPatchName = new javax.swing.JLabel();
        jLabelSDCardPresent = new javax.swing.JLabel();
        filler3 = new javax.swing.Box.Filler(new java.awt.Dimension(0, 0), new java.awt.Dimension(32767, 0), new java.awt.Dimension(32767, 0));
        jPanel5 = new javax.swing.JPanel();
        jPanel4 = new axoloti.swingui.target.TargetRTInfo(getDModel());
        filler2 = new javax.swing.Box.Filler(new java.awt.Dimension(0, 0), new java.awt.Dimension(0, 0), new java.awt.Dimension(0, 0));
        jScrollPaneLog = new javax.swing.JScrollPane();
        jTextPaneLog = new javax.swing.JTextPane();
        jPanelProgress = new javax.swing.JPanel();
        jProgressBar1 = new javax.swing.JProgressBar();
        jMenuBar1 = new javax.swing.JMenuBar();
        fileMenu = new axoloti.swingui.menus.FileMenu();
        jMenuEdit = new javax.swing.JMenu();
        jMenuItemCopy = new javax.swing.JMenuItem();
        jMenuBoard = new axoloti.swingui.target.TargetMenu(getDModel());
        windowMenu1 = new axoloti.swingui.menus.WindowMenu();
        helpMenu1 = new axoloti.swingui.menus.HelpMenu();

        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setTitle("Akso Patcher");
        setMinimumSize(new java.awt.Dimension(200, 200));
        setPreferredSize(new java.awt.Dimension(512, 512));
        addWindowListener(new java.awt.event.WindowAdapter() {
            public void windowClosing(java.awt.event.WindowEvent evt) {
                formWindowClosing(evt);
            }
        });
        getContentPane().setLayout(new javax.swing.BoxLayout(getContentPane(), javax.swing.BoxLayout.PAGE_AXIS));

        jPanel2.setPreferredSize(new java.awt.Dimension(272, 0));
        jPanel2.setLayout(new javax.swing.BoxLayout(jPanel2, javax.swing.BoxLayout.LINE_AXIS));

        jPanel3.setBorder(javax.swing.BorderFactory.createEmptyBorder(10, 10, 10, 10));
        jPanel3.setLayout(new javax.swing.BoxLayout(jPanel3, javax.swing.BoxLayout.PAGE_AXIS));

        jLabelIcon.setIcon(new javax.swing.ImageIcon(getClass().getResource("/resources/axoloti_icon.png"))); // NOI18N
        jLabelIcon.setVerticalAlignment(javax.swing.JLabel.CENTER);
        jLabelIcon.setHorizontalAlignment(javax.swing.JLabel.CENTER);
        jPanel3.add(jLabelIcon);

        jButtonClear.setText("Clear Log");
        jButtonClear.setVerticalAlignment(javax.swing.JLabel.CENTER);
        jButtonClear.setHorizontalAlignment(javax.swing.JLabel.CENTER);
        jButtonClear.setMaximumSize(new java.awt.Dimension(128, 32));
        jButtonClear.setMinimumSize(new java.awt.Dimension(128, 32));
        jButtonClear.setPreferredSize(new java.awt.Dimension(128, 32));
        jButtonClear.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButtonClearActionPerformed(evt);
            }
        });

        jPanel2.add(jPanel3);

        jPanel1.setBorder(javax.swing.BorderFactory.createEmptyBorder(3, 3, 3, 3));
        jPanel1.setLayout(new javax.swing.BoxLayout(jPanel1, javax.swing.BoxLayout.PAGE_AXIS));

        jCheckBoxConnect.setText("Connect");
        jCheckBoxConnect.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jCheckBoxConnectActionPerformed(evt);
            }
        });
        jPanel1.add(jCheckBoxConnect);

        jLabelFirmwareID.setText("FirmwareID");
        jPanel1.add(jLabelFirmwareID);

        jLabelPatch.setText("patchIndex");
        jPanel1.add(jLabelPatch);

        jLabelPatchName.setText("patchName");
        jPanel1.add(jLabelPatchName);

        jLabelSDCardPresent.setText("no SDCard");
        jPanel1.add(jLabelSDCardPresent);

        jPanel2.add(jPanel1);
        jPanel2.add(filler3);

        jPanel5.setLayout(new javax.swing.BoxLayout(jPanel5, javax.swing.BoxLayout.PAGE_AXIS));
        jPanel5.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));

        getDModel().getController().addView((axoloti.swingui.target.TargetRTInfo)jPanel4);
//        jPanel5.add(jPanel4);
//        jPanel5.add(filler2);

        jPanel2.add(jPanel5);

        getContentPane().add(jPanel2);

        jScrollPaneLog.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_ALWAYS);
        jScrollPaneLog.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_ALWAYS);
        jScrollPaneLog.setPreferredSize(new java.awt.Dimension(32767, 32767));

        jTextPaneLog.setEditable(false);
        jScrollPaneLog.setViewportView(jTextPaneLog);

        getContentPane().add(jScrollPaneLog);

        jPanelProgress.setMaximumSize(new java.awt.Dimension(605, 32));
        jPanelProgress.setLayout(new javax.swing.BoxLayout(jPanelProgress, javax.swing.BoxLayout.LINE_AXIS));

        jProgressBar1.setAlignmentX(0.0F);
        jProgressBar1.setMaximumSize(new java.awt.Dimension(10000, 32));
        jProgressBar1.setMinimumSize(new java.awt.Dimension(100, 32));
        jProgressBar1.setPreferredSize(new java.awt.Dimension(100, 32));
        jPanelProgress.add(jButtonClear);
        jPanelProgress.add(jProgressBar1);


        getContentPane().add(jPanelProgress);

        fileMenu.setText("File");
        jMenuBar1.add(fileMenu);

        jMenuEdit.setText("Edit");

        jMenuItemCopy.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_C, KeyUtils.CONTROL_OR_CMD_MASK));
        jMenuItemCopy.setText("Copy");
        jMenuEdit.add(jMenuItemCopy);

        jMenuBar1.add(jMenuEdit);

        getDModel().getController().addView((axoloti.swingui.target.TargetMenu)jMenuBoard);
        jMenuBoard.setText("Board");
        jMenuBar1.add(jMenuBoard);
        jMenuBar1.add(windowMenu1);

        helpMenu1.setText("Help");
        jMenuBar1.add(helpMenu1);

        setJMenuBar(jMenuBar1);

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void jButtonClearActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButtonClearActionPerformed
        jTextPaneLog.setText("");
    }//GEN-LAST:event_jButtonClearActionPerformed

    private void jCheckBoxConnectActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jCheckBoxConnectActionPerformed
        if (!jCheckBoxConnect.isSelected()) {
            IConnection connection = getDModel().getConnection();
            if (connection != null) {
                connection.disconnect();
            }
        } else {
            jCheckBoxConnect.setEnabled(false);
            boolean success = CConnection.getConnection().connect(null);
        }
    }//GEN-LAST:event_jCheckBoxConnectActionPerformed



    private void formWindowClosing(java.awt.event.WindowEvent evt) {//GEN-FIRST:event_formWindowClosing
        quit();
    }//GEN-LAST:event_formWindowClosing

    public void openPatchFromURL() {
        String url = JOptionPane.showInputDialog(this, "Enter URL:");
        if (url == null) {
            return;
        }
        try {
            InputStream input = new URL(url).openStream();
            String name = url.substring(url.lastIndexOf('/') + 1, url.length());
            PatchViewSwing.openPatch(name, input);
        } catch (MalformedURLException ex) {
            Logger.getLogger(MainFrame.class.getName()).log(Level.SEVERE, "Invalid URL {0}\n{1}", new Object[]{url, ex});
        } catch (IOException ex) {
            Logger.getLogger(MainFrame.class.getName()).log(Level.SEVERE, "Unable to open URL {0}\n{1}", new Object[]{url, ex});
        }
    }

    public void createNewPatch() {
        PatchModel patchModel = new PatchModel();
        AbstractDocumentRoot documentRoot = new AbstractDocumentRoot();
        patchModel.setDocumentRoot(documentRoot);
        PatchController patchController = patchModel.getController();
        PatchFrame pf = new PatchFrame(patchModel);
        patchController.addView(pf);
        pf.setVisible(true);
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private axoloti.swingui.menus.FileMenu fileMenu;
    private javax.swing.Box.Filler filler2;
    private javax.swing.Box.Filler filler3;
    private axoloti.swingui.menus.HelpMenu helpMenu1;
    private javax.swing.JButton jButtonClear;
    private javax.swing.JCheckBox jCheckBoxConnect;
    private javax.swing.JLabel jLabelFirmwareID;
    private javax.swing.JLabel jLabelIcon;
    private javax.swing.JLabel jLabelPatch;
    private javax.swing.JLabel jLabelPatchName;
    private javax.swing.JLabel jLabelSDCardPresent;
    private javax.swing.JMenuBar jMenuBar1;
    private javax.swing.JMenu jMenuBoard;
    private javax.swing.JMenu jMenuEdit;
    private javax.swing.JMenuItem jMenuItemCopy;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
    private javax.swing.JPanel jPanel5;
    private javax.swing.JPanel jPanelProgress;
    private javax.swing.JProgressBar jProgressBar1;
    private javax.swing.JScrollPane jScrollPaneLog;
    private javax.swing.JTextPane jTextPaneLog;
    private axoloti.swingui.menus.WindowMenu windowMenu1;
    // End of variables declaration//GEN-END:variables

    private void showConnectDisconnect(boolean connect) {
        jCheckBoxConnect.setSelected(connect);
        jCheckBoxConnect.setEnabled(true);
        if (!connect) {
            jLabelPatch.setText(" ");
            v5000c = 0;
            vdd00c = 0;
            jLabelPatch.setText("");
            jLabelSDCardPresent.setText(" ");
        }
    }

    public void quit() {
        if (DocumentWindowList.askCloseAll()) {
            return;
        }

        Preferences.getPreferences().savePrefs();
        dispose();
        System.exit(0);
    }

    void setFirmwareID(String firmwareId) {
        String linkFwId = getDModel().getFirmwareLinkID();
        if (!firmwareId.equals(linkFwId)) {
            if (!TargetModel.getTargetModel().getWarnedAboutFWCRCMismatch()) {
                Logger.getLogger(MainFrame.class.getName()).log(Level.SEVERE, "Firmware CRC mismatch! Please flash the firmware first! " + "Hardware firmware CRC = {0} <> Software CRC = {1}", new Object[]{firmwareId, linkFwId});
                TargetModel.getTargetModel().setWarnedAboutFWCRCMismatch(true);
                SwingUtilities.invokeLater(new Runnable() {
                    @Override
                    public void run() {
                        interactiveFirmwareUpdate();
                    }
                });
            }
        }
    }

    private void showPatchIndex(int patchIndex) {
        String s;
        switch (patchIndex) {
            case -1:
                s = "running /start.bin";
                break;
            case -2:
                s = "running flash patch";
                break;
            case -3:
                s = "running sdcard .bin file";
                break;
            case -4:
                s = "running live patch";
                break;
            case -5:
                s = " ";
                break;
            default:
                s = "running #" + patchIndex;
        }
        jLabelPatch.setText(s);
    }

    private int v5000c = 0;
    private int vdd00c = 0;
    private boolean pwarn = false;

    public void interactiveFirmwareUpdate() {
        int s = JOptionPane.showConfirmDialog(this,
                "A firmware update is required to use this version of the patcher. Would you like to update now?\n"
                + "The hardware will disconnect and enter firmware flash mode.\n"
                + "When flashing completes, it will restart and show the normal fast startup LED blink sequence ending in solid green.\n"
                + "At that point, click the Connect checkbox to reconnect.\n",
                "Firmware update...",
                JOptionPane.YES_NO_OPTION);
        if (s == 0) {
            String fname = Axoloti.firmwareDirectory() + "/flasher/flasher_build/flasher";
            String pname = Axoloti.firmwareDirectory() + "/build/axoloti.bin";
            getDModel().flashUsingSDRam(fname, pname);
        }
    }

    @Override
    public void actionPerformed(ActionEvent e) {
        String cmd = e.getActionCommand();
        if (cmd.startsWith("open:")) {
            String fn = cmd.substring(5);
            if (fn.endsWith(".axb")) {
                PatchBank.openPatchBankEditor(new File(fn));
            } else if (fn.endsWith(".axp") || fn.endsWith(".axs") || fn.endsWith(".axh")) {
                PatchViewSwing.openPatch(new File(fn));
            }
        }
    }

    public void setGrabFocusOnSevereErrors(boolean b) {
//        bGrabFocusOnSevereErrors = b;
    }

    @Override
    public void modelPropertyChange(PropertyChangeEvent evt) {
        if (TargetModel.CONNECTION.is(evt)) {
            IConnection connection = (IConnection)evt.getNewValue();
            boolean isConneced = evt.getNewValue() != null;
            showConnectDisconnect(isConneced);
            if (connection != null) {
                setFirmwareID(connection.getFWID());
            }
        } else if (TargetModel.FIRMWARE_LINK_ID.is(evt)) {
            String linkFwId = (String) evt.getNewValue();
            jLabelFirmwareID.setText("Firmware ID = " + linkFwId);
            Logger.getLogger(MainFrame.class.getName()).log(Level.INFO, "Local firmware CRC: {0}", linkFwId);
            TargetModel.getTargetModel().setWarnedAboutFWCRCMismatch(false);
        } else if (TargetModel.HAS_SDCARD.is(evt)) {
            Boolean b = (Boolean) evt.getNewValue();
            if (b) {
                jLabelSDCardPresent.setText("SDCard mounted");
            } else {
                jLabelSDCardPresent.setText("no SDCard");
            }
        } else if (TargetModel.PATCHINDEX.is(evt)) {
            int i = (Integer)evt.getNewValue();
            showPatchIndex(i);
        } else if (TargetModel.PATCHNAME.is(evt)) {
            String s = (String) evt.getNewValue();
            jLabelPatchName.setText(s);
        }
    }

    private IProgressReporter getProgressReporter() {

        jProgressBar1.setMinimum(0);
        jProgressBar1.setMaximum(16384);
        jProgressBar1.setStringPainted(true);
        return new IProgressReporter() {
            @Override
            public void setProgress(float value) {
                jProgressBar1.setValue((int) (value * 16384));
            }

            @Override
            public void setProgressIndeterminate() {
                throw new UnsupportedOperationException("Not supported yet.");
            }

            @Override
            public void setNote(String note) {
                jProgressBar1.setEnabled(true);
                jProgressBar1.setString(note);
            }

            @Override
            public void setReady() {
                jProgressBar1.setEnabled(false);
                jProgressBar1.setString("Ready");
                jProgressBar1.setValue(0);
            }
        };

    }

}
