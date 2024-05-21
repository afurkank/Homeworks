
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.stream.Collector;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.util.stream.Stream;

class Purchase {
    String customerName;
    Integer customerAge;
    String purchaseDate;

    Float breadPrice;
    Integer breadQuantity;
    
    Float milkPrice;
    Integer milkQuantity;
    
    Float eggPrice;
    Integer eggQuantity;
    
    Float potatoPrice;
    Integer potatoQuantity;
    
    Float tomatoPrice;
    Integer tomatoQuantity;

    Float totalPrice;
    Integer totalQuantity;

    Purchase(String name, Integer age, String date,
        Float breadP, Integer breadQ,
        Float milkP, Integer milkQ,
        Float eggP, Integer eggQ,
        Float potatoP, Integer potatoQ,
        Float tomatoP, Integer tomatoQ
    ){
        customerName = name;
        customerAge = age;
        purchaseDate = date;

        breadPrice = breadP != null ? breadP : 0.0f;
        milkPrice = milkP != null ? milkP : 0.0f;
        eggPrice = eggP != null ? eggP : 0.0f;
        potatoPrice = potatoP != null ? potatoP : 0.0f;
        tomatoPrice = tomatoP != null ? tomatoP : 0.0f;
        
        breadQuantity = breadQ != null ? breadQ : 0;
        milkQuantity = milkQ != null ? milkQ : 0;
        eggQuantity = eggQ != null ? eggQ : 0;
        potatoQuantity = potatoQ != null ? potatoQ : 0;
        tomatoQuantity = tomatoQ != null ? tomatoQ : 0;
        
        totalPrice = breadP+milkP+eggP+potatoP+tomatoP;
        totalQuantity = breadQ+milkQ+eggQ+potatoQ+tomatoQ;
    }
}

public class Main {
    public static void main(String[] args) throws FileNotFoundException {
        if (args.length < 2) {
            System.err.println("Usage: java Main <csv file> <question number>");
            return;
        }

        String csvFileName = args[0];
        Integer questionNo = Integer.parseInt(args[1]);

        BufferedReader br = new BufferedReader(
        new InputStreamReader(
        new FileInputStream(
        new File(csvFileName))));

        Function<String, Purchase> mapToPurchase = (line) -> {
            String[] p = line.split(",");
            return new Purchase(
                p[0], // customer name
                parseInteger(p, 1), // customer age
                p[2], // purchase date
                parseFloat(p, 3), parseInteger(p, 4), // bread info
                parseFloat(p, 5), parseInteger(p, 6), // milk info
                parseFloat(p, 7), parseInteger(p, 8), // egg info
                parseFloat(p, 9), parseInteger(p, 10), // potato info
                parseFloat(p, 11), parseInteger(p, 12)); // tomato info
        };

        List<Purchase> prc = br.lines().skip(1).map(mapToPurchase).collect(Collectors.toList());

        switch (questionNo) {
            case 1:
                // total quantity of products purchased by customer whose names start with 'A'
                System.out.println(prc
                .stream()
                .filter(p -> p.customerName.startsWith("A"))
                .flatMapToInt(p -> IntStream.of(p.breadQuantity, p.milkQuantity, p.eggQuantity, p.potatoQuantity, p.tomatoQuantity))
                .sum());
                break;
            case 2:
                // price of the most expensive product sold
                System.out.println(prc
                .stream()
                .flatMap(p -> Stream.of(p.breadPrice, p.milkPrice, p.eggPrice, p.potatoPrice, p.tomatoPrice))
                .max(Comparator.naturalOrder())
                .orElse(0.0f));
                break;
            case 3:
                // date of the highest paid purchase by a customer
                System.out.println(prc
                .stream()
                .max(Comparator.comparing(p -> p.breadPrice + p.milkPrice + p.eggPrice + p.potatoPrice + p.tomatoPrice))
                .map(p -> p.purchaseDate)
                .orElse("No purchases found"));
                break;
            case 4:
                // most popular product before 2000 in terms of total number of purchases
                Map<String, Integer> productQuantities = prc.stream()
                    .filter(p -> Integer.parseInt(p.purchaseDate.substring(0, 4)) < 2000)
                    .collect(Collectors.toMap(
                        p -> "bread", p -> p.breadQuantity, Integer::sum,
                        () -> new HashMap<String, Integer>() {{
                            put("bread", 0);
                            put("milk", 0);
                            put("egg", 0);
                            put("potato", 0);
                            put("tomato", 0);
                        }}
                    ));
                
                productQuantities.merge("milk", prc.stream().filter(p -> Integer.parseInt(p.purchaseDate.substring(0, 4)) < 2000).mapToInt(p -> p.milkQuantity).sum(), Integer::sum);
                productQuantities.merge("egg", prc.stream().filter(p -> Integer.parseInt(p.purchaseDate.substring(0, 4)) < 2000).mapToInt(p -> p.eggQuantity).sum(), Integer::sum);
                productQuantities.merge("potato", prc.stream().filter(p -> Integer.parseInt(p.purchaseDate.substring(0, 4)) < 2000).mapToInt(p -> p.potatoQuantity).sum(), Integer::sum);
                productQuantities.merge("tomato", prc.stream().filter(p -> Integer.parseInt(p.purchaseDate.substring(0, 4)) < 2000).mapToInt(p -> p.tomatoQuantity).sum(), Integer::sum);

                String mostPopularProduct = productQuantities.entrySet().stream()
                .max(Map.Entry.comparingByValue())
                .map(Map.Entry::getKey)
                .orElse("No purchases found");

                System.out.println(mostPopularProduct);
                break;
            default:
                break;
        }
    }
    private static Integer parseInteger(String[] array, int index) {
        if (index >= array.length || array[index].isEmpty()) {
            return 0;
        }
        try {
            return Integer.parseInt(array[index]);
        } catch (NumberFormatException e) {
            return 0;
        }
    }

    private static Float parseFloat(String[] array, int index) {
        if (index >= array.length || array[index].isEmpty()) {
            return 0.0f;
        }
        try {
            return Float.parseFloat(array[index]);
        } catch (NumberFormatException e) {
            return 0.0f;
        }
    }
}

/* case 2:
                
                break;
            case 3:
                
                break;
            case 4:
                
                break;
            case 5:
                
                break;
            case 6:
                
                break;
            case 7:
                
                break; */


               /* System.out.println(p[0]);
                System.out.println(p[1]);
                System.out.println(p[2]);
                System.out.println(p[3]);
                System.out.println(p[4]);
                System.out.println(p[5]);
                System.out.println(p[6]);
                System.out.println(p[7]);
                System.out.println(p[8]);
                System.out.println(p[9]);
                System.out.println(p[10]);
                System.out.println(p[11]);
                System.out.println(p[12]); */