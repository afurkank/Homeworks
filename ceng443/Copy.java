import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.*;
import java.util.stream.Collector;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class Copy {
    public static void main(String[] args) {
        if (args.length != 2) {
            System.out.println("Usage: java MarketResearch <csv_file_name> <question_no>");
            return;
        }

        String fileName = args[0];
        int questionNo = Integer.parseInt(args[1]);

        try (BufferedReader br = new BufferedReader(new FileReader(fileName))) {
            Stream<String> lines = br.lines().skip(1); // Skip header
            switch (questionNo) {
                case 1:
                    System.out.println(totalQuantityByNamesStartingWithA(lines));
                    break;
                case 2:
                    System.out.println(priceOfMostExpensiveProduct(lines));
                    break;
                case 3:
                    System.out.println(dateOfHighestPaidPurchase(lines));
                    break;
                case 4:
                    System.out.println(mostPopularProductBefore2000(lines));
                    break;
                default:
                    System.out.println("Invalid question number");
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private static int totalQuantityByNamesStartingWithA(Stream<String> lines) {
        return lines.filter(line -> line.startsWith("A"))
        .mapToInt(Copy::totalQuantity)
        .sum();
    }

    private static double priceOfMostExpensiveProduct(Stream<String> lines) {
        return lines.flatMap(line -> {
            String[] parts = line.split(",");

            List<String> prices = new ArrayList<>();

            for(int i = 3; i < parts.length; i+=2) {
                if(!parts[i].isEmpty()){
                    prices.add(parts[i]);
                }
            }
            return prices.stream();
        })
        .mapToDouble(Double::parseDouble)
        .max().orElse(0);
    }

    private static String dateOfHighestPaidPurchase(Stream<String> lines) {
        return lines.max(Comparator.comparing(Copy::totalPrice))
        .map(line -> line.split(",")[2])
        .orElse("");
    }

    private static String mostPopularProductBefore2000(Stream<String> lines) {
        Map <String, Integer> productQuantities = lines
        .filter(line -> line.split(",")[2].compareTo("2000-01-01") < 0)
        .collect(Collectors.toMap("bread", line.split(",")[4] if !line.split(",")[4].isEmpty() else 0,
        ))
        
    }

    private static int totalQuantity(String line){
        String[] parts = line.split(",");
        
        int quantity = 0;
        for (int i = 4; i < parts.length; i+=2) {
            if(!(parts[i].isEmpty())){
                quantity += Integer.parseInt(parts[i]);
            }
        }
        return quantity;
    }

    private static double totalPrice(String line) {
        String[] parts = line.split(",");

        double price = 0;

        for(int i = 3; i < parts.length; i+=2) {
            if(!(parts[i].isEmpty())) {
                price += Double.parseDouble(parts[i]);
            }
        }

        return price;
    }
}
