import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.stream.Stream;

public class Test {
    
    public static void main(String[] args){
        // check input format
        if (args.length != 2) {
            System.out.println("The input format is wrong");
            return;
        }

        String fileName = args[0];
        Integer questionNo = Integer.parseInt(args[1]);

        try (BufferedReader br = new BufferedReader(new FileReader(fileName))) {

            Stream<String> lines = br.lines().skip(1);

            switch (questionNo) {
                case 1:
                    System.out.println(q1(lines));
                    break;
                case 2:
                    System.out.println(q2(lines));
                    break;
                case 3:
                    System.out.println(q3(lines));
                    break;
                default:
                    System.out.println("Invalid question no");
                    break;
            }

        }
        catch (IOException ex) {
            ex.printStackTrace();
        }
    }


    private static int q1(Stream<String> lines) {
        // total quantity of products purchased by customer whose names start with 'A'
        return lines.filter(line -> line.startsWith("A"))
        .mapToInt(Test::totalQuantity)
        .sum();
    }

    private static double q2(Stream<String> lines) {
        // price of the most expensive product sold
        return lines.flatMap(line -> {
            String[] parts = line.split(",");

            List<String> prices = new ArrayList<>();

            for(int i = 3; i < parts.length; i+=2) {
                if(!parts[i].isEmpty()) {
                    prices.add(parts[i]);
                }
            }

            return prices.stream();
        })
        .mapToDouble(Double::parseDouble)
        .max().orElse(0);
    }

    private static String q3(Stream<String> lines) {
        // date of the highest paid purchase by a customer
        return lines.max(Comparator.comparing(Test::totalCost))
        .map(line -> line.split(",")[2])
        .orElse("");
    }

    private static double totalCost(String line) {
        double price = 0;

        String[] parts = line.split(",");
        for (int i = 3; i < parts.length; i+=2) {
            if(!parts[i].isEmpty()) {
                price += Double.parseDouble(parts[i]);
            }
        }
        return price;
    }

    private static int totalQuantity(String line) {
        int quantity = 0;

        String[] parts = line.split(",");

        for (int i = 4; i < parts.length; i+=2) {
            if(!parts[i].isEmpty()) {
                quantity += Integer.parseInt(parts[i]);
            }
        }

        return quantity;
    }


}
