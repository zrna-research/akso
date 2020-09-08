package axoloti.shell;

import axoloti.Axoloti;
import axoloti.utils.OSDetect;
import axoloti.job.IJob;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author jtaelman
 */
public class ShellTask {

    final private String workingDirectory;
    final private String exec;
    final private String[] environment;

    public ShellTask(String workingDirectory, String exec, String[] environment) {
        this.workingDirectory = workingDirectory;
        this.exec = exec;
        this.environment = environment;
    }

    public IJob getJob() {
        return (ctx) -> {
            try {
                run(workingDirectory, exec, environment);
            } catch (Exception ex) {
                ctx.reportException(ex);
            }
        };
    }

    private static class StreamHandlerThread implements Runnable {

        private final InputStream in;

        StreamHandlerThread(InputStream in) {
            this.in = in;
        }

        @Override
        public void run() {
            BufferedReader br = new BufferedReader(new InputStreamReader(in));
            String line;
            try {
                line = br.readLine();
                while (line != null) {
                    line = line.replaceAll("\u001B\\[[;\\d]*m", "");
                    if (line.contains("error")) {
                        Logger.getLogger(ShellTask.class.getName()).log(Level.SEVERE, "{0}", line);
                    } else {
                        Logger.getLogger(ShellTask.class.getName()).log(Level.INFO, "{0}", line);
                    }
                    line = br.readLine();
                }
            } catch (IOException ex) {
                Logger.getLogger(ShellTask.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
    }

    public static String[] getEnvironment() {
        ArrayList<String> list = new ArrayList<>();
        Map<String, String> env = System.getenv();
        for (String v : env.keySet()) {
            list.add((v + "=" + env.get(v)));
        }

        String platform = "linux";
        if (OSDetect.getOS() == OSDetect.OS.WIN) {
            platform = "win";
        } else if (OSDetect.getOS() == OSDetect.OS.MAC) {
            platform = "mac";
        }
        list.add("firmware_directory=" + Axoloti.firmwareDirectory());
        list.add("external=" + Axoloti.externalDirectory());
        list.add("platform=" + platform);
        list.add("mac_prefix=" + Axoloti.macLibraryDirectory());


        String vars[] = new String[list.size()];
        list.toArray(vars);
        return vars;
    }

    private File getWorkingDir() {
        return new File(Axoloti.buildDirectory());
    }

    private final CompletableFuture<Boolean> success = new CompletableFuture<>();

    private void run(
            String workingDirectory,
            String exec,
            String[] environment) throws ExecutionFailedException {
        Runtime runtime = Runtime.getRuntime();
        try {
            Process p1 = runtime.exec(exec, environment, new File(workingDirectory));
            Thread thd_out = new Thread(new StreamHandlerThread(p1.getInputStream()));
            thd_out.start();
            Thread thd_err = new Thread(new StreamHandlerThread(p1.getErrorStream()));
            thd_err.start();
            p1.waitFor();
            thd_out.join();
            thd_err.join();
            success.complete(p1.exitValue() == 0);
            if (p1.exitValue() != 0) {
                Logger.getLogger(ShellTask.class.getName()).log(Level.SEVERE, "shell task failed, exit value: {0}", p1.exitValue());
                throw new ExecutionFailedException();
            }
        } catch (InterruptedException | IOException ex) {
            Logger.getLogger(ShellTask.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public boolean isSuccess() {
        try {
            return success.get();
        } catch (InterruptedException | ExecutionException ex) {
            throw new IllegalStateException(ex);
        }
    }


}
