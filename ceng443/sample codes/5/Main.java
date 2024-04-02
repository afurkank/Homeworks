// upcasting & downcasting 

abstract class SweetProducer { 
    public abstract void produceSweet();  
} 

class CakeProducer extends SweetProducer { 
    @Override 
    public void produceSweet() { 
        System.out.println("Cake produced"); 
    } 
} 
class ChocolateProducer extends SweetProducer { 
    @Override 
    public void produceSweet() { 
        System.out.println("Chocolate produced"); 
    } 
    // spesific method 
    public void moreCacao() {
        System.out.println("More cacao added"); 
    } 
} 
class CookieProducer extends SweetProducer { 
    @Override  
    public void produceSweet() { 
        System.out.println("Cookie produced");  
    }
}

public class Main {
    public static void main(String[] args) { 
        SweetProducer s1 = new ChocolateProducer();
        SweetProducer s2 = (SweetProducer) new CakeProducer(); // No need for "explicit" upcasting 
        s1.produceSweet(); 
        s2.produceSweet(); 
        // s1.moreCacao(); Error: cannot find moreCacao()  
        ((ChocolateProducer) s1).moreCacao();
        // ((ChocolateProducer) s2).moreCacao(); Exception: CakeProducer cannot be cast to ChocolateProducer
    }
}