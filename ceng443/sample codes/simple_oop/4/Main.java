// dynamic polymorphism

import java.util.*; 

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
} 
class CookieProducer extends SweetProducer { 
    @Override  
    public void produceSweet() { 
        System.out.println("Cookie produced");  
    }
}

class SweetCreator { 
    private List<SweetProducer> sweetProducer; 
    public SweetCreator(List<SweetProducer> sweetProducer) { 
        this.sweetProducer = sweetProducer; 
        
    } 
    public void createSweets() { 
        for(SweetProducer sweet: sweetProducer) sweet.produceSweet();  
        
    }
}

public class Main {
    public static void main(String[] args) { 
        SweetCreator sweetCreator = new SweetCreator(Arrays.asList( new ChocolateProducer(), new CookieProducer())); 
        sweetCreator.createSweets(); 
    }
}