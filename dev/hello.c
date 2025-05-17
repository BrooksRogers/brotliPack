#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char stored_password[] = "It insists upon itself";

unsigned char encrypted_flag[] = {0x0F, 0x38, 0x61, 0x2E, 0x15, 0x3B, 0x0C, 0x1F, 0x18, 0x1C, 0x5D};

int main(void)
{
	char input[128];
	printf("Enter password: ");
	if (fgets(input, sizeof(input), stdin) == NULL)
	{
		fprintf(stderr, "Error reading input.\n");
		return EXIT_FAILURE;
	}
	input[strcspn(input, "\n")] = '\0';

	if (strcmp(input, stored_password) != 0)
	{
		printf("L password.\n");
		return EXIT_FAILURE;
	}

	int flag_len = sizeof(encrypted_flag);
	char decrypted_flag[flag_len + 1];
	int key_len = strlen(input);

	for (int i = 0; i < flag_len; i++)
	{
		decrypted_flag[i] = encrypted_flag[i] ^ input[i % key_len];
	}
	decrypted_flag[flag_len] = '\0';

	printf("Good job dude\nDecrypted flag: %s\n", decrypted_flag);

	return EXIT_SUCCESS;
}
