import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class Main {
    public static void main(String[] args) {
        if (args.length != 1) {
            System.out.println("Usage: java Main <number of workers>");
            return;
        }

        int n = Integer.parseInt(args[0]);

        if (n < 0) {
            System.out.println("n cannot be smaller than 0");
            return;
        }

        Lock[] tools = new ReentrantLock[n];
        for (int i = 0; i < n; i++) {
            tools[i] = new ReentrantLock();
        }

        Worker[] workers = new Worker[n];
        for (int i = 0; i < n; i++) {
            workers[i] = new Worker(i, i, (i+1)%n, tools[i], tools[(i+1)%n]);
        }

        for (Worker worker : workers) {
            worker.start();
        }
    }

    private static class Worker extends Thread  {
        private final int id;
        private final int firstTool;
        private final int secondTool;
        private final Lock firstLock;
        private final Lock secondLock;

        private Worker(int id, int firstTool, int secondTool, Lock firstLock, Lock secondLock) {
            this.id = id;
            this.firstTool = firstTool;
            this.secondTool = secondTool;
            this.firstLock = firstLock;
            this.secondLock = secondLock;
        }

        @Override
        public void run() {
            while (true) {
                Actions.tryToTakeTool(id, firstTool);
                if (firstLock.tryLock()) {
                    Actions.takeToolSuccessful(id, firstTool);
                    try {
                        Actions.tryToTakeTool(id, secondTool);
                        if (secondLock.tryLock()) {
                            try {
                                Actions.takeToolSuccessful(id, secondTool);
                                Actions.work(id);
                                Actions.putTool(id, secondTool);
                            } finally {
                                secondLock.unlock();
                            }
                        } else {
                            Actions.takeToolFail(id, secondTool);
                        }
                        Actions.putTool(id, firstTool);
                    } finally {
                        firstLock.unlock();
                    }
                } else {
                    Actions.takeToolFail(id, firstTool);
                }
            }
        }
    }
}