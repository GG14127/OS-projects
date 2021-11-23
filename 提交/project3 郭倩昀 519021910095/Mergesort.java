import java.util.Arrays;
import java.util.Scanner;
import java.util.concurrent.*;

public class Mergesort extends RecursiveAction {
	static final int THRESHOLD = 10;

	private int begin;
	private int end;
	private int[] array;

	public Mergesort(int begin, int end, int[] array) {
		this.begin = begin;
		this.end = end;
		this.array = array;
	}

	protected void compute() 
	{
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
			int mid = begin + (end - begin) / 2;
            //new task
			Mergesort leftTask = new Mergesort(begin, mid, array);
			Mergesort rightTask = new Mergesort(mid + 1, end, array);
			//fork and join
			leftTask.fork();
			rightTask.fork();

			leftTask.join();
			rightTask.join();

			int[] tmp = new int [end - begin + 1];
			//merge the 2 halves
			int dex1 = begin, dex2 = mid + 1, newdex = 0;
			while (dex1 <= mid && dex2 <= end) {
				if (array[dex1]<=array[dex2]) tmp[newdex++] = array[dex1++];
				else tmp[newdex++] = array[dex2++];
			}
			while (dex1 <= mid) tmp[newdex++] = array[dex1++];
			while (dex2 <= end) tmp[newdex++] = array[dex2++];

			for (int i = 0; i < newdex; ++ i)
				array[i + begin] = tmp[i];
		}
	}

	public static void main(String[] args) 
	{
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
		

		Mergesort task = new Mergesort(0, n - 1, array);
		pool.invoke(task);
		System.out.print("after sorting: ");
		System.out.println(Arrays.toString(array));
	}
}

//javac Mergesort.java
//java Mergesort
