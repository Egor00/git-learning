class Animal {
    private String name;
    private int age;
    private float weight;

    public Animal(String name, int age, float weight) {
        this.name = name;
        this.age = age;
        this.weight = weight;
    }

    public void eat() {
        System.out.println(name+" ест.");
        this.weight+=weight/50;
    }

    public void sleep() {
        System.out.println(name+" спит.");
    }

    public void infa() {
        System.out.println("Имя: "+name);
        System.out.println("  Возраст: "+age);
        System.out.println("  Вес: "+weight);
    }

    public String getname() {return name;}
    public int getage() {return age;}
    public float getweight() {return weight;}
    
    public void setname(String name) {this.name = name;}
    public void setage(int age) {this.age = age;}
    public void setweight(float weight) {this.weight = weight;}
}

class Fish extends Animal{
    private String type;
    private int fins;
    private boolean breathsair;

    public Fish(String name, int age, float weight, String type, int fins, boolean breathsair){
        super(name, age, weight);
        this.type = type;
        this.fins = fins;
        this.breathsair = breathsair;
    }

    public void swim() {
        System.out.println(getname()+ " плывет.");
    }
    public void out_of_air() {
        if(breathsair) System.out.println(getname()+" выныривает для вдоха.");
    }
    public void fishinfa() {
        infa();
        System.out.println("  Тип: "+type);
        System.out.println("  Плавники: "+fins);
        System.out.println("  Дышит воздухом: "+breathsair);
    }

    public String gettype() {return type;}
    public int getfins() {return fins;}
    public boolean getbreathsair() {return breathsair;}
    
    public void setname(String type) {this.type = type;}
    public void setage(int fins) {this.fins = fins;}
    public void setweight(boolean breathsair) {this.breathsair = breathsair;}
}

class Shark extends Fish {
    public int teeth;
    public float speed;
    public boolean eatspeople;

    public Shark(String name, int age, float weight, String type, 
        int fins, boolean breathsair, int teeth, float speed, boolean eatspeople){
        super(name, age, weight, type, fins, breathsair);
        this.teeth = teeth;
        this.speed = speed;
        this.eatspeople = eatspeople;
    }

    public void hunt() {
        System.out.println(getname()+ " охотится.");
    }
    public void throttle() {
        if(getbreathsair()) System.out.println(getname()+" плывет со скоростью "+speed);
    }
    public void sharkinfa() {
        fishinfa();
        System.out.println("  Зубы: "+teeth);
        System.out.println("  Скорость: "+speed);
        System.out.println("  Ест людей: "+eatspeople);
    }

    public int getteeth() {return teeth;}
    public float getspeed() {return speed;}
    public boolean geteatspeople() {return eatspeople;}
    
    public void setteeth(int teeth) {this.teeth = teeth;}
    public void setspeed(float speed) {this.speed = speed;}
    public void seteatspeople(boolean eatspeople) {this.eatspeople = eatspeople;}
}

public class Main {
    public static void main(String[] args) {
        Shark tethboy = new Shark("Теть", 9, 30.0f, 
        "Жирнюк", 4, false, 50, 15.2f, true);
        tethboy.sharkinfa();
    }
}