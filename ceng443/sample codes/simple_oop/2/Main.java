// static polymorphism 

import java.util.*;

public class Main {
    public static String classify(Set<?> s) {
        return "Set";
    }
    public static String classify(HashSet<?> h) {
        return "HashSet";
    }
    public static String classify(List<?> l) {
        return "List";
    }
    public static String classify(Collection<?> c) {
        return "Unknown Collection";
    }
    public static void main(String[] args) {
        HashSet<?> h = new HashSet<Integer>();
        Collection<?>[] tests = {
                new HashSet<String>(),
                new ArrayList<Character>(),
                new HashMap<Integer,String>().values()
        };
        for(int i = 0; i < tests.length; i++) {
            System.out.println(classify(tests[i]));
        }
        System.out.println(classify(h));
    }
}
