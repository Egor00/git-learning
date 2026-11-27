public class PersonBuilder {

    private String firstName;
    private String lastName;
    private int age;
    private String profession;
    private String address;


    public PersonBuilder setFirstName(String firstName) {
        this.firstName = firstName;
        return this;
    }

    public PersonBuilder setLastName(String lastName) {
        this.lastName = lastName;
        return this;
    }

    public PersonBuilder setAge(int age) {
        this.age = age;
        return this;
    }

    public PersonBuilder setProfession(String profession) {
        this.profession = profession;
        return this;
    }

    public PersonBuilder setAddress(String address) {
        this.address = address;
        return this;
    }


    public Person build() {
        return new Person(firstName, lastName, age, profession, address);
    }


    public PersonBuilder reset() {
        firstName = null;
        lastName = null;
        age = 0;
        profession = null;
        address = null;

        return this;
    }

    public boolean isReady() {
        return firstName != null && lastName != null && age > 0 && profession != null && address != null;
    }
}