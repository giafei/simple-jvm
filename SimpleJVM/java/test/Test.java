package test;

public class Test {
	private long value = 0x123456789ABCDEFL;
	
	public static void main(String[] args) {
		System.out.println("Hello World!");
		System.out.println(new Test().value);
		System.out.println(0.5);
		
		for (String arg:args)
		{
			System.out.println(arg);
		}
	}
}