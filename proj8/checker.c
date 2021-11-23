# include <stdio.h>
# include <string.h>

int main() {
	FILE *fp_out = fopen("output.txt", "r");
	FILE *fp_ans = fopen("correct.txt", "r");
	
	int right = 1;
	int right_val, out_val, cnt = 0;
	while (~fscanf(fp_ans, "Virtual address: %*d Physical address: %*d Value: %d\n", &right_val)) {
		++ cnt;
		if (fscanf(fp_out, "Virtual address: %*d Physical address: %*d Value: %d\n", &out_val) == EOF) {
			printf("File length dismatch!\n");
			right = 0;
			break;
		}
		if (right_val != out_val) {
			printf("Error on line %d.\n", ++ cnt);
			right = 0;
		}
	}

	if (right == 0) printf("Wrong answer.\n");
	else printf("Right answer.\n");

	fclose(fp_out);
	fclose(fp_ans);
	return 0;
}
