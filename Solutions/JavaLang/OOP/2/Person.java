public class Person {

    private String firstName;
    private String lastName;
    private int age;
    private String profession;
    private String address;


    public Person(String firstName, String lastName, int age, String profession, String address) {
        this.firstName = firstName;
        this.lastName = lastName;
        this.age = age;
        this.profession = profession;
        this.address = address;
    }


    public void introduce() {
        System.out.println("Здравствуйте! Меня зовут " + firstName + " " + lastName + ".");
    }

    public void work() {
        System.out.println(firstName + " работает по профессии: " + profession);
    }

    public void move() {
        System.out.println(firstName + " живёт по адресу: " + address);
    }


    public String getFirstName() {
        return firstName;
    }

    public String getLastName() {
        return lastName;
    }

    public int getAge() {
        return age;
    }

    public String getProfession() {
        return profession;
    }

    public String getAddress() {
        return address;
    }


    public void setFirstName(String firstName) {
        this.firstName = firstName;
    }

    public void setLastName(String lastName) {
        this.lastName = lastName;
    }

    public void setAge(int age) {
        this.age = age;
    }

    public void setProfession(String profession) {
        this.profession = profession;
    }

    public void setAddress(String address) {
        this.address = address;
    }
}