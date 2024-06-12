package lab3;

import java.io.IOException;
import java.lang.reflect.Array;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Main {
    public static void main(String[] args) {

        if (args.length != 2) {
            System.out.println("Usage is 'java YourProgramName csv_file_name question_no'");
            return;
        }

        String fileName = args[0];
        String questionNo = args[1];

        Stream<String> lines;
        try {
            lines = Files.lines(Paths.get(fileName)).skip(1);
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        switch (questionNo) {
            case "1":
                System.out.println(q1(lines));
                break;
            case "2":
                System.out.println(q2(lines));
                break;
            case "3":
                System.out.println(q3(lines));
                break;
            default:
                System.out.println("Provide a question number between 1 and 7");
        }

    }

    private static int q1(Stream<String> lines) {
        // What is the total quantity of products purchased by the
        // customers whose names start with 'A'?
        return lines
                .filter(l -> l.split(",")[0].startsWith("A"))
                .flatMapToInt(l -> {
                    String[] data = l.split(",");

                    List<Integer> quantities = new ArrayList<>();

                    for (int i = 4; i < data.length; i+=2) {
                        if (!data[i].isEmpty()) {
                            quantities.add(Integer.parseInt(data[i]));
                        }
                    }

                    return quantities.stream().mapToInt(Integer::intValue);
                })
                .sum();
    }

    private static double q2(Stream<String> lines) {
        // price of the most expensive product sold
        return lines
                .flatMapToDouble(l -> {
                    String[] data = l.split(",");

                    List<Double> quantities = new ArrayList<>();

                    for (int i = 3; i < data.length; i+=2) {
                        if (!data[i].isEmpty()) {
                            quantities.add(Double.parseDouble(data[i]));
                        }
                    }

                    return quantities.stream().mapToDouble(Double::doubleValue);
        })
                .max()
                .orElse(0.0f);
    }

    private static String q3(Stream<String> lines) {
        // What was the date of the highest paid purchase by a customer?
        return lines
                .max(Comparator.comparing(l -> {
                    String[] data = l.split(",");

                    double sum = 0.0f;

                    for (int i = 3; i < data.length; i+=2) {
                        if (!data[i].isEmpty()) {
                            sum += Double.parseDouble(data[i]) * Integer.parseInt(data[i+1]);
                        }
                    }

                    return sum;
                }))
                .map(l -> l.split(",")[2])
                .orElse("no data");
    }

    private static String q4(Stream<String> lines) {
        // What was the most popular product before 2000, in terms of
        // total number of purchases whose include that item?
        return "";
    }

}