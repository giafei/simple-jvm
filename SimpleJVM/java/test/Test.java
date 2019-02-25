package test;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class Test {
    private int i;
    private long l;
    private double d;
    private String s;

    static {
        System.out.print("��̬���캯����");
        System.out.println(Test.class.toString());
    }

    public Test()  {
    }

    public Test(String x) {
        s = x;
    }

    public void v2(int v) {
        this.i = v;
    }

    public void displayV2() {
        System.out.println("displayV2:" + s);

        System.out.println("�쳣��ջ");
        Exception e = new Exception("xx");
        e.printStackTrace(System.out);
    }


    public static void main(String[] args) throws Exception {
        Constructor<Test> constructor = Test.class.getDeclaredConstructor(String.class);
        Test t = constructor.newInstance("Hello World!");

        System.out.println("ͨ��������ú���");
        Method method = Test.class.getDeclaredMethod("displayV2");
        method.invoke(t);

        System.out.print("ͨ�������ȡ�ֶ�ֵ��");
        Field s1 = Test.class.getDeclaredField("s");
        System.out.println(s1.get(t));

        System.out.println("�����в�����");
        for (String arg:args)
        {
            System.out.println(arg);
        }

    }
}