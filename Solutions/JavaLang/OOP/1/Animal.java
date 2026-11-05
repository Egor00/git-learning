public class Animal {
    private String name;
    private int age;
    private float weight;

    public Animal(String name, int age, float weight) {
        this.name = name;
        this.age = age;
        this.weight = weight;
    }

    public void eat() {
        System.out.println(name + " ест.");
        this.weight += weight / 50;
    }

    public void sleep() {
        System.out.println(name + " спит.");
    }

    public void infa() {
        System.out.println("Имя: " + name);
        System.out.println("  Возраст: " + age);
        System.out.println("  Вес: " + weight);
    }

    public String getname() { return name; }
    public int getage() { return age; }
    public float getweight() { return weight; }

    public void setname(String name) { this.name = name; }
    public void setage(int age) { this.age = age; }
    public void setweight(float weight) { this.weight = weight; }
}
