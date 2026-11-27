
class HumanBuilder {
    private String firstname;
    private String lastname;
    private int age;
    private String occupation;
    private String address;

    public HumanBuilder() {
        this.firstname = "Без имени";
        this.lastname = "Без фамилии";
        this.age = 18;
        this.address = "Город N";
        this.occupation = "Не указано";
    }

    public String getFirstname() {
        return firstname;
    }

    public HumanBuilder setFirstname(String firstname) {
        this.firstname = firstname;
        return this;
    }

    public String getLastname() {
        return lastname;
    }

    public HumanBuilder setLastname(String lastname) {
        this.lastname = lastname;
        return this;
    }

    public int getAge() {
        return age;
    }

    public HumanBuilder setAge(int age) {
        this.age = age;
        return this;
    }

    public String getOccupation() {
        return occupation;
    }

    public HumanBuilder setOccupation(String occupation) {
        this.occupation = occupation;
        return this;
    }

    public String getAddress() {
        return address;
    }

    public HumanBuilder setAddress(String address) {
        this.address = address;
        return this;
    }


    public HumanBuilder withIdentity(String firstname, String lastname, int age) {
        this.firstname = firstname;
        this.lastname = lastname;
        this.age = age;
        return this;
    }

    public HumanBuilder withOccupation(String occupation) {
        this.occupation = occupation;
        return this;
    }

    public HumanBuilder withAddress(String address) {
        this.address = address;
        return this;
    }

    public Human build() {
        if (firstname != null && lastname != null && age > 0 && occupation != null && address != null) {
            return new Human(firstname, lastname, age, address, occupation);
        }
        else {
            throw new IllegalArgumentException("Что-то непонятное с аргументами!");
        }
    }
}

class Human {
    private String firstname;
    private String lastname;
    private int age;
    private String address;
    private String occupation;

    public Human() {
        this.firstname = "Без имени";
        this.lastname = "Без фамилии";
        this.age = 18;
        this.address = "Город N";
        this.occupation = "Безработный";
    }

    public Human(String firstname, String lastname, int age, String address, String occupation) {
        this.firstname = firstname;
        this.lastname = lastname;
        this.age = age;
        this.address = address;
        this.occupation = occupation;
    }

    public String getFirstname() {
        return firstname;
    }

    public void setFirstname(String firstname) {
        this.firstname = firstname;
    }

    public String getLastname() {
        return lastname;
    }

    public void setLastname(String lastname) {
        this.lastname = lastname;
    }

    public int getAge() {
        return age;
    }

    public void setAge(int age) {
        this.age = age;
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
    }

    public String getOccupation() {
        return occupation;
    }

    public void setOccupation(String occupation) {
        this.occupation = occupation;
    }


    public void introduce() {
        System.out.println("Привет! Меня зовут " + firstname + " " + lastname + ", мне " + age +
                " лет, род занятий: " + occupation + ", я живу в " + address);
    }

    public void leaveWork() {
        if(this.occupation != "Безработный") {
            System.out.println("Уволился.");
            this.occupation = "Безработный";
        }
    }

    public void aged() {
        System.out.println("Вот и еще год прошел. Мне теперь " + (age+1));
        this.age = age+1;
    }
}