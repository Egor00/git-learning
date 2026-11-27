public class Main {
    public static void main(String[] args) {

        HumanBuilder builder = new HumanBuilder();
        builder.setFirstname("Pepik").setLastname("Popski").setAge(10).setOccupation("Digger");

        Human person = builder.build();

        person.introduce();
        person.leaveWork();
        person.aged();
    }
}