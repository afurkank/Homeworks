// overloaded constructors 

class Car {
    String color;
    String brand;
    int weight;
    
    Car() {
        this("white", "fhx", 920);
    }
    Car(String brand, int weight) {
        this("white", brand, weight);
    }
    Car(String color, String brand, int weight) {
        this.color=color; 
        this.brand=brand;
        this.weight=weight;
    }
}
public class Main {
    public static void main(String[] args) {
        Car c=new Car("red", "togg", 1200);
        System.out.println(c.color);
        System.out.println(c.brand);
        System.out.println(c.weight);
    }
}
