package cz.cvut.fel.pjv;

import java.util.ArrayList;

public class SequenceStats {
    private ArrayList<Double> numbers;

    public SequenceStats() {
        this.numbers = new ArrayList<>();
    }

    public void addNumber(double number) {
        if (numbers.size() == 10) {
            numbers.clear();
        }
        numbers.add(number);
    }

    public void clear() {
        numbers.clear();
    }

    public double getAverage() {
        double sum = 0;
        for (int i = 0; i < numbers.size(); i++) {
            sum += numbers.get(i);
        }
        return sum / numbers.size();
    }

    public double getStandardDeviation() {
        double avg = getAverage();

        double sum = 0;
        for (int i = 0; i < numbers.size(); i++) {
            sum += Math.pow(numbers.get(i) - avg, 2);
        }
        double variance = sum / numbers.size();

        return Math.sqrt(variance);
    }

    public int getCount() {
        return numbers.size();
    }

    public String getFormattedStatistics() {
        String stats = "%2d %.3f %.3f";
        return String.format(stats, numbers.size(), getAverage(), getStandardDeviation());
    }
}
