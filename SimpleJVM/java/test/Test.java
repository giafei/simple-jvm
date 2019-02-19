package test;

public class Test {
	private long value = 0x123456789ABCDEFL;
	
	public static void main(String[] args) {
		System.out.println("Hello World!");
		System.out.format("0x%08X\n", new Test().value);
		System.out.println(0.5);
		
		for (String arg:args)
		{
			System.out.println(arg);
		}
	}
}