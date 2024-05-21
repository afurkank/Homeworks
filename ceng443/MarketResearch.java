import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.*;
import java.util.stream.Collectors;
import java.util.stream.Stream;

public class MarketResearch {
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
                case 5:
                    System.out.println(leastPopularProductAfter2000(lines));
                    break;
                case 6:
                    System.out.println(mostPopularProductAmongTeens(lines));
                    break;
                case 7:
                    System.out.println(mostInflatedProduct(lines));
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
                .mapToInt(MarketResearch::totalQuantity)
                .sum();
    }

    private static double priceOfMostExpensiveProduct(Stream<String> lines) {
        return lines.flatMap(line -> Arrays.stream(line.split(",")))
                .filter(s -> s.matches("\\d+(\\.\\d+)?"))
                .mapToDouble(Double::parseDouble)
                .max().orElse(0);
    }

    private static String dateOfHighestPaidPurchase(Stream<String> lines) {
        return lines.max(Comparator.comparingDouble(MarketResearch::totalCost))
                .map(line -> line.split(",")[2])
                .orElse("");
    }

    private static String mostPopularProductBefore2000(Stream<String> lines) {
        return lines.filter(line -> line.split(",")[2].compareTo("2000-01-01") < 0)
                .flatMap(line -> Arrays.stream(line.split(",")).skip(3).filter(s -> !s.isEmpty()))
                .collect(Collectors.groupingBy(s -> s, Collectors.counting()))
                .entrySet().stream().max(Map.Entry.comparingByValue())
                .map(Map.Entry::getKey).orElse("");
    }

    private static String leastPopularProductAfter2000(Stream<String> lines) {
        return lines.filter(line -> line.split(",")[2].compareTo("2000-01-01") >= 0)
                .flatMap(line -> Arrays.stream(line.split(",")).skip(3).filter(s -> !s.isEmpty()))
                .collect(Collectors.groupingBy(s -> s, Collectors.counting()))
                .entrySet().stream().min(Map.Entry.comparingByValue())
                .map(Map.Entry::getKey).orElse("");
    }

    private static String mostPopularProductAmongTeens(Stream<String> lines) {
        Map<String, List<Integer>> productAges = new HashMap<>();
        lines.forEach(line -> {
            String[] parts = line.split(",");
            int age = Integer.parseInt(parts[1]);
            if (age >= 13 && age <= 19) {
                for (int i = 3; i < parts.length; i += 2) {
                    if (!parts[i].isEmpty()) {
                        productAges.computeIfAbsent(parts[i], k -> new ArrayList<>()).add(age);
                    }
                }
            }
        });
        return productAges.entrySet().stream()
                .min(Comparator.comparingDouble(e -> e.getValue().stream().mapToInt(Integer::intValue).average().orElse(0)))
                .map(Map.Entry::getKey).orElse("");
    }

    private static String mostInflatedProduct(Stream<String> lines) {
        Map<String, Double> oldestPrices = new HashMap<>();
        Map<String, Double> newestPrices = new HashMap<>();
        lines.forEach(line -> {
            String[] parts = line.split(",");
            String date = parts[2];
            for (int i = 3; i < parts.length; i += 2) {
                if (!parts[i].isEmpty()) {
                    double price = Double.parseDouble(parts[i]);
                    String product = parts[i];
                    oldestPrices.putIfAbsent(product, price);
                    newestPrices.put(product, price);
                }
            }
        });
        return oldestPrices.entrySet().stream()
                .max(Comparator.comparingDouble(e -> (newestPrices.get(e.getKey()) - e.getValue()) / e.getValue()))
                .map(Map.Entry::getKey).orElse("");
    }

    private static int totalQuantity(String line) {
        String[] parts = line.split(",");
        int quantity = 0;
        for (int i = 4; i < parts.length; i += 2) {
            if (!parts[i].isEmpty()) {
                quantity += Integer.parseInt(parts[i]);
            }
        }
        return quantity;
    }

    private static double totalCost(String line) {
        String[] parts = line.split(",");
        double total = 0;
        for (int i = 3; i < parts.length; i += 2) {
            if (!parts[i].isEmpty()) {
                total += Double.parseDouble(parts[i]) * Integer.parseInt(parts[i + 1]);
            }
        }
        return total;
    }
}
