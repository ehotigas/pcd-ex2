import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

public class TrafficController {
    private final ReentrantLock lock = new ReentrantLock();
    private final Condition left = lock.newCondition();
    private final Condition right = lock.newCondition();
    private int leftCount = 0;
    private int rightCount = 0;
    private boolean control = false;

    public void enterLeft() {
        lock.lock();
        try {
            leftCount++;
            while (control) {
                left.await();
            }
            leftCount--;
            control = true;
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        } finally {
            lock.unlock();
        }
    }

    public void enterRight() {
        lock.lock();
        try {
            rightCount++;
            while (control) {
                right.await();
            }
            rightCount--;
            control = true;
        } catch (InterruptedException e) {
            Thread.currentThread().interrupt();
        } finally {
            lock.unlock();
        }
    }

    public void leaveLeft() {
        lock.lock();
        try {
            control = false;
            if (rightCount > 0) {
                right.signal();
            } else {
                left.signalAll();
            }
        } finally {
            lock.unlock();
        }
    }

    public void leaveRight() {
        lock.lock();
        try {
            control = false;
            if (leftCount > 0) {
                left.signal();
            } else {
                right.signalAll();
            }
        } finally {
            lock.unlock();
        }
    }
}