import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.*;

public class Quicksort extends RecursiveAction {
	static final int THRESHOLD = 10;

	private int begin;
	private int end;
	private int[] array;

	public Quicksort(int begin, int end, int[] array) {
		this.begin = begin;
		this.end = end;
		this.array = array;
	}

	protected void compute() {
		if (end - begin < THRESHOLD) {
			//Bubble Sort
			for (int i = end; i >= begin + 1; -- i)
			{
				for (int j = begin; j < i; ++ j)
				{
					if (array[j]>(array[j + 1])) {
						int tmp = array[j];
						array[j] = array[j + 1];
						array[j + 1] = tmp;
					}
				}
			}
		} 
		else 
		{
			int pivot = array[begin];
			int low = begin, high = end;
			while (low < high) {
				while (low < high && array[high]>=pivot) -- high;
				if (low < high) array[low ++] = array[high];
				while (low < high && array[low]<=pivot) ++ low;
				if (low < high) array[high --] = array[low];
			}
			array[low] = pivot;
            
			Quicksort leftTask = new Quicksort(begin, low - 1, array);
			Quicksort rightTask = new Quicksort(low + 1, end, array);

			leftTask.fork();
			rightTask.fork();

			leftTask.join();
			rightTask.join();
		}
	}

	public static void main(String[] args) {
		ForkJoinPool pool = new ForkJoinPool();
		Scanner sc = new Scanner(System.in);

		System.out.print("input the array length n (0<=n<=10000): ");
		int n = sc.nextInt();
		if (n < 0 || n > 10000)
		{
			System.out.println("ERROR:n is out of range");
			System.exit(1);
		}

		int[] array = new int[n];

		System.out.print("generate the random elements?(Y/N): ");
		char opt = sc.next().charAt(0);
		if (opt == 'Y') 
		{
			java.util.Random rand = new java.util.Random();
			for (int i = 0; i < n; ++ i) 
				array[i] = rand.nextInt(1000);
			System.out.print("The original array: ");
			System.out.println(Arrays.toString(array));
		} 
		else
		{
			System.out.println("input the array elements: ");
			for (int i = 0; i < n; ++ i)
				array[i] = sc.nextInt();
		}
		

		Quicksort task = new Quicksort(0, n - 1, array);
		pool.invoke(task);
		System.out.print("after sorting: ");
		System.out.println(Arrays.toString(array));
	}
}
//javac Quicksort.java
//java Quicksort
