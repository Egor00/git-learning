public class Main {
    public static void main(String[] args) {

        PersonBuilder builder = new PersonBuilder();

        PersonDirector director = new PersonDirector(builder);

        Person person1 = director.createProgrammer("Иван", "Иванов", 25);

        person1.introduce();
        person1.work();
        person1.move();

        System.out.println();

        Person person2 = new PersonBuilder()
                .setFirstName("Анна")
                .setLastName("Петрова")
                .setAge(30)
                .setProfession("Врач")
                .setAddress("Санкт-Петербург")
                .build();

        person2.introduce();
        person2.work();
        person2.move();
    }
}