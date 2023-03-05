import random

# Open the file for writing
with open("addresses.txt", "w") as f:
    # Generate and write 1000 random numbers
    for i in range(1000):
        # Generate a random number between 0 and 65535 (inclusive)
        num = random.randint(0, 65535)
        # Write the number to the file followed by a newline character
        f.write(str(num) + "\n")
