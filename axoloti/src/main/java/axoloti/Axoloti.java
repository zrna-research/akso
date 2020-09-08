/**
 * Copyright (C) 2013, 2014, 2015 Johannes Taelman
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
package axoloti;

import axoloti.job.IJobContext;
import axoloti.job.JobContext;
import axoloti.objectlibrary.AxoObjects;
import axoloti.objectlibrary.AxolotiLibrary;
import axoloti.preferences.Preferences;
import axoloti.swingui.MainFrame;
import axoloti.target.TargetModel;
import axoloti.utils.OSDetect;
import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.UIManager;
import javax.swing.UnsupportedLookAndFeelException;
import java.nio.file.Paths;


/**
 *
 * @author Johannes Taelman
 */
public class Axoloti {
    public static void main(final String[] args) {
        try {
            if(OSDetect.getOS() == OSDetect.OS.LINUX) {
                UIManager.setLookAndFeel("com.sun.java.swing.plaf.gtk.GTKLookAndFeel");
            } else {
                UIManager.setLookAndFeel(
                    UIManager.getSystemLookAndFeelClassName());
            }
        }
        catch (UnsupportedLookAndFeelException e) {
        }
        catch (ClassNotFoundException e) {
        }
        catch (InstantiationException e) {
        }
        catch (IllegalAccessException e) {
        }
        System.setProperty("line.separator", "\n");
        new File(buildDirectory()).mkdirs();

        handleCommandLine(args);
    }

    private static URI jarURI() {
        try {
            return Axoloti.class.getProtectionDomain()
                .getCodeSource().getLocation().toURI();
        }
        catch(URISyntaxException e) {
            throw new Error(e);
        }
    }

    public static boolean isDevelopmentBuild() {
        return jarURI().toString().endsWith("dist" + File.separator + "Axoloti.jar");
    }

    public static String rootDirectory() {
        if(isDevelopmentBuild()) {
            return new File(jarURI()).getParentFile().getParentFile().getPath();
        }
        else {
            return new File(jarURI()).getParentFile().getPath();
        }
    }

    public static String macLibraryDirectory() {
        return System.getProperty("user.home") + File.separator + "Library" + File.separator + "Akso";
    }

    public static String buildDirectory() {
        if(OSDetect.getOS() == OSDetect.OS.MAC) {
            return macLibraryDirectory() + File.separator + "build";
        }
        else {
            return rootDirectory() + File.separator + "build";
        }
    }

    public static String defaultObjectLibraryDirectory() {
        if(OSDetect.getOS() == OSDetect.OS.MAC) {
            return macLibraryDirectory();
        }
        else {
            return rootDirectory();
        }
    }

    public static String externalDirectory() {
        if(isDevelopmentBuild()) {
            return new File(jarURI()).getParentFile().getParentFile().getParentFile().getPath() + File.separator + "external";
        }
        else {
            return rootDirectory() + File.separator + "external";
        }
    }

    public static String firmwareDirectory() {
        return rootDirectory() + File.separator + "firmware";
    }

    public static boolean isDeveloper() {
        return false;
    }

    private static void handleCommandLine(final String args[]) {
        boolean cmdLineOnly = false;
        boolean cmdRunAllTest = false;
        boolean cmdRunPatchTest = false;
        boolean cmdRunObjectTest = false;
        boolean cmdRunFileTest = false;
        boolean cmdRunUpgrade = false;
        boolean stopOnFirstFail = false;
        String cmdFile = null;
        for (int i = 0; i < args.length; i++) {
            String arg = args[i];
            if (arg.equalsIgnoreCase("-exitOnFirstFail")) {
                stopOnFirstFail = true;
            }

            // exclusive options
            if (arg.equalsIgnoreCase("-runAllTests")) {
                cmdLineOnly = true;
                cmdRunAllTest = true;
            } else if (arg.equalsIgnoreCase("-runPatchTests")) {
                cmdLineOnly = true;
                cmdRunPatchTest = true;
            } else if (arg.equalsIgnoreCase("-runObjTests")) {
                cmdLineOnly = true;
                cmdRunObjectTest = true;
            } else if (arg.equalsIgnoreCase("-runTest")) {
                cmdLineOnly = true;
                cmdRunFileTest = true;
                if (i + 1 < args.length) {
                    cmdFile = args[i + 1];
                } else {
                    System.err.println("-runTest patchname/directory : missing file/dir");
                    System.exit(-1);
                }
            } else if (arg.equalsIgnoreCase("-runUpgrade")) {
                cmdLineOnly = true;
                cmdRunUpgrade = true;
                if (i + 1 < args.length) {
                    cmdFile = args[i + 1];
                } else {
                    System.err.println("-runUpgrade patchname/directory : missing file/dir");
                    System.exit(-1);
                }
            } else if (arg.equalsIgnoreCase("-help")) {
                System.out.println("Axoloti "
                                   + " [-runAllTests|-runPatchTests|-runObjTests] "
                                   + " [-runTest patchfile|dir]"
                                   + " [-runUpgrade patchfile|dir]"
                                   + " [-exitOnFirstFail");
                System.exit(0);
            }
        }

        if (cmdLineOnly) {
            try {
                MainFrame frame = new MainFrame(args, TargetModel.getTargetModel());
                Tests tests = new Tests();
                AxoObjects objs = new AxoObjects();
                IJobContext progress = new JobContext();
                AxoObjects.loadAxoObjects(progress);

                System.out.println("Axoloti cmd line initialised");
                int exitCode = 0;
                List<String> failedPatches = null;
                if (cmdRunAllTest) {
                    failedPatches = tests.runAllTests(stopOnFirstFail);
                    exitCode = failedPatches.isEmpty() ? 0 : -1;
                } else if (cmdRunPatchTest) {
                    failedPatches = tests.runPatchTests(stopOnFirstFail);
                    exitCode = failedPatches.isEmpty() ? 0 : -1;
                } else if (cmdRunObjectTest) {
                    failedPatches = tests.runObjectTests(stopOnFirstFail);
                    exitCode = failedPatches.isEmpty() ? 0 : -1;
                } else if (cmdRunFileTest) {
                    exitCode = tests.runFileTest(cmdFile) ? 0 : -1;
                } else if (cmdRunUpgrade) {
                    exitCode = tests.runFileUpgrade(cmdFile) ? 0 : -1;
                }

                if (failedPatches != null && !failedPatches.isEmpty()) {
                    System.out.println("List of failing patches:");
                    for (String patchname : failedPatches) {
                        System.out.println(patchname);
                    }
                }

                System.out.println("Axoloti cmd line complete");
                System.exit(exitCode);
            } catch (Exception e) {
                e.printStackTrace();
                System.exit(-2);
            }
        } else {
            try {
                MainFrame frame = new MainFrame(args, TargetModel.getTargetModel());
                frame.setVisible(true);
                frame.initLogging();
                frame.initLibrary();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
