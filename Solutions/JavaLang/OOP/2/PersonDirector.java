public class PersonDirector {
    private PersonBuilder builder;
    private String defaultProfession;
    private String defaultAddress;

    public PersonDirector(PersonBuilder builder) {
        this.builder = builder;
        this.defaultProfession = "Программист";
        this.defaultAddress = "Москва";
    }


    public Person createProgrammer(String firstName, String lastName, int age) {

        return builder
                .setFirstName(firstName)
                .setLastName(lastName)
                .setAge(age)
                .setProfession(defaultProfession)
                .setAddress(defaultAddress)
                .build();
    }

    public Person createPerson(String firstName, String lastName, int age, String profession, String address) {

        return builder
                .setFirstName(firstName)
                .setLastName(lastName)
                .setAge(age)
                .setProfession(profession)
                .setAddress(address)
                .build();
    }

    public void changeDefaults(
            String profession,
            String address) {

        this.defaultProfession = profession;
        this.defaultAddress = address;
    }

    public void showDefaults() {
        System.out.println("Профессия по умолчанию: " + defaultProfession);
        System.out.println("Адрес по умолчанию: " + defaultAddress);
    }
}