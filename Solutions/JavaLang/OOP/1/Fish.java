public class Fish extends Animal {
    private String type;
    private int fins;
    private boolean breathsair;

    public Fish(String name, int age, float weight,
                String type, int fins, boolean breathsair) {
        super(name, age, weight);
        this.type = type;
        this.fins = fins;
        this.breathsair = breathsair;
    }

    public void swim() {
        System.out.println(getname() + " плывет.");
    }

    public void out_of_air() {
        if (breathsair)
            System.out.println(getname() + " выныривает для вдоха.");
    }

    public void fishinfa() {
        infa();
        System.out.println("  Тип: " + type);
        System.out.println("  Плавники: " + fins);
        System.out.println("  Дышит воздухом: " + breathsair);
    }

    public String gettype() { return type; }
    public int getfins() { return fins; }
    public boolean getbreathsair() { return breathsair; }

    public void settype(String type) { this.type = type; }
    public void setfins(int fins) { this.fins = fins; }
    public void setbreathsair(boolean breathsair) {
        this.breathsair = breathsair;
    }
}
