import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

class Worker extends Thread {
    private final int id;
    private final Lock firstTool;
    private final Lock secondTool;
    private final int firstToolId;
    private final int secondToolId;

    public Worker(int id, Lock firstTool, int firstToolId, Lock secondTool, int secondToolId) {
        this.id = id;
        this.firstTool = firstTool;
        this.firstToolId = firstToolId;
        this.secondTool = secondTool;
        this.secondToolId = secondToolId;
    }

    private void acquireTools() {
        Actions.tryToTakeTool(id, firstToolId);
        firstTool.lock();
        Actions.takeToolSuccessful(id, firstToolId);

        Actions.tryToTakeTool(id, secondToolId);
        secondTool.lock();
        Actions.takeToolSuccessful(id, secondToolId);
    }

    private void releaseTools() {
        Actions.putTool(id, firstToolId);
        firstTool.unlock();

        Actions.putTool(id, secondToolId);
        secondTool.unlock();
    }

    @Override
    public void run() {
        while (true) {
            acquireTools();
            Actions.work(id);
            releaseTools();
            // Optional sleep to simulate job time (can be removed for simplicity)
            try {
                Thread.sleep((long) (Math.random() * 10000));
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
            }
        }
    }
}

public class ConstructionSite {
    public static void main(String[] args) {
        if (args.length != 1) {
            System.err.println("Usage: java ConstructionSite <number of workers>");
            System.exit(1);
        }

        int n = Integer.parseInt(args[0]);
        if (n <= 0) {
            System.err.println("Number of workers must be positive.");
            System.exit(1);
        }

        Lock[] tools = new ReentrantLock[n];
        for (int i = 0; i < n; i++) {
            tools[i] = new ReentrantLock();
        }

        Worker[] workers = new Worker[n];
        for (int i = 0; i < n; i++) {
            int firstTool = i;
            int secondTool = (i + 1) % n;
            // Determine the correct locking order to avoid deadlock
            Lock first = tools[firstTool];
            Lock second = tools[secondTool];

            // Make sure the lower numbered tool is always locked first
            if (firstTool > secondTool) {
                Lock temp = first;
                first = second;
                second = temp;
            }

            workers[i] = new Worker(i, first, firstTool, second, secondTool);
        }

        for (Worker worker : workers) {
            worker.start();
        }
    }
}
