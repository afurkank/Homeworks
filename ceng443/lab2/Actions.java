import java.util.concurrent.ThreadLocalRandom;
import java.lang.Thread;

public class Actions {
    private static void action(int min_time, int max_time, String msg) {
        assert min_time <= max_time;

        if (max_time > 0) {
            try {
                int wait_time;
                if (min_time == max_time) {
                    wait_time = max_time;
                } else {
                    wait_time = ThreadLocalRandom.current().nextInt(min_time, max_time + 1);
                }
                Thread.sleep(wait_time);
            } catch (InterruptedException e) {
            }
        }

        System.out.println(msg + " (on Thread " + Thread.currentThread().getId() + ")");
    }

    static void tryToTakeTool(int worker_number, int tool_number) {
        action(20, 40, "Worker " + worker_number + " tries to take tool " + tool_number);

    }

    static void takeToolSuccessful(int worker_number, int tool_number) {
        action(0, 0, "Worker " + worker_number + " takes tool " + tool_number + " successfully");
    }

    static void takeToolFail(int worker_number, int tool_number) {
        action(0, 0, "Worker " + worker_number + " fails to take tool " + tool_number);
    }

    static void putTool(int worker_number, int tool_number) {
        action(50, 50, "Worker " + worker_number + " puts tool " + tool_number + " back");

    }

    static void work(int worker_number) {
        action(100, 200, "Worker " + worker_number + " completes the job");
    }

}
