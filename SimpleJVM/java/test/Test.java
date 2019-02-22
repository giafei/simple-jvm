package test;

public class Test {

    private long value = 0x123456789ABCDEFL;

    static {
        System.out.println("static Test");
    }

    private void print(long v, String s) {
        System.out.println(Long.toString(v));
        System.out.println(s);
    }

    public static void main(String[] args) {
        System.out.println("Hello World!");
        System.out.println(Long.toString( new Test().value));

        new Test().print(0xFEDCBA987654321L, "abdc");

        System.out.println(0.5);

        for (String arg:args)
        {
            System.out.println(arg);
        }

        System.out.println(Test.class.toString());
    }
}