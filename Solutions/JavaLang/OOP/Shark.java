public class Shark extends Fish {
    public int teeth;
    public float speed;
    public boolean eatspeople;

    public Shark(String name, int age, float weight, String type,
                 int fins, boolean breathsair, int teeth,
                 float speed, boolean eatspeople) {
        super(name, age, weight, type, fins, breathsair);
        this.teeth = teeth;
        this.speed = speed;
        this.eatspeople = eatspeople;
    }

    public void hunt() {
        System.out.println(getname() + " охотится.");
    }

    public void throttle() {
        if (getbreathsair())
            System.out.println(getname() + " плывет со скоростью " + speed);
    }

    public void sharkinfa() {
        fishinfa();
        System.out.println("  Зубы: " + teeth);
        System.out.println("  Скорость: " + speed);
        System.out.println("  Ест людей: " + eatspeople);
    }

    public int getteeth() { return teeth; }
    public float getspeed() { return speed; }
    public boolean geteatspeople() { return eatspeople; }

    public void setteeth(int teeth) { this.teeth = teeth; }
    public void setspeed(float speed) { this.speed = speed; }
    public void seteatspeople(boolean eatspeople) {
        this.eatspeople = eatspeople;
    }
}
